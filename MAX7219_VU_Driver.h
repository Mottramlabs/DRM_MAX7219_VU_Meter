/***************************************************************************************************************************/
/*
  6th November 2019 - David Mottram
  Dual VU Meter Using a MAX7219
  Library for the MAX7219 display on the "ESP8266 Display Board Version 1"
  Since there is only one MAX7219 this can be very simple, no need to allow for more than one chip
*/
/***************************************************************************************************************************/

// Check that the library has not yet been called
#ifndef Max7219_Lib
#define Max7219_Lib

// Check of legacy IDE
#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

// Include needed library and files
#include <SPI.h>

// below is when the library is local
#include "7_Segment_Font.h"

/***************************************************************************************************************************/
/*
  Class for writing to the MAX7219 display
*/
/***************************************************************************************************************************/
class MAX7219_VU_Driver
{

  public:

    // Add a constructor
    MAX7219_VU_Driver(int Pin);

    // prototypes of the functions
    void MAX7219_Chip(uint8_t Address, uint8_t Data);
    void Display_Clear(int _Brightness);
    void Bar_Display_Update(int _Left_Bar, int _Right_Bar, int _Brightness);
    void LED_Animate(int mode, int animate_speed, int _Brightness);
    void Display_Update(int _Brightness);
    void String_To_Buffer(String Severn_Seg, int Brightness);

  private:

    // Private version of the CS Pin and number of MAX chips in use
    int _Pin;


// is this needed for 7-segment display?????
    // sufficient space for 8off MAX7214 drivers or a 64 digit display. An UNO has 2048 bytes available for local variables
    uint8_t _Frame_Buffer[4];

    // MAX7219 register list
    #define MAX7219_Noop          0x0
    #define MAX7219_DecodeMode    0x9
    #define MAX7219_Intensity     0xA
    #define MAX7219_ScanLimit     0xB
    #define MAX7219_Shutdown      0xC
    #define MAX7219_Displaytest   0xF

    // bar graph items
    #define Left_LSB_Reg          0x5
    #define Left_MSB_Reg          0x6
    #define Right_LSB_Reg         0x7
    #define Right_MSB_Reg         0x8

};
#endif
