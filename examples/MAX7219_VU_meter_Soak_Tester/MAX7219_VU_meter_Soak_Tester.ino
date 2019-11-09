/*
      Product:  Stereo LED bar graph Soak Tester
      Date:     6th November 2019
      Version:  1.0
      Board:	Arduino UNO
      Author:   David Mottram
*/

// add MAX7219 driver
#include <MAX7219_VU_Driver.h>
#define MAX7219_CS 10
int Brill = 0x0F;
// make an instance of MAX7219_Digit called My_Display and set CS pin
MAX7219_VU_Driver My_Display(MAX7219_CS);

// VU meter items
#define LED_Segments 16                   // number of LEDs per channel
#define Test_Loops 5                      // number of loops during test

void setup() {

  Serial.begin(115200);
  Serial.println(""); Serial.println("Hello");

  // display startup
  My_Display.Display_Clear(Brill);

} // end setup


void loop() {

  // Test the VU display
  for (int yy = 0; yy < Test_Loops; yy++) {
    My_Display.String_To_Buffer("tE5t", Brill);
    My_Display.LED_Animate(1, 25, 0xF); // mode, speed, brightness
    My_Display.LED_Animate(2, 25, 0xF); // mode, speed, brightness
    My_Display.LED_Animate(3, 25, 0xF); // mode, speed, brightness
    delay(100);
  } // end VU test

  // All LED's on and brightness test
  My_Display.String_To_Buffer("8.8.8.8.", 0x00);
  for (int yy = 0; yy < Test_Loops; yy++) {
    for (int zz = 0; zz < 0x10; zz++) {
      My_Display.Bar_Display_Update(0xFFFF, 0xFFFF, zz);
      delay(200);
    }
  } // end brightness test

  // random display
  My_Display.String_To_Buffer("rAND", 0x00);

  for  (int yy = 0; yy < 1000; yy++) {

    long aa = random(0x0000, 0xFFFF);
    long bb = random(0x0000, 0xFFFF);
    My_Display.Bar_Display_Update(aa, bb, Brill);
    delay(25);

  }

} // end loop
