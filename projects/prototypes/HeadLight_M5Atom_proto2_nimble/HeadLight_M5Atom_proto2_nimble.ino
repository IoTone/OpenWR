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
/**
 *  NimBLE_Server Demo:
 *
 *  Demonstrates many of the available features of the NimBLE server library.
 *
 *  Created: on March 22 2020
 *      Author: H2zero
 */
// #include "M5Atom.h"
#include <M5Stack.h>
// #include <M5Unified.h>
#include "FastLED.h"

#include <Arduino.h>
#include <NimBLEDevice.h>
#include <NimBLECharacteristic.h>

static NimBLEServer* pServer;

//
// Atom uses pin G26
// AtomU uses PIN G21
// StickC uses pin G26
// M5NanoC6 uses G2
#define Neopixel_PIN (26) // (26) // (21)
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
#define SERVICE_UUID "19B10000-E8F2-537E-4F6C-D104768A1228" //  "e529dbf6-8968-43e3-9930-0000000000018"
#define TOGGLE_SWITCH_UUID "19B10001-E8F2-537E-4F6C-D104768A1228"
#define BUTTON_UUID "19B10001-E8F2-537E-4F6C-D104768A1229"

/**  None of these are required as they will be handled by the library with defaults. **
 **                       Remove as you see fit for your needs                        */
class ServerCallbacks : public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) override {
        Serial.printf("Client address: %s\n", connInfo.getAddress().toString().c_str());

        /**
         *  We can use the connection handle here to ask for different connection parameters.
         *  Args: connection handle, min connection interval, max connection interval
         *  latency, supervision timeout.
         *  Units; Min/Max Intervals: 1.25 millisecond increments.
         *  Latency: number of intervals allowed to skip.
         *  Timeout: 10 millisecond increments.
         */
        pServer->updateConnParams(connInfo.getConnHandle(), 24, 48, 0, 180);
        isBLEConnected = true;
    }

    void onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) override {
        Serial.printf("Client disconnected - start advertising\n");
        NimBLEDevice::startAdvertising();
        isBLEConnected = false;
    }

    void onMTUChange(uint16_t MTU, NimBLEConnInfo& connInfo) override {
        Serial.printf("MTU updated: %u for connection ID: %u\n", MTU, connInfo.getConnHandle());
    }

    /********************* Security handled here *********************/
    uint32_t onPassKeyDisplay() override {
        Serial.printf("Server Passkey Display\n");
        /**
         * This should return a random 6 digit number for security
         *  or make your own static passkey as done here.
         */
        return 123456;
    }

    void onConfirmPassKey(NimBLEConnInfo& connInfo, uint32_t pass_key) override {
        Serial.printf("The passkey YES/NO number: %" PRIu32 "\n", pass_key);
        /** Inject false if passkeys don't match. */
        NimBLEDevice::injectConfirmPasskey(connInfo, true);
    }

    void onAuthenticationComplete(NimBLEConnInfo& connInfo) override {
        /** Check that encryption was successful, if not we disconnect the client */
        if (!connInfo.isEncrypted()) {
            NimBLEDevice::getServer()->disconnect(connInfo.getConnHandle());
            Serial.printf("Encrypt connection failed - disconnecting client\n");
            return;
        }

        Serial.printf("Secured connection to: %s\n", connInfo.getAddress().toString().c_str());
    }
} serverCallbacks;

/** Handler class for characteristic actions */
class CharacteristicCallbacks : public NimBLECharacteristicCallbacks {
    void onRead(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override {
        Serial.printf("%s : onRead(), value: %s\n",
               pCharacteristic->getUUID().toString().c_str(),
               pCharacteristic->getValue().c_str());
    }

    void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override {
        Serial.printf("%s : onWrite(), value: %s\n",
               pCharacteristic->getUUID().toString().c_str(),
               pCharacteristic->getValue().c_str());
    }

    /**
     *  The value returned in code is the NimBLE host return code.
     */
    void onStatus(NimBLECharacteristic* pCharacteristic, int code) override {
        Serial.printf("Notification/Indication return code: %d, %s\n", code, NimBLEUtils::returnCodeToString(code));
    }

    /** Peer subscribed to notifications/indications */
    void onSubscribe(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo, uint16_t subValue) override {
        std::string str  = "Client ID: ";
        str             += connInfo.getConnHandle();
        str             += " Address: ";
        str             += connInfo.getAddress().toString();
        if (subValue == 0) {
            str += " Unsubscribed to ";
        } else if (subValue == 1) {
            str += " Subscribed to notifications for ";
        } else if (subValue == 2) {
            str += " Subscribed to indications for ";
        } else if (subValue == 3) {
            str += " Subscribed to notifications and indications for ";
        }
        str += std::string(pCharacteristic->getUUID());

        Serial.printf("%s\n", str.c_str());
    }
} chrCallbacks;

/** Handler class for descriptor actions */
class DescriptorCallbacks : public NimBLEDescriptorCallbacks {
    void onWrite(NimBLEDescriptor* pDescriptor, NimBLEConnInfo& connInfo) override {
        std::string dscVal = pDescriptor->getValue();
        Serial.printf("Descriptor written value: %s\n", dscVal.c_str());
    }

    void onRead(NimBLEDescriptor* pDescriptor, NimBLEConnInfo& connInfo) override {
        Serial.printf("%s Descriptor read\n", pDescriptor->getUUID().toString().c_str());
    }
} dscCallbacks;

void setup(void) {
    M5.begin();             // Init M5Stack.  初始化M5Stack
    M5.Power.begin();       // Init power  初始化电源模块
    M5.Lcd.setTextSize(2);  // 设置字号大小为2
    M5.Lcd.println("      HEX Example");
    M5.Lcd.println("Display rainbow effect");
    Serial.begin(115200);
    Serial.println(std::string("Starting  HeadLight-OpenWR " + std::string(SWVERSION)).c_str());

    /** Initialize NimBLE and set the device name */
    NimBLEDevice::init(ADVERT);

    /**
     * Set the IO capabilities of the device, each option will trigger a different pairing method.
     *  BLE_HS_IO_DISPLAY_ONLY    - Passkey pairing
     *  BLE_HS_IO_DISPLAY_YESNO   - Numeric comparison pairing
     *  BLE_HS_IO_NO_INPUT_OUTPUT - DEFAULT setting - just works pairing
     */
    // NimBLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_ONLY); // use passkey
    // NimBLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_YESNO); //use numeric comparison

    /**
     *  2 different ways to set security - both calls achieve the same result.
     *  no bonding, no man in the middle protection, BLE secure connections.
     *
     *  These are the default values, only shown here for demonstration.
     */
    // NimBLEDevice::setSecurityAuth(false, false, true);

    NimBLEDevice::setSecurityAuth(/*BLE_SM_PAIR_AUTHREQ_BOND | BLE_SM_PAIR_AUTHREQ_MITM |*/ BLE_SM_PAIR_AUTHREQ_SC);
    pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(&serverCallbacks);

    NimBLEService*        pDeadService = pServer->createService("DEAD");
    NimBLECharacteristic* pBeefCharacteristic =
        pDeadService->createCharacteristic("BEEF",
                                           NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE |
                                               /** Require a secure connection for read and write access */
                                               NIMBLE_PROPERTY::READ_ENC | // only allow reading if paired / encrypted
                                               NIMBLE_PROPERTY::WRITE_ENC  // only allow writing if paired / encrypted
        );

    pBeefCharacteristic->setValue("Burger");
    pBeefCharacteristic->setCallbacks(&chrCallbacks);

    /**
     *  2902 and 2904 descriptors are a special case, when createDescriptor is called with
     *  either of those uuid's it will create the associated class with the correct properties
     *  and sizes. However we must cast the returned reference to the correct type as the method
     *  only returns a pointer to the base NimBLEDescriptor class.
     */
    NimBLE2904* pBeef2904 = pBeefCharacteristic->create2904();
    pBeef2904->setFormat(NimBLE2904::FORMAT_UTF8);
    pBeef2904->setCallbacks(&dscCallbacks);

    NimBLEService*        pBaadService = pServer->createService("BAAD");
    NimBLECharacteristic* pFoodCharacteristic =
        pBaadService->createCharacteristic("F00D", NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY);

    pFoodCharacteristic->setValue("Fries");
    pFoodCharacteristic->setCallbacks(&chrCallbacks);

    /** Custom descriptor: Arguments are UUID, Properties, max length of the value in bytes */
    NimBLEDescriptor* pC01Ddsc =
        pFoodCharacteristic->createDescriptor("C01D",
                                              NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_ENC,
                                              20);
    pC01Ddsc->setValue("Send it back!");
    pC01Ddsc->setCallbacks(&dscCallbacks);

    /** Start the services when finished creating all Characteristics and Descriptors */
    pDeadService->start();
    pBaadService->start();

    /** Create an advertising instance and add the services to the advertised data */
    NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->setName(ADVERT);
    pAdvertising->addServiceUUID(pDeadService->getUUID());
    pAdvertising->addServiceUUID(pBaadService->getUUID());
    /**
     *  If your device is battery powered you may consider setting scan response
     *  to false as it will extend battery life at the expense of less data sent.
     */
    pAdvertising->enableScanResponse(true);
    pAdvertising->start();

    Serial.printf("Advertising Started\n");

    // Neopixel initialization.  初始化灯带
    FastLED.addLeds<WS2811, Neopixel_PIN, GRB>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(255);  // set the LED brightness to 5. 设置灯带的亮度为5

}

void loop() {
    /** Loop here and send notifications to connected peers */
    // delay(2000);

    /*
    if (pServer->getConnectedCount()) {
        NimBLEService* pSvc = pServer->getServiceByUUID("BAAD");
        if (pSvc) {
            NimBLECharacteristic* pChr = pSvc->getCharacteristic("F00D");
            if (pChr) {
                pChr->notify();
            }
        }
    }
    */

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
      FastLED.clear(true);
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
