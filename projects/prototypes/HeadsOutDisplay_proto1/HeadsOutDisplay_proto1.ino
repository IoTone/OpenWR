/**
 * 2024 IoTone Inc.
 */
// SPDX-FileCopyrightText: 2022 Phil B. for Adafruit Industries
//
// SPDX-License-Identifier: MIT

// Example for 5x5 NeoBFF - scrolls a message across the LED matrix.
// Requires Adafruit_GFX, Adafruit_NeoPixel and Adafruit_NeoMatrix libraries.
/*
  HeadsOutDisplay_proto1

  Goal of PoC 1 is to incoporate a BLE device that can display arbitrary logo or text
  externally on an LED matrix.

  switch characteristic is a toggle switch intended to be multistate to change the matrix display
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
*/
#include <Adafruit_GFX.h>       // Graphics library
#include <Adafruit_NeoPixel.h>  // NeoPixel library
#include <Adafruit_NeoMatrix.h> // Bridges GFX and NeoPixel
#include <ArduinoBLE.h>
#include <Fonts/TomThumb.h>     // A tiny 3x5 font incl. w/GFX



#include "RGB.h"




BLEService ledService("19B10000-E8F2-537E-4F6C-D104768A1228"); // Bluetooth® Low Energy LED Service

// Bluetooth® Low Energy LED Switch Characteristic - custom 128-bit UUID, read and writable by central
BLEIntCharacteristic switchCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1228", BLERead | BLEWrite);
BLEByteCharacteristic buttonCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1229", BLERead | BLEWrite | BLENotify);


#define PIN A3

#define HEADSOUT_AVAILABLE      0
#define HEADSOUT_DONOTDISTURB   1
#define HEADSOUT_INMEETING      2
#define HEADSOUT_RELAXING       3
#define MATRIX_W                5
#define MATRIX_H                5

const int ledPin = LED_BUILTIN; // pin to use for the LED

// const int BUTTON_PIN = 0; // the number of the pushbutton pin
// int lastState = HIGH; // the previous state from the input pin
// int currentState;    // the current reading from the input pin
int matrixIsOn = 1; // Default on
int matrixBrightness = 32; // What is the scale?
int matrixHeadsOutMode = HEADSOUT_RELAXING;
int lastMatrixHeadsOutMode = matrixHeadsOutMode;

// NeoMatrix declaration for BFF with the power and
// Neo pins at the top (same edge as QT Py USB port):
Adafruit_NeoMatrix matrix(5, 5, PIN,
  NEO_MATRIX_TOP  + NEO_MATRIX_RIGHT +
  NEO_MATRIX_ROWS + NEO_MATRIX_PROGRESSIVE,
  NEO_GRB         + NEO_KHZ800);

// Fill the pixels one after the other with a color
void colorWipe(RGB color, uint8_t wait) {
  for(uint16_t row=0; row < 8; row++) {
    for(uint16_t column=0; column < 8; column++) {
      matrix.drawPixel(column, row, matrix.Color(color.r, color.g, color.b));
      matrix.show();
      delay(wait);
    }
  }
}

// Fade pixel (x, y) from startColor to endColor
void fadePixel(int x, int y, RGB startColor, RGB endColor, int steps, int wait) {
  for(int i = 0; i <= steps; i++) 
  {
     int newR = startColor.r + (endColor.r - startColor.r) * i / steps;
     int newG = startColor.g + (endColor.g - startColor.g) * i / steps;
     int newB = startColor.b + (endColor.b - startColor.b) * i / steps;
     
     matrix.drawPixel(x, y, matrix.Color(newR, newG, newB));
     matrix.show();
     delay(wait);
  }
}

// Crossfade entire screen from startColor to endColor
void crossFade(RGB startColor, RGB endColor, int steps, int wait) {
  for(int i = 0; i <= steps; i++)
  {
     int newR = startColor.r + (endColor.r - startColor.r) * i / steps;
     int newG = startColor.g + (endColor.g - startColor.g) * i / steps;
     int newB = startColor.b + (endColor.b - startColor.b) * i / steps;
     
     matrix.fillScreen(matrix.Color(newR, newG, newB));
     matrix.show();
     delay(wait);
  }
}

void drawLogo() {
  // This 8x8 array represents the LED matrix pixels. 
  // A value of 1 means we’ll fade the pixel to white
  int logo[8][8] = {  
   {0, 0, 0, 0, 0, 0, 0, 0},
   {0, 1, 1, 0, 0, 1, 1, 0},
   {0, 1, 1, 0, 0, 1, 1, 0},
   {0, 0, 0, 0, 0, 0, 0, 0},
   {0, 0, 0, 0, 0, 0, 0, 0},
   {0, 1, 1, 0, 0, 1, 1, 0},
   {0, 1, 1, 0, 0, 1, 1, 0},
   {0, 0, 0, 0, 0, 0, 0, 0}
  };
   
  for(int row = 0; row < 8; row++) {
    for(int column = 0; column < 8; column++) {
     if(logo[row][column] == 1) {
       fadePixel(column, row, red, white, 120, 0);
     }
   }
  }
}

void drawAvailableIcon() {
  // This 8x8 array represents the LED matrix pixels. 
  // A value of 1 means we’ll fade the pixel to white
  int logo[MATRIX_W][MATRIX_H] = {  
   {1, 1, 1, 1, 1},
   {1, 1, 1, 1, 1},
   {1, 1, 0, 1, 1},
   {1, 1, 1, 1, 1},
   {1, 1, 1, 1, 1}
  };
   
  for(int row = 0; row < MATRIX_H; row++) {
    for(int column = 0; column < MATRIX_W; column++) {
     if(logo[row][column] == 1) {
       fadePixel(column, row, white, rgb_green, 120, 0);
     }
   }
  }
}

void drawDoNotDisturbIcon() {
  // This 8x8 array represents the LED matrix pixels. 
  // A value of 1 means we’ll fade the pixel to white
  int logo[MATRIX_W][MATRIX_H] = {  
   {1, 0, 0, 0, 1},
   {0, 1, 0, 1, 0},
   {0, 0, 1, 0, 0},
   {0, 1, 0, 1, 0},
   {1, 0, 0, 0, 1}
  };
   
  for(int row = 0; row < MATRIX_H; row++) {
    for(int column = 0; column < MATRIX_W; column++) {
     if(logo[row][column] == 1) {
       fadePixel(column, row, rgb_blue, rgb_red, 25, 0);
     }
   }
  }
}

void drawInMeetingIcon() {
  // This 8x8 array represents the LED matrix pixels. 
  // A value of 1 means we’ll fade the pixel to white
  int logo[MATRIX_W][MATRIX_H] = {  
   {1, 1, 1, 0, 0},
   {0, 1, 1, 0, 0},
   {0, 1, 1, 0, 0},
   {0, 1, 1, 0, 0},
   {1, 1, 1, 0, 0}
  };
   
  for(int row = 0; row < MATRIX_H; row++) {
    for(int column = 0; column < MATRIX_W; column++) {
     if(logo[row][column] == 1) {
       fadePixel(column, row, rgb_green, rgb_yellow, 25, 0);
     }
   }
  }
}

void scrollText(String textToDisplay) {
  int x = matrix.width();
  
  // Account for 6 pixel wide characters plus a space
  int pixelsInText = textToDisplay.length() * 7;
  
  matrix.setCursor(x, 0);
  matrix.print(textToDisplay);
  matrix.show();
  
  while(x > (matrix.width() - pixelsInText)) {
    matrix.fillScreen(matrix.Color(red.r, red.g, red.b));
    matrix.setCursor(--x, 0);
    matrix.print(textToDisplay);
    matrix.show();
    delay(150);
  }
}

void setup() {
  // TODO: consider adding an external button or two ... this would help if we want to turn on/off, etc.
  // set LED pin to output mode
  // pinMode(ledPin, OUTPUT);

  // initialize digital pin LED_BUILTIN as an output.
  
  // pinMode(LED_BUILTIN, OUTPUT);
  // pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy module failed!");

    while (1);
  }

  // set advertised local name and service UUID:
  BLE.setLocalName("HeadsOut_OpenWR");
  BLE.setAdvertisedService(ledService);

  // add the characteristic to the service
  ledService.addCharacteristic(switchCharacteristic);
  ledService.addCharacteristic(buttonCharacteristic);
  // add service
  BLE.addService(ledService);

  // set the initial value for the characeristic:
  switchCharacteristic.writeValue(0);
  buttonCharacteristic.writeValue(matrixIsOn); // TODO: we don't really have a button, but maybe this is a VR button
  
  // start advertising
  BLE.advertise();

  Serial.println("BLE NeoMatrix Peripheral");

  // Matrix code
  matrix.begin();
  matrix.setBrightness(matrixIsOn);
  matrix.setTextColor( matrix.Color(255, 255, 255) );
  matrix.setTextWrap(false);
}

void loop() {
  // listen for Bluetooth® Low Energy peripherals to connect:
  BLEDevice central = BLE.central();

  // if a central is connected to peripheral:
  if (central) {
    Serial.print("Connected to central: ");
    // print the central's MAC address:
    Serial.println(central.address());
    // while the central is still connected to peripheral:
    // If we use the while loop, you can't update anything visually on the device!
    while(central.connected()) {
      if (switchCharacteristic.written()) {
        // Just save the state
        // switchCharacteristic.readValue(&matrixHeadsOutMode,2);
        //  matrixHeadsOutMode = switchCharacteristic.read();
        /*
        const char* buffer = switchCharacteristic.value();
        matrixHeadsOutMode = *(int*)buffer;
        Serial.print("HeadsOut Value written:");
        Serial.print(matrixHeadsOutMode);
        Serial.print("\n");
        */
        /*
        if (switchCharacteristic.value()) {   
          // Available mode
          matrixHeadsOutMode = HEADSOUT_AVAILABLE;
          // digitalWrite(ledPin, LOW); // changed from HIGH to LOW       
        } else {             
           matrixHeadsOutMode = HEADSOUT_DONOTDISTURB;                 
          // Serial.println(F("LED off"));
          // digitalWrite(ledPin, HIGH); // changed from LOW to HIGH     
        }
        */

        byte value = 0;
        switchCharacteristic.readValue(value);
        
        lastMatrixHeadsOutMode = matrixHeadsOutMode;
        if (value == HEADSOUT_AVAILABLE) {
          matrixHeadsOutMode = HEADSOUT_AVAILABLE;
        } else if (value == HEADSOUT_DONOTDISTURB) {
          matrixHeadsOutMode = HEADSOUT_DONOTDISTURB;
        } else if (value == HEADSOUT_INMEETING) {
          matrixHeadsOutMode = HEADSOUT_INMEETING;
        } else if (value == HEADSOUT_RELAXING) {
          matrixHeadsOutMode = HEADSOUT_RELAXING;
        } else {
          // Ignore
        }
        /*
        if (value & 0x00) {
          matrixHeadsOutMode = HEADSOUT_AVAILABLE;
        } else if (value & 0x01) {
          matrixHeadsOutMode = HEADSOUT_DONOTDISTURB;
        } else if (value & 0x02) {
          matrixHeadsOutMode = HEADSOUT_INMEETING;
        } else if (value & 0x03) {
          matrixHeadsOutMode = HEADSOUT_RELAXING;
        } else {
          // Ignore
        } */
        Serial.print("HeadsOut Value written:");
        Serial.print(value);
        Serial.print(" vs ");
        Serial.print(matrixHeadsOutMode);
        Serial.print("\n");
      }

      if (buttonCharacteristic.written()) {
        if (buttonCharacteristic.value()) {   
          Serial.println("MATRIX on");
          matrix.show();
          matrixBrightness = 32;
          matrix.setBrightness(matrixBrightness);
        } else {                              
          Serial.println(F("MATRIX off"));
          matrixBrightness = 0;
          matrix.setBrightness(matrixBrightness);
        }
      }

      

      // read the state of the switch/button:
      /*
      currentState = digitalRead(BUTTON_PIN);
    
      if(lastState == LOW && currentState == HIGH) {
        Serial.println("The state changed from LOW to HIGH");
        
      }
        
      // save the last state
      if (currentState != lastState) {
        buttonCharacteristic.writeValue(lastState);
      }
      lastState = currentState;
      */
    }

    // when the central disconnects, print it out:
    Serial.print(F("Disconnected from central: "));
    Serial.println(central.address());
  }

  if (matrixIsOn) {

    if (lastMatrixHeadsOutMode != matrixHeadsOutMode) {
      colorWipe(off, 10);
      lastMatrixHeadsOutMode = matrixHeadsOutMode;
    }
    if (matrixHeadsOutMode == HEADSOUT_AVAILABLE) {
      matrix.show();
      drawAvailableIcon();

    } else if (matrixHeadsOutMode == HEADSOUT_DONOTDISTURB) {
      matrix.show();
      crossFade(off, white, 50, 5);
      delay(500);

      colorWipe(red, 50);
      delay(500);
    } else if (matrixHeadsOutMode == HEADSOUT_INMEETING) {
      matrix.show();
      drawInMeetingIcon();
    } else if (matrixHeadsOutMode == HEADSOUT_RELAXING) {
      matrix.show();
      // crossFade(off, rgb_pink, 25, 5);
      colorWipe(rgb_blue, 50);
      delay(200);

      colorWipe(white, 18);
      delay(100);
    }
  }
  delay(100);
  /*
  // read the state of the switch/button:
  currentState = digitalRead(BUTTON_PIN);

  if(lastState == LOW && currentState == HIGH)
    Serial.println("The state changed from LOW to HIGH");

  // save the last state
  lastState = currentState;
  */
  /*
  crossFade(off, white, 50, 5);
  delay(1000);

  colorWipe(red, 50);
  delay(1000);

  drawLogo();
  delay(2000);

  matrix.fillScreen(matrix.Color(red.r, red.g, red.b));
  matrix.show();
  String twitterHandle = "@brentschooley";
  scrollText(twitterHandle);
  scrollText(twitterHandle);
  delay(500);

  crossFade(red, white, 120, 5);
  crossFade(white, off, 120, 5);
  delay(1000);
  */
  
}








