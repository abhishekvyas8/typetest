// testshapes demo for Adafruit RGBmatrixPanel library.
// Demonstrates the drawing abilities of the RGBmatrixPanel library.
// For 32x32 RGB LED matrix:
// http://www.adafruit.com/products/607

// Written by Limor Fried/Ladyada & Phil Burgess/PaintYourDragon
// for Adafruit Industries.
// BSD license, all text above must be included in any redistribution.
#include <Wire.h>

#include <Adafruit_GFX.h>   // Core graphics library
#include <RGBmatrixPanel.h> // Hardware-specific library

// If your 32x32 matrix has the SINGLE HEADER input,
// use this pinout:
#define CLK 11  // MUST be on PORTB! (Use pin 11 on Mega)
#define OE  9
#define LAT 10
#define A   A0
#define B   A1
#define C   A2
#define D   A3
// If your matrix has the DOUBLE HEADER input, use:
//#define CLK 8  // MUST be on PORTB! (Use pin 11 on Mega)
//#define LAT 9
//#define OE  10
//#define A   A3
//#define B   A2
//#define C   A1
//#define D   A0
RGBmatrixPanel matrix(A, B, C, D, CLK, LAT, OE, false, 64);
static int gflag = 0;
static int rflag = 0;
static int wflag = 0;

void setup() {
  // put your setup code here, to run once:
  Wire.begin(0x42);
  Wire.onReceive(receiveEvent);  
    
  matrix.begin();
  
  // fill the screen with 'black'
  matrix.fillScreen(matrix.Color333(0, 0, 0));
  
  // draw some text!
  matrix.setCursor(1, 0);    // start at top left, with one pixel of spacing
  matrix.setTextSize(1);     // size 1 == 8 pixels high
  matrix.setTextWrap(true); // Don't wrap at end of line - will do ourselves

  matrix.setTextColor(matrix.Color333(7,7,7));
  // whew!
}

void loop() {
  // do nothing
}

void receiveEvent(int howMany){
  while(Wire.available()){
    char c = Wire.read();
    if(c == '&'){
      matrix.setCursor(1, 0); 
      matrix.fillScreen(matrix.Color333(0, 0, 0));
    }
    else if(c == '$'){
      gflag++;
      if(gflag == 2){
        gflag = 0;
      }
    }
    else if(c == '%'){
      rflag++;
      if(rflag == 2){
        rflag = 0;
      }
    }
    else if(c == '#'){
      wflag++;
      if(wflag == 2){
        wflag = 0;
      }
    }
    else{
      if(gflag == 1 && rflag != 1){
        matrix.setTextColor(matrix.Color333(0,7,0));
        matrix.print(c);
      }
      else if(rflag == 1 && gflag != 1){
        matrix.setTextColor(matrix.Color333(7,0,0));
        matrix.print(c);
      }
      else if(wflag == 1 && gflag != 1 && rflag != 1 ){
        int      x, y, hue;
        float    dx, dy, d;
        uint8_t  sat, val;
        uint16_t c;
        matrix.setCursor(32, 0);
        for(y=0; y < matrix.width(); y++) {
          dy = 15.5 - (float)y;
          for(x=0; x < matrix.height(); x++) {
            dx = 15.5 - (float)x;
            d  = dx * dx + dy * dy;
            if(d <= (16.5 * 16.5)) { // Inside the circle(ish)?
              hue = (int)((atan2(-dy, dx) + PI) * 1536.0 / (PI * 2.0));
              d = sqrt(d);
              if(d > 15.5) {
                // Do a little pseudo anti-aliasing along perimeter
                sat = 255;
                val = (int)((1.0 - (d - 15.5)) * 255.0 + 0.5);
              } else
              {
                // White at center
                sat = (int)(d / 15.5 * 255.0 + 0.5);
                val = 255;
              }
              c = matrix.ColorHSV(hue, sat, val, true);
            } else {
              c = 0;
            }
            matrix.drawPixel(x, y, c);
          }
        }
      }
      else{
        matrix.setTextColor(matrix.Color333(0,0,7));
        matrix.print(c);
      }
    }
  }
}
