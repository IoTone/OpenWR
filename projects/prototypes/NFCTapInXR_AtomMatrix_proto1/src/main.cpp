// NFCTapInXR on an M5 ATOM Matrix with a PN532 NFC Module V3 (serial / HSU).
//
// Serial, not I2C or SPI: on this particular module the mode switch only
// produced a working chip with both sliders at the HSU setting (the "0 0" row
// of the silkscreen table; both sliders away from the "ON KE" lettering).
// I2C and SPI never answered in any setting tried (2026-11-20).
//
// Wiring. The module's serial lines share pads with the other interfaces:
// its TX is the pad marked SDA (4-pin header) or MOSI (8-pin header), its RX
// is the pad marked SCL or SS.
//   PN532 VCC          -> ATOM 3V3
//   PN532 GND          -> ATOM GND
//   PN532 TX (SDA/MOSI) -> ATOM G19
//   PN532 RX (SCL/SS)   -> ATOM G22
//   SCK, MISO, IRQ and RSTO are not used.
//
// BLE: Nordic UART service, no pairing. Each good read notifies "TAP <uid hex>\n";
// a failed read notifies "ERR read\n". Writing "ping" to RX answers "pong\n".
//
// LED matrix:
//   blinking blue   advertising, waiting for a connection
//   solid green     connected
//   rainbow         card read
//   blinking red    connection dropped (2 s), then back to blinking blue
//   orange flash    bad read, or the reader is not answering (retried every 2 s)

#include <Arduino.h>
#include <FastLED.h>
#include <NimBLEDevice.h>
#include <Adafruit_PN532.h>

// ---- pins -------------------------------------------------------------------
static const int LED_PIN = 27;  // 5x5 WS2812C matrix
static const int NUM_LEDS = 25;
static const int NFC_RX = 19;  // ATOM receives; module pad SDA / MOSI
static const int NFC_TX = 22;  // ATOM transmits; module pad SCL / SS

// M5: keep the matrix near brightness 20 or the LEDs and the cover overheat.
static const uint8_t BRIGHTNESS = 20;

// ---- BLE --------------------------------------------------------------------
static const char *DEVICE_NAME = "NFCTapIn";
static const char *NUS_SERVICE = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E";
static const char *NUS_RX = "6E400002-B5A3-F393-E0A9-E50E24DCCA9E";  // central writes
static const char *NUS_TX = "6E400003-B5A3-F393-E0A9-E50E24DCCA9E";  // we notify
static const size_t BLE_CHUNK = 20;  // lenses cannot raise the MTU; default 23 - 3

// ---- timings (ms) -----------------------------------------------------------
static const uint32_t RAINBOW_MS = 1200;
static const uint32_t ORANGE_MS = 400;
static const uint32_t DROPPED_MS = 2000;
static const uint32_t SAME_CARD_MS = 1500;  // a card held on the reader taps once
static const uint32_t REARM_MS = 150;
static const uint32_t READER_RETRY_MS = 2000;

static CRGB leds[NUM_LEDS];
static Adafruit_PN532 *nfc = nullptr;

static NimBLEServer *server = nullptr;
static NimBLECharacteristic *txChar = nullptr;

static volatile bool connected = false;
static volatile uint32_t droppedUntil = 0;
static volatile bool pingRequested = false;

static uint32_t rainbowUntil = 0;
static uint32_t orangeUntil = 0;

static bool readerOk = false;
static bool armed = false;
static uint32_t nextReaderTry = 0;
static uint32_t rearmAt = 0;

static uint8_t lastUid[10];
static uint8_t lastLen = 0;
static uint32_t lastSeen = 0;

// ---- BLE plumbing -------------------------------------------------------------

static void sendLine(const String &line) {
  Serial.print(line);
  if (!connected || !txChar) return;
  const uint8_t *p = (const uint8_t *)line.c_str();
  size_t left = line.length();
  while (left) {
    size_t n = left < BLE_CHUNK ? left : BLE_CHUNK;
    txChar->setValue(p, n);
    txChar->notify();
    p += n;
    left -= n;
  }
}

class ServerCallbacks : public NimBLEServerCallbacks {
  void onConnect(NimBLEServer *, NimBLEConnInfo &info) override {
    connected = true;
    droppedUntil = 0;
    Serial.printf("ble: connected %s\r\n", info.getAddress().toString().c_str());
  }
  void onDisconnect(NimBLEServer *, NimBLEConnInfo &, int reason) override {
    connected = false;
    droppedUntil = millis() + DROPPED_MS;
    Serial.printf("ble: disconnected (reason %d), advertising\r\n", reason);
    NimBLEDevice::startAdvertising();
  }
};

class RxCallbacks : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic *c, NimBLEConnInfo &) override {
    std::string v = c->getValue();
    while (!v.empty() && (v.back() == '\n' || v.back() == '\r' || v.back() == ' ')) v.pop_back();
    if (v == "ping") pingRequested = true;  // answered from loop(), not the BLE task
  }
};

static void bleBegin() {
  NimBLEDevice::init(DEVICE_NAME);
  NimBLEDevice::setSecurityAuth(false, false, false);  // no bonding: Spectacles cannot pair

  server = NimBLEDevice::createServer();
  server->setCallbacks(new ServerCallbacks());

  NimBLEService *svc = server->createService(NUS_SERVICE);
  NimBLECharacteristic *rx =
      svc->createCharacteristic(NUS_RX, NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR);
  rx->setCallbacks(new RxCallbacks());
  txChar = svc->createCharacteristic(NUS_TX, NIMBLE_PROPERTY::NOTIFY);
  svc->start();

  // Name in the advertisement (lenses scan by name), service UUID in the scan response.
  NimBLEAdvertisementData adv;
  adv.setFlags(BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP);
  adv.setName(DEVICE_NAME);
  NimBLEAdvertisementData scan;
  scan.setCompleteServices(NimBLEUUID(NUS_SERVICE));

  NimBLEAdvertising *a = NimBLEDevice::getAdvertising();
  a->setAdvertisementData(adv);
  a->setScanResponseData(scan);
  a->enableScanResponse(true);
  a->start();
  Serial.printf("ble: advertising as %s\r\n", DEVICE_NAME);
}

// ---- NFC --------------------------------------------------------------------

// Send one PN532 command over serial and check its acknowledge frame.
static bool command(const uint8_t *data, uint8_t n) {
  static const uint8_t ack[] = {0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00};
  uint8_t sum = 0xD4;
  for (uint8_t i = 0; i < n; i++) sum += data[i];
  const uint8_t head[] = {0x00, 0x00, 0xFF, (uint8_t)(n + 1), (uint8_t)(0 - (n + 1)), 0xD4};
  const uint8_t tail[] = {(uint8_t)(0 - sum), 0x00};
  while (Serial1.available()) Serial1.read();
  Serial1.write(head, sizeof head);
  Serial1.write(data, n);
  Serial1.write(tail, sizeof tail);
  uint8_t got[sizeof ack];
  uint32_t start = millis();
  while (Serial1.available() < (int)sizeof ack && millis() - start < 100) delay(1);
  return Serial1.readBytes(got, sizeof got) == sizeof got && memcmp(got, ack, sizeof ack) == 0;
}

static bool readerBegin() {
  if (!nfc) nfc = new Adafruit_PN532((uint8_t)0xFF, &Serial1);  // no reset pin
  if (!nfc->begin()) return false;
  uint32_t ver = nfc->getFirmwareVersion();
  if (!ver) return false;
  Serial.printf("nfc: PN5%02X firmware %u.%u\r\n", (unsigned)((ver >> 24) & 0xFF),
                (unsigned)((ver >> 16) & 0xFF), (unsigned)((ver >> 8) & 0xFF));
  if (!nfc->SAMConfig()) return false;
  // Type A analog settings (RFConfiguration item 0x0A) with the receiver gain
  // raised from the default 38 dB (0x59) to 48 dB (0x79); the other ten bytes
  // are the chip's defaults. Helps small tags. Not fatal if refused.
  static const uint8_t gain[] = {0x32, 0x0A, 0x79, 0xF4, 0x3F, 0x11, 0x4D, 0x85, 0x61, 0x6F, 0x26, 0x62, 0x87};
  if (!command(gain, sizeof gain)) Serial.println("nfc: gain setting refused");
  delay(20);
  while (Serial1.available()) Serial1.read();  // drop that command's reply
  return true;
}

// A reply from the armed reader means a card arrived. The reply is 19 bytes or
// more; wait for them (or for the line to go quiet) before handing it to the
// library, which would otherwise block on a half-received frame.
static bool cardWaiting() {
  static int seen = 0;
  static uint32_t lastGrowth = 0;
  int n = Serial1.available();
  if (n == 0) {
    seen = 0;
    return false;
  }
  if (n != seen) {
    seen = n;
    lastGrowth = millis();
  }
  if (n < 19 && millis() - lastGrowth < 20) return false;
  seen = 0;
  return true;
}

// Ask the reader to poll until a tag of any family it knows shows up
// (InAutoPoll: endless, 150 ms period; type A, FeliCa 212/424, type B, Jewel).
// The library has no call for this, and over serial its own arming call
// blocks for a second and reports failure while no card is present.
static bool arm() {
  static const uint8_t autoPoll[] = {0x60, 0xFF, 0x01, 0x00, 0x01, 0x02, 0x03, 0x04};
  armed = command(autoPoll, sizeof autoPoll);
  return armed;
}

// Pull the tag's ID out of an InAutoPoll reply. Returns the family name, or
// nullptr if the reply is not a well-formed single-tag answer.
static const char *parseTag(const uint8_t *b, int n, const uint8_t **id, uint8_t *idLen) {
  int i = 0;
  while (i + 2 < n && !(b[i] == 0x00 && b[i + 1] == 0x00 && b[i + 2] == 0xFF)) i++;
  b += i;
  n -= i;
  if (n < 11 || (uint8_t)(b[3] + b[4]) != 0 || b[5] != 0xD5 || b[6] != 0x61 || b[7] < 1) return nullptr;
  uint8_t type = b[8], ln = b[9];
  const uint8_t *d = b + 10;  // d[0] is the target number
  if (10 + ln > n) return nullptr;
  switch (type & 0x0F) {
    case 0x00:  // ISO14443A: SENS_RES(2) SEL_RES idLen id
      if (ln < 5 || 5 + d[4] > ln) return nullptr;
      *id = d + 5;
      *idLen = d[4];
      return "type A";
    case 0x01:
    case 0x02:  // FeliCa: len 0x01 IDm(8)
      if (ln < 11) return nullptr;
      *id = d + 3;
      *idLen = 8;
      return "FeliCa";
    case 0x03:  // ISO14443B: 0x50 PUPI(4)
      if (ln < 6) return nullptr;
      *id = d + 2;
      *idLen = 4;
      return "type B";
    case 0x04:  // Jewel/Topaz: SENS_RES(2) id(4)
      if (ln < 7) return nullptr;
      *id = d + 3;
      *idLen = 4;
      return "Jewel";
  }
  return nullptr;
}

static void badRead(const char *why) {
  orangeUntil = millis() + ORANGE_MS;
  Serial.printf("nfc: bad read (%s)\r\n", why);
  sendLine("ERR read\n");
}

static void serviceReader() {
  uint32_t now = millis();

  if (!readerOk) {
    if ((int32_t)(now - nextReaderTry) < 0) return;
    nextReaderTry = now + READER_RETRY_MS;
    readerOk = readerBegin() && arm();
    if (readerOk) {
      Serial.println("nfc: ready");
    } else {
      orangeUntil = now + ORANGE_MS;
      Serial.println("nfc: no answer from the reader");
    }
    return;
  }

  if (!armed) {
    if ((int32_t)(now - rearmAt) < 0) return;
    if (!arm()) {
      readerOk = false;  // lost the reader; retry from scratch
      badRead("reader stopped answering");
    }
    return;
  }

  if (!cardWaiting()) return;  // nothing in the field yet

  uint8_t raw[80];
  int got = 0;
  while (Serial1.available() && got < (int)sizeof raw) raw[got++] = Serial1.read();
  while (Serial1.available()) Serial1.read();  // nothing of this reply may leak into the next
  const uint8_t *uid = nullptr;
  uint8_t len = 0;
  const char *family = parseTag(raw, got, &uid, &len);
  bool ok = family != nullptr;
  armed = false;
  rearmAt = now + REARM_MS;

  if (!ok || len < 4 || len > 10) {
    badRead(ok ? "odd id length" : "card moved or collided");
    return;
  }

  bool same = len == lastLen && memcmp(uid, lastUid, len) == 0 && now - lastSeen < SAME_CARD_MS;
  lastSeen = now;
  if (same) return;  // still the same card on the reader
  memcpy(lastUid, uid, len);
  lastLen = len;

  String line = "TAP ";
  char hex[3];
  for (uint8_t i = 0; i < len; i++) {
    snprintf(hex, sizeof hex, "%02X", uid[i]);
    line += hex;
  }
  line += "\n";
  Serial.printf("nfc: %s tag\r\n", family);
  sendLine(line);
  rainbowUntil = now + RAINBOW_MS;
}

// ---- LEDs -------------------------------------------------------------------

static void fill(const CRGB &c) { fill_solid(leds, NUM_LEDS, c); }

static void renderLeds() {
  static uint32_t lastFrame = 0;
  uint32_t now = millis();
  if (now - lastFrame < 20) return;  // ~50 fps
  lastFrame = now;

  if ((int32_t)(rainbowUntil - now) > 0) {
    uint8_t base = (uint8_t)(now / 4);
    for (int i = 0; i < NUM_LEDS; i++) {
      int row = i / 5, col = i % 5;
      leds[i] = CHSV(base + (row + col) * 24, 255, 255);  // diagonal sweep
    }
  } else if ((int32_t)(orangeUntil - now) > 0) {
    fill(CRGB(255, 80, 0));
  } else if ((int32_t)(droppedUntil - now) > 0) {
    fill((now / 250) % 2 ? CRGB::Red : CRGB::Black);
  } else if (connected) {
    fill(CRGB::Green);
  } else {
    fill((now / 500) % 2 ? CRGB::Blue : CRGB::Black);
  }
  FastLED.show();
}

// ---- main -------------------------------------------------------------------

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\r\nNFCTapInXR ATOM Matrix");

  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  fill(CRGB::Black);
  FastLED.show();

  Serial1.begin(115200, SERIAL_8N1, NFC_RX, NFC_TX);  // the library re-begins it and keeps these pins
  Serial1.setTimeout(30);  // its fixed-length reads must not stall the LEDs
  bleBegin();
}

void loop() {
  serviceReader();
  if (pingRequested) {
    pingRequested = false;
    sendLine("pong\n");
  }
  renderLeds();
  delay(2);
}
