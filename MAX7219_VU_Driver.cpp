#include "Arduino.h"
#include <SPI.h>
#include "MAX7219_VU_Driver.h"


// ------------------------------------------------------------------------------------------------------------------------
// constructor
// ------------------------------------------------------------------------------------------------------------------------
MAX7219_VU_Driver::MAX7219_VU_Driver(int Pin) {

  // local variables
  _Pin = Pin;

  // set load pin to output
  pinMode(_Pin, OUTPUT);

  // reverse the SPI transfer to send the MSB first
  SPI.setBitOrder(MSBFIRST);

  // start SPI, SPI slow on ESP8266 https://github.com/esp8266/Arduino/issues/2624
  SPI.begin();

  digitalWrite(_Pin, HIGH);  // Take CS High

#if defined(__AVR_ATmega168__) ||defined(__AVR_ATmega168P__) ||defined(__AVR_ATmega328P__)
  Serial.println("Hello UNO");
#else
  // only needed for ESP8266, does not work with UNO, the larger the number the faster the SPI
  Serial.println("Hello ESP8266");
  SPI.setFrequency(8000000L);
#endif


  // clear the frame buffer
  for (int zz = 0; zz < 8; zz++) {
    _Frame_Buffer[zz] = 0x00;
  } // end of flush

} // end of function


// ------------------------------------------------------------------------------------------------------------------------
// write data to MAX7219 chip.
// ------------------------------------------------------------------------------------------------------------------------
void MAX7219_VU_Driver::MAX7219_Chip(uint8_t Address, uint8_t Data) {

  // take CS low
  digitalWrite(_Pin, LOW);

  SPI.transfer(Address);      //  address
  SPI.transfer(Data);         // data value

  // return CS high
  digitalWrite(_Pin, HIGH);

} // end Function



// ------------------------------------------------------------------------------------------------------------------------
// set control registers and clear all the data register
// ------------------------------------------------------------------------------------------------------------------------
void MAX7219_VU_Driver::Display_Clear(int _Brightness) {

  // range check the brightess level, valid 0-15
  if (_Brightness < 0 || _Brightness > 15) {

    _Brightness = 15;

  } // end range check

  // write to the MAX7219 chip control registers
  MAX7219_Chip(MAX7219_DecodeMode, 0x00);
  MAX7219_Chip(MAX7219_Intensity, _Brightness);
  MAX7219_Chip(MAX7219_ScanLimit, 0x07);
  MAX7219_Chip(MAX7219_Displaytest, 0x00);
  MAX7219_Chip(MAX7219_Shutdown, 0x01);

  // clear data registers
  for (int zz = 1; zz < 9; zz++ ) {
    MAX7219_Chip(zz, 0x00);
  }
}  // end function


// ------------------------------------------------------------------------------------------------------------------------
// write a string Severn_Seg to the data buffer
// ------------------------------------------------------------------------------------------------------------------------
void MAX7219_VU_Driver::Bar_Display_Update(int _Left_Bar, int _Right_Bar, int _Brightness) {

  // range check the brightess level, valid 0-15
  if (_Brightness < 0 || _Brightness > 15) {

    _Brightness = 15;

  } // end range check

  // write to bar graph
  MAX7219_Chip(Left_MSB_Reg, highByte(_Left_Bar));
  MAX7219_Chip(Left_LSB_Reg, lowByte(_Left_Bar));
  MAX7219_Chip(Right_MSB_Reg, highByte(_Right_Bar));
  MAX7219_Chip(Right_LSB_Reg, lowByte(_Right_Bar));
  // re-write control registers
  MAX7219_Chip(MAX7219_DecodeMode, 0x00);
  MAX7219_Chip(MAX7219_Intensity, _Brightness);
  MAX7219_Chip(MAX7219_ScanLimit, 0x07);
  MAX7219_Chip(MAX7219_Displaytest, 0x00);
  MAX7219_Chip(MAX7219_Shutdown, 0x01);

}  // end function


// ------------------------------------------------------------------------------------------------------------------------
// update the display with the data in the Frame_Buffer, note that Frame_Buffer[0] is far left of the display
// ------------------------------------------------------------------------------------------------------------------------
void MAX7219_VU_Driver::Display_Update(int _Brightness) {

  // range check the brightess level, valid 0-15
  if (_Brightness < 0 || _Brightness > 15) {

    _Brightness = 15;

  } // end range check

  // write to the MAX7219 chip control registers
  MAX7219_Chip(MAX7219_DecodeMode, 0x00);
  MAX7219_Chip(MAX7219_Intensity, _Brightness);
  MAX7219_Chip(MAX7219_ScanLimit, 0x07);
  MAX7219_Chip(MAX7219_Displaytest, 0x00);
  MAX7219_Chip(MAX7219_Shutdown, 0x01);

  // write to the MAX7219 display registers 1-4 (digits 1-4)
  for (int i = 0; i <  4; i++) {

    MAX7219_Chip((i + 1), _Frame_Buffer[i]); // Write to column, with data

  } // end of loop

} // end of function


// VU display animation
void MAX7219_VU_Driver::LED_Animate(int mode, int animate_speed, int _Brightness) {

  // range check the brightess level, valid 0-15
  if (_Brightness < 0 || _Brightness > 15) {

    _Brightness = 15;

  } // end range check

  int xx = 1;
  for (int zz = 0; zz < 16; zz++) {
    bitSet(xx, zz);
    if (mode == 1) {
      Bar_Display_Update(xx, 0x00, _Brightness);
    }
    if (mode == 2) {
      Bar_Display_Update(0x00, xx, _Brightness);
    }
    if (mode == 3) {
      Bar_Display_Update(xx, xx, _Brightness);
    }
    delay(animate_speed);
  }
  for (int zz = 15; zz > -1; zz--) {
    bitClear(xx, zz);
    if (mode == 1) {
      Bar_Display_Update(xx, 0x00, _Brightness);
    }
    if (mode == 2) {
      Bar_Display_Update(0x00, xx, _Brightness);
    }
    if (mode == 3) {
      Bar_Display_Update(xx, xx, _Brightness);
    }
    delay(animate_speed);
  }
} // end of function

// ------------------------------------------------------------------------------------------------------------------------
// write a string Severn_Seg to the data buffer
// ------------------------------------------------------------------------------------------------------------------------
void MAX7219_VU_Driver::String_To_Buffer(String Severn_Seg, int _Brightness) {

  // clear the data buffer
  for (int a = 0; a < 4; a++) {
    // blank digit
    _Frame_Buffer[a] = 0x00;
  } // end of loop

  // define a buffer pointer and decimal point flag
  int Buffer_Pointer = 3; int DP_Flag = 0;

  for (int ll = (Severn_Seg.length() - 1); ll > -1; ll--) {

    // test of ascii value is a decimal point
    if (Severn_Seg.charAt(ll) == 0x2E) {
      DP_Flag = 1;  // set flag
    } // end if

    // the character is not a dp
    else {

      // start with a blank digit
      int Value = 0x1B;

      // test if numbers are 0-9
      if (Severn_Seg.charAt(ll) > 0x2F && Severn_Seg.charAt(ll) < 0x3A) {
        Value = (Severn_Seg.charAt(ll) - 0x30);
      }

      // test if numbers are A-F
      if (Severn_Seg.charAt(ll) > 0x40 && Severn_Seg.charAt(ll) < 0x47) {
        Value = (Severn_Seg.charAt(ll) - 0x37);
      }

      // special characters. -, H, L, n, P, t, Y, U and c (centigrade) and o to a degree character
      // minus sign
      if (Severn_Seg.charAt(ll) == 0x2D) {
        Value = 0x10;
      }

      // c for centigrade
      if (Severn_Seg.charAt(ll) == 0x63) {
        Value = 0x11;
      }

      // o for degree
      if (Severn_Seg.charAt(ll) == 0x6F) {
        Value = 0x12;
      }

      // Space
      if (Severn_Seg.charAt(ll) == 0x20) {
        Value = 0x1B;
      }

      // H
      if (Severn_Seg.charAt(ll) == 0x48 || Severn_Seg.charAt(ll) == 0x68) {
        Value = 0x13;
      }

      // L
      if (Severn_Seg.charAt(ll) == 0x4C || Severn_Seg.charAt(ll) == 0x6C) {
        Value = 0x14;
      }

      // n
      if (Severn_Seg.charAt(ll) == 0x4E || Severn_Seg.charAt(ll) == 0x6E) {
        Value = 0x15;
      }

      // P
      if (Severn_Seg.charAt(ll) == 0x50 || Severn_Seg.charAt(ll) == 0x70) {
        Value = 0x16;
      }

      // t
      if (Severn_Seg.charAt(ll) == 0x54 || Severn_Seg.charAt(ll) == 0x74) {
        Value = 0x17;
      }

      // Y
      if (Severn_Seg.charAt(ll) == 0x59 || Severn_Seg.charAt(ll) == 0x79) {
        Value = 0x18;
      }

      // U
      if (Severn_Seg.charAt(ll) == 0x55 || Severn_Seg.charAt(ll) == 0x75) {
        Value = 0x19;
      }

      // r
      if (Severn_Seg.charAt(ll) == 0x72 || Severn_Seg.charAt(ll) == 0x75) {
        Value = 0x1A;
      }

      // range check, only incrument the pointer if not under range
      if (Buffer_Pointer > -1) {

        if (DP_Flag == 0) {

          // convert to severn segment
          Value = pgm_read_byte(&Char_Set[Value]);
          _Frame_Buffer[Buffer_Pointer] = Value;

        }
        else {
          Value = pgm_read_byte(&Char_Set[Value]);
          _Frame_Buffer[Buffer_Pointer] = Value + 0x80;
          DP_Flag = 0;  // clear flag
        }

        // buffer at location Buffer_Pointer now has the location within the lookup
        // table for the character, if bit 7 set then a d.p. should be added
        Buffer_Pointer--;
      } // end range check

    } // end else

  } // end Severn_Seg read

  // update the 7-segment and LED display
  Display_Update(_Brightness);

}  // end function
