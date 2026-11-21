// NFCTapInXR bring-up: Stamp-S3Bat DIP + Adafruit PN532 shield over I2C.
//
// Wiring (see README.md):
//   Stamp G5    -> shield A4 (SDA)
//   Stamp G6    -> shield A5 (SCL)
//   Stamp G7    -> shield D2 (IRQ)
//   Stamp 5VOUT -> shield 5V
//   Stamp GND   -> shield GND
//
// The board's own I2C bus (G48 SDA, G47 SCL) belongs to the M5PM1 power chip,
// which switches the 5VOUT pin through its GPIO1. The reader gets a second bus.
//
// Serial output, 115200:
//   boot lines, an I2C scan of the reader bus (expect 0x24), the PN532 firmware
//   version, then one "TAP <uid>" line per card presented.

#include <Arduino.h>
#include <esp_system.h>
#include <Wire.h>
#include <M5PM1.h>
#include <Adafruit_PN532.h>

static const int PM1_SDA = 48;
static const int PM1_SCL = 47;

static const int NFC_SDA = 5;
static const int NFC_SCL = 6;
static const int NFC_IRQ = 7;
static const int NFC_RESET = -1;  // shield reset is not wired

static const m5pm1_gpio_num_t PM1_5VOUT_EN = M5PM1_GPIO_NUM_1;

// Without a LiPo fitted, switching 5VOUT on into the shield collapses the
// board's supply and the M5PM1 power-cycles everything (seen 2026-11-14).
// The shield's 5V pin is fed from the Stamp's 3V3 pin instead, which also
// keeps its I2C pull-ups (10k to that pin) at a level the ESP32-S3 accepts.
static const bool READER_ON_5VOUT = false;

static M5PM1 pm1;
static Adafruit_PN532 *nfc = nullptr;

static void halt(const char *why) {
  Serial.printf("HALT %s\r\n", why);
  for (;;) delay(1000);
}

// The M5PM1 sleeps when its bus is idle and fails the first transaction by
// design, so every setup call gets one retry.
static bool pm1Ok(m5pm1_err_t (*call)(), const char *what) {
  for (int i = 0; i < 2; i++) {
    if (call() == M5PM1_OK) return true;
    delay(5);
  }
  Serial.printf("pm1: %s failed\r\n", what);
  return false;
}

static void powerUp() {
  m5pm1_err_t err = M5PM1_FAIL;
  for (int i = 0; i < 3 && err != M5PM1_OK; i++) {
    err = pm1.begin(&Wire, M5PM1_DEFAULT_ADDR, PM1_SDA, PM1_SCL, M5PM1_I2C_FREQ_100K);
    if (err != M5PM1_OK) delay(20);
  }
  if (err != M5PM1_OK) halt("M5PM1 not found on G48/G47");

  Serial.println("pm1 up");
  Serial.flush();

  // No battery in this build: stop the charger cycling into an empty socket.
  pm1Ok([] { return pm1.setChargeEnable(false); }, "charge off");
  Serial.println("charge off");
  Serial.flush();
  delay(200);

  if (!READER_ON_5VOUT) {
    Serial.println("5VOUT left off; reader runs from its own supply");
    delay(100);
    return;
  }

  Serial.println("switching 5VOUT on");
  Serial.flush();

  // 5VOUT_EN is M5PM1 GPIO1. It is low after power-up.
  bool on = pm1Ok([] { return pm1.gpioSetFunc(PM1_5VOUT_EN, M5PM1_GPIO_FUNC_GPIO); }, "5VOUT_EN func") &&
            pm1Ok([] { return pm1.gpioSet(PM1_5VOUT_EN, M5PM1_GPIO_MODE_OUTPUT, 1, M5PM1_GPIO_PULL_NONE,
                                          M5PM1_GPIO_DRIVE_PUSHPULL); }, "5VOUT_EN high");
  if (!on) halt("could not switch on 5VOUT");
  Serial.println("5VOUT on");

  delay(100);  // let the shield's supply settle before the first I2C transfer
}

static void scanReaderBus() {
  Serial.print("reader bus:");
  int found = 0;
  for (uint8_t a = 1; a < 0x7F; a++) {
    Wire1.beginTransmission(a);
    if (Wire1.endTransmission() == 0) {
      Serial.printf(" 0x%02X", a);
      found++;
    }
  }
  Serial.println(found ? "" : " nothing (check SDA/SCL, 5VOUT, pull-ups)");
}

void setup() {
  Serial.begin(115200);
  delay(1500);  // give the USB console time to attach
  Serial.printf("\r\nNFCTapInXR bring-up (reset reason %d)\r\n", (int)esp_reset_reason());

  powerUp();

  Wire1.begin(NFC_SDA, NFC_SCL, 100000);
  scanReaderBus();

  nfc = new Adafruit_PN532(NFC_IRQ, NFC_RESET, &Wire1);
  if (!nfc->begin()) halt("PN532 begin failed");

  uint32_t ver = nfc->getFirmwareVersion();
  if (!ver) halt("no PN532 answer at 0x24");
  Serial.printf("PN5%02X firmware %u.%u\r\n", (unsigned)((ver >> 24) & 0xFF),
                (unsigned)((ver >> 16) & 0xFF), (unsigned)((ver >> 8) & 0xFF));

  if (!nfc->SAMConfig()) halt("SAMConfig failed");
  Serial.println("ready, present a card");
}

void loop() {
  static uint8_t lastUid[7];
  static uint8_t lastLen = 0;
  static uint32_t lastSeen = 0;

  uint8_t uid[7];
  uint8_t len = 0;

  // A timeout keeps loop() from blocking forever with no card present.
  if (!nfc->readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &len, 500)) {
    if (lastLen && millis() - lastSeen > 1000) lastLen = 0;  // card gone
    return;
  }

  bool same = (len == lastLen) && memcmp(uid, lastUid, len) == 0;
  lastSeen = millis();
  if (same) return;  // one line per tap, not per poll

  memcpy(lastUid, uid, len);
  lastLen = len;

  Serial.print("TAP ");
  for (uint8_t i = 0; i < len; i++) Serial.printf("%02X", uid[i]);
  Serial.printf("  (%u-byte uid%s)\r\n", len,
                len == 4 ? ", MIFARE Classic class" : len == 7 ? ", Ultralight/NTAG class" : "");
}
