/**
 * 2024 IoTone Inc.
 */
// SPDX-FileCopyrightText: 2022 Phil B. for Adafruit Industries
//
// SPDX-License-Identifier: MIT

// Example for 5x5 NeoBFF - scrolls a message across the LED matrix.
// Requires Adafruit_GFX, Adafruit_NeoPixel and Adafruit_NeoMatrix libraries.

#include <Adafruit_GFX.h>       // Graphics library
#include <Adafruit_NeoPixel.h>  // NeoPixel library
#include <Adafruit_NeoMatrix.h> // Bridges GFX and NeoPixel
#include <Fonts/TomThumb.h>     // A tiny 3x5 font incl. w/GFX



#include "RGB.h"

#define PIN A3
// NeoMatrix declaration for BFF with the power and
// Neo pins at the top (same edge as QT Py USB port):
Adafruit_NeoMatrix matrix(5, 5, PIN,
  NEO_MATRIX_TOP  + NEO_MATRIX_RIGHT +
  NEO_MATRIX_ROWS + NEO_MATRIX_PROGRESSIVE,
  NEO_GRB         + NEO_KHZ800);

int 
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
  matrix.begin();
  matrix.setBrightness(30);
  matrix.setTextColor( matrix.Color(255, 255, 255) );
  matrix.setTextWrap(false);
}

void loop() {
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