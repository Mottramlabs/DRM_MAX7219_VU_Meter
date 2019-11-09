/*
      Product:  Stereo LED bar graph using an Arduino and a MAX7219 VU display
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
// pins for Bar segments
#define VU_Left A0                        // analog input pin
#define VU_Right A1                       // analog input pin
#define LED_Pause 2                       // LED lite time between multiplexing left/right
#define Mode_Select 7                     // Mode select input pin
// VU levels, dB values
//const int SegmentValue[] = { 20, 50, 60, 70, 89, 100, 158, 223, 281, 354, 446, 500, 561, 629, 706, 792 };
const int SegmentValue[] = { 28, 50, 89, 126, 158, 223, 281, 315, 354, 397, 446, 500, 561, 629, 706, 792 };
#define Mean_Samples 25//15               // number of analog samples
float Mean_VU_Left[Mean_Samples];         // array used for VU samples
float Mean_VU_Right[Mean_Samples];        // array used for VU samples
// A/D variables
int16_t Analog_value = 0;
long VU_Calc = 0;
long VU_Display = 0;
// peak variable
int16_t Peak_Left = 0;
int16_t Peak_Right = 0;
#define Peak_Decay 2//4                   // the larger the number the faster the peak decay
int16_t Bar_Left = 0x00;
int16_t Bar_Right = 0x00;
int Mode = 0;                             // mode, peak hold LED or normal. 1=peak

// for 7-segment demo
#define Digit A2                          // analog input pin
float Severn_Seg_Display[Mean_Samples];   // array used for 7 segment display samples


void setup() {

  Serial.begin(115200);
  Serial.println(""); Serial.println("Hello");

  // display startup
  My_Display.Display_Clear(Brill);
  My_Display.String_To_Buffer("H1 ", Brill);

  // discover peak/normal mode. Pulling Mode_Select input low then peak mode is selected
  pinMode(Mode_Select, INPUT_PULLUP);
  delay(100);
  int Link = digitalRead(Mode_Select);
  // is the link if place (to ground)
  if ( Link == 0) {
    Mode = 1;                       // select peak hold mode
  }
  else {
    Mode = 0;                       // no peak hold mode
  }

  // LED demo
  My_Display.LED_Animate(1, 25, 0xF); // mode, speed, brightness
  My_Display.LED_Animate(2, 25, 0xF); // mode, speed, brightness
  My_Display.LED_Animate(3, 25, 0x8); // mode, speed, brightness
  // clear display
  My_Display.Display_Clear(Brill);
  delay(1000);

} // end setup


void loop() {

  /* ************************************************************************************************************************************ */
  // demo 7-segment display, just display the A/D input as 0-5V. Again using many samples to get an mean value

  // increment array contents
  for (int zz = (Mean_Samples - 2); zz > -1; zz--) {
    float yy = Severn_Seg_Display[zz];
    Severn_Seg_Display[zz + 1] = yy;
  }
  // place new value in position 0
  Severn_Seg_Display[0] = analogRead(Digit);

  /* ------------------------------------------------------------------ */
  // calculate mean value
  long Display_Calc = 0;
  for (int zz = 0; zz < Mean_Samples; zz++) {
    Display_Calc = Display_Calc + Severn_Seg_Display[zz];
  }
  // compose and display result
  float Volts = (Display_Calc / Mean_Samples);
  Volts = map(Volts, 0, 1023, 0, 5000);
  Volts = Volts / 1000;
  My_Display.String_To_Buffer(String(Volts, 3), Brill);


  /* ************************************************************************************************************************************ */
  // do left side VU
  /* ------------------------------------------------------------------ */
  // increment array contents
  for (int zz = (Mean_Samples - 2); zz > -1; zz--) {
    float yy = Mean_VU_Left[zz];
    Mean_VU_Left[zz + 1] = yy;
  }
  // place new value in position 0
  Mean_VU_Left[0] = (analogRead(VU_Left) * 3);

  /* ------------------------------------------------------------------ */
  // calculate mean value
  VU_Calc = 0;
  for (int zz = 0; zz < Mean_Samples; zz++) {
    VU_Calc = VU_Calc + Mean_VU_Left[zz];
  }
  VU_Display = (VU_Calc / Mean_Samples);

  /* ------------------------------------------------------------------ */
  // peak hold value
  if (VU_Display > Peak_Left) {
    Peak_Left = VU_Display;
  }
  // decay the peak value
  else {
    for (int zz = Peak_Decay; zz > 0; zz--) {
      Peak_Left--;
    }
    
  } // end of decay

  /* ------------------------------------------------------------------ */
  // display bar graph
  for (int zz = 0; zz < LED_Segments; zz++) {

    if (VU_Display >= SegmentValue[zz]) {
      bitSet(Bar_Left, zz);
    }
    else {
      bitClear(Bar_Left, zz);
    }

  } // end loop

  /* ------------------------------------------------------------------ */
  // add peak LED
  for (int zz = 0; zz < LED_Segments; zz++) {

    if (Peak_Left >= SegmentValue[zz] && Peak_Left <= SegmentValue[zz + 1]) {
      if (Mode == 1) {
        bitSet(Bar_Left, zz);
      }
    }

  } // end loop


  /* ************************************************************************************************************************************ */
  // do right side VU
  /* ------------------------------------------------------------------ */
  // increment array contents
  for (int zz = (Mean_Samples - 2); zz > -1; zz--) {
    float yy = Mean_VU_Right[zz];
    Mean_VU_Right[zz + 1] = yy;
  }
  // place new value in position 0
  Mean_VU_Right[0] = (analogRead(VU_Right) * 3);

  /* ------------------------------------------------------------------ */
  // calculate mean value
  VU_Calc = 0;
  for (int zz = 0; zz < Mean_Samples; zz++) {
    VU_Calc = VU_Calc + Mean_VU_Right[zz];
  }
  VU_Display = (VU_Calc / Mean_Samples);

  /* ------------------------------------------------------------------ */
  // peak hold value
  if (VU_Display > Peak_Right) {
    Peak_Right = VU_Display;
  }
  // decay the peak value
  else {
    for (int zz = Peak_Decay; zz > 0; zz--) {
      Peak_Right--;
    }
  } // end of decay

  /* ------------------------------------------------------------------ */
  // display bar graph
  for (int zz = 0; zz < LED_Segments; zz++) {

    if (VU_Display >= SegmentValue[zz]) {
      bitSet(Bar_Right, zz);
    }
    else {
      bitClear(Bar_Right, zz);
    }

  } // end loop

  /* ------------------------------------------------------------------ */
  // add peak LED
  for (int zz = 0; zz < LED_Segments; zz++) {

    if (Peak_Right >= SegmentValue[zz] && Peak_Right <= SegmentValue[zz + 1]) {
      if (Mode == 1) {
        bitSet(Bar_Right, zz);
      }
    }

  } // end loop

  // update the display and pause to reduce flicker
  My_Display.Bar_Display_Update(Bar_Left, Bar_Right, Brill);
  delay(LED_Pause);

} // end loop
