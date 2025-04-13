/*
 * TODO: Add in OpenWR BLE Controller functions
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*
 * @Hardwares: M5Core + Unit HEX
 * @Platform Version: Arduino M5Stack Board Manager v2.1.3
 * @Dependent Library:
 * M5Stack@^0.4.6: https://github.com/m5stack/M5Stack
 * FastLED@^3.9.10: https://github.com/FastLED/FastLED
 */
/*
  HeadLight_M5Atom_proto1

  Goal of PoC is to power on the super bright LED matrix.  Eventually later we can set it to allow color control.

  switch characteristic is a toggle switch intended to be multistate to change the led matrix colors
  button characteristic is intended to be a physical button on the device, however, it could be written to
  remotely over ble to turn on off the matrix
  
  Blink + BLE Example

``BLE Example: https://wiki.seeedstudio.com/XIAO-BLE-Sense-Bluetooth-Usage/

  Turns an LED on for one second, then off for one second, repeatedly.

  Most Arduinos have an on-board LED you can control. On the UNO, MEGA and ZERO
  it is attached to digital pin 13, on MKR1000 on pin 6. LED_BUILTIN is set to
  the correct LED pin independent of which board is used.
  If you want to know what pin the on-board LED is connected to on your Arduino
  model, check the Technical Specs of your board at:
  https://www.arduino.cc/en/Main/Products

  modified 8 May 2014
  by Scott Fitzgerald
  modified 2 Sep 2016
  by Arturo Guadalupi
  modified 8 Sep 2016
  by Colby Newman

  This example code is in the public domain.

  http://www.arduino.cc/en/Tutorial/Blink

  For changes to NIMBLE: https://h2zero.github.io/NimBLE-Arduino/md_1_8x__to2_8x__migration__guide.html

*/
// #include "M5Atom.h"
#include <M5Stack.h>
// #include <M5Unified.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "FastLED.h"
//
// Atom uses pin G26
// StickC uses pin G26
// M5NanoC6 uses G2
#define Neopixel_PIN (26) // (21)
#define NUM_LEDS     (37)
#define TOGGLE_STATE_0 0  // BRIGHT LIGHTS
#define TOGGLE_STATE_1 1  // DIM_LIGHTS

CRGB leds[NUM_LEDS];
uint8_t gHue = 0;  // Initial tone value.  起始色调数值
uint8_t gToggleState = TOGGLE_STATE_0;
bool gButtonState = true;
static bool isBLEConnected = false;

#define SWVERSION "0.0.1"
#define ADVERT "HeadLight-OpenWR"
#define ADVERT_SHORT "HeadLight

// https://github.com/IoTone/ProjectMagNET/wiki/Mag*NET-BLE-Service-Definition#service-uuid-
#define SERVICE_UUID "e529dbf6-8968-43e3-9930-0000000000018"
#define TOGGLE_SWITCH_UUID "19B10001-E8F2-537E-4F6C-D104768A1228"
#define BUTTON_UUID "19B10001-E8F2-537E-4F6C-D104768A1229"


// BLEService ledService("19B10000-E8F2-537E-4F6C-D104768A1228"); // Bluetooth® Low Energy LED Service

// Bluetooth® Low Energy LED Switch Characteristic - custom 128-bit UUID, read and writable by central
// BLEIntCharacteristic switchCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1228", BLERead | BLEWrite | BLENotify);
// BLEByteCharacteristic buttonCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1229", BLERead | BLEWrite | BLENotify);
BLECharacteristic requestStringacteristicUUID(TOGGLE_SWITCH_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_INDICATE | BLECharacteristic::PROPERTY_WRITE_NR);
BLEDescriptor requestStringDescriptor(BLEUUID((uint16_t)0x2902));

BLECharacteristic responseStringacteristicUUID(BUTTON_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_WRITE_NR);
BLEDescriptor responseStringDescriptor(BLEUUID((uint16_t)0x2902));

//Setup callbacks onConnect and onDisconnect
class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    isDeviceConnected = true;
  };
  void onDisconnect(BLEServer* pServer) {
    isDeviceConnected = false;
    //starts advertising once again
    BLEDevice::startAdvertising();
    Serial.println("Advertising Started again");
  }
};

class MyCallbacks: public BLECharacteristicCallbacks {

  int step = 1;

  void onWrite(BLECharacteristic *pCharacteristic) {
    std::string rxValue = pCharacteristic->getValue();
    
      responseStringacteristicUUID.setValue(rxValue);
      responseStringacteristicUUID.notify();
      // int tpp = atoi(current_tpp.c_str());
      
  }
  
};

void setup()
{
    M5.begin();             // Init M5Stack.  初始化M5Stack
    M5.Power.begin();       // Init power  初始化电源模块
    M5.Lcd.setTextSize(2);  // 设置字号大小为2
    M5.Lcd.println("      HEX Example");
    M5.Lcd.println("Display rainbow effect");
    Serial.begin(115200);
    Serial.println(std::string("Starting  HeadLight-OpenWR " + std::string(SWVERSION)).c_str());
    NimBLEDevice::init("OpenWR");
    
    /** Optional: set the transmit power, default is 3db */
#ifdef ESP_PLATFORM
    NimBLEDevice::setPower(ESP_PWR_LVL_P9); /** +9db */
#else
    NimBLEDevice::setPower(9); /** +9db */
#endif

    /** Set the IO capabilities of the device, each option will trigger a different pairing method.
     *  BLE_HS_IO_DISPLAY_ONLY    - Passkey pairing
     *  BLE_HS_IO_DISPLAY_YESNO   - Numeric comparison pairing
     *  BLE_HS_IO_NO_INPUT_OUTPUT - DEFAULT setting - just works pairing
     */
    //NimBLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_ONLY); // use passkey
    //NimBLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_YESNO); //use numeric comparison

    /** 2 different ways to set security - both calls achieve the same result.
     *  no bonding, no man in the middle protection, secure connections.
     *
     *  These are the default values, only shown here for demonstration.
     */
    //NimBLEDevice::setSecurityAuth(false, false, true);
    // NimBLEDevice::setSecurityAuth(/*BLE_SM_PAIR_AUTHREQ_BOND | BLE_SM_PAIR_AUTHREQ_MITM |*/ BLE_SM_PAIR_AUTHREQ_SC);
    // https://github.com/h2zero/NimBLE-Arduino/issues/588#issuecomment-1732479238
    NimBLEDevice::setSecurityAuth(true, false, true);
    pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(new ServerCallbacks());

    NimBLEService* pBaadService = pServer->createService(SERVICE_UUID);
    pButtonCharacteristic = pBaadService->createCharacteristic(
                                               BUTTON_UUID,
                                               NIMBLE_PROPERTY::READ |
                                               NIMBLE_PROPERTY::WRITE |
                                               NIMBLE_PROPERTY::NOTIFY
                                              );

    pButtonCharacteristic->setValue(false);
    pButtonCharacteristic->setCallbacks(&chrCallbacks);

    pToggleSwitchCharacteristic = pBaadService->createCharacteristic(
                                               TOGGLE_SWITCH_UUID,
                                               NIMBLE_PROPERTY::READ |
                                               NIMBLE_PROPERTY::WRITE |
                                               NIMBLE_PROPERTY::NOTIFY
                                              );

    pToggleSwitchCharacteristic->setValue(TOGGLE_STATE_0);
    pToggleSwitchCharacteristic->setCallbacks(&chrCallbacks);
    /** Note a 0x2902 descriptor MUST NOT be created as NimBLE will create one automatically
     *  if notification or indication properties are assigned to a characteristic.
     */

    /** Custom descriptor: Arguments are UUID, Properties, max length in bytes of the value */
   
    NimBLEDescriptor* pC01Ddsc = pButtonCharacteristic->createDescriptor(
                                               "C01D",
                                               NIMBLE_PROPERTY::READ |
                                               NIMBLE_PROPERTY::WRITE|
                                               NIMBLE_PROPERTY::WRITE_NR,
                                               512 
                                              );
    pC01Ddsc->setValue("Send it back!");
    pC01Ddsc->setCallbacks(&dscCallbacks);
    
    /** Start the services when finished creating all Characteristics and Descriptors */
    pBaadService->start();

    NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
    /** Add the services to the advertisment data **/
    pAdvertising->addServiceUUID(pBaadService->getUUID());
    /** If your device is battery powered you may consider setting scan response
     *  to false as it will extend battery life at the expense of less data sent.
     */
    // pAdvertising->setScanResponse(true);
    pAdvertising->enableScanResponse(true);
    pAdvertising->start();

    Serial.println("Advertising Started");
    // M5.dis.clear();

    // Neopixel initialization.  初始化灯带
    FastLED.addLeds<WS2811, Neopixel_PIN, GRB>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(255);  // set the LED brightness to 5. 设置灯带的亮度为5
}

void loop()
{
  /*
    void fill_gradient_RGB( CRGB* leds,
                   uint16_t startpos, CRGB startcolor,
                   uint16_t endpos,   CRGB endcolor )
    */
    if (!isBLEConnected) {
      /*
      M5.dis.drawpix(0, 0xFF00FF);
      delay(100);
      M5.dis.drawpix(0, 0x66FFFF); 
      delay(100);
      M5.dis.clear();
      */
    } else {
      // M5.dis.drawpix(0, 0x7fff00); /* GREEN */
      if (gToggleState == TOGGLE_STATE_0) {
        fill_gradient_RGB(leds,
                    0, CRGB::White,
                    NUM_LEDS,  CRGB::Yellow );
      
      } else if (gToggleState == TOGGLE_STATE_1) {
        fill_rainbow(leds, NUM_LEDS, gHue,
                  7);  // Set the leds from 0 to 37 beads to gradient rainbow color, the
                        // starting tone value to gHue, and the color difference between
                        // adjacent beads to 7.
                        // 将leds灯带的从0~37个灯珠设置为渐变彩虹色,起始色调数值设置为gHue,相邻灯珠色差为7
      
      }
      /*
      fill_rainbow(leds, NUM_LEDS, gHue,
                  7);  // Set the leds from 0 to 37 beads to gradient rainbow color, the
                        // starting tone value to gHue, and the color difference between
                        // adjacent beads to 7.
                        // 将leds灯带的从0~37个灯珠设置为渐变彩虹色,起始色调数值设置为gHue,相邻灯珠色差为7
      */
      if (gButtonState) {
        FastLED.show();  // Updated LED color.  更新LED色彩
      } else {
        FastLED.clear(true);
      }
    }

    M5.update();
  
    EVERY_N_MILLISECONDS(20)
    {
        gHue++;
    }  // The program is executed every 20 milliseconds.
       // 每20毫秒执行一次其中的程序
}
