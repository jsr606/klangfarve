/*
 Simple Leddar(TM) Example
 Language: Arduino
 
 This program displays on a LCD screen the distance
 and returned signal amplitude of the first detection
 in a segment of a Leddar(TM)
 
 Segments are selected by pressing up and down on the
 LCD_Keypad shield.
 
 
   Shields used:
 * RS-485 Shield
 * DFRobot LCD Keypad Shield
 
 **REQUIRES LCD_KEY LIBRARY**
 
 Created 21 Jan. 2014
 by Pier-Olivier Hamel

 
 This example code is in the public domain.

 */
#include <Leddar.h>
#include <LiquidCrystal.h>
#include <LCD_Key.h>


Leddar16 Leddar1(115200,1);
//Baudrate = 115200
//Modbus slave ID = 01
// NOTE: If your RS-485 shield has a Tx Enable (or DE) pin, 
// use: Leddar Leddar1(115200,1, TxEnablePinNumber, 1);

LiquidCrystal lcd(8, 9, 4, 5, 6, 7); 
LCD_Key keypad;
int PrevKey = 0;
unsigned char ErrorCount = 0;  


void setup()
{
  
  // Initialize LCD
  lcd.begin(16, 2);
  
  //Initialize Leddar 
  Leddar1.init();
  
}



void loop()
{
  unsigned int Distance = 0;
  unsigned int Amplitude = 0;
  static char CurSegment = 0;
  int CurKey = keypad.getKey();
  
  if (PrevKey != CurKey)
  {
    PrevKey = CurKey;
    if (CurKey == 3)  // up
    {
      if (++CurSegment > 15)
            CurSegment = 0;
    }
    else if (CurKey == 4) // down
    {
       if (--CurSegment < 0)
            CurSegment = 15;     
    }
  }
  
  if (Leddar1.getDetections() >= 0)
  {
    
    for (int i = 0; i < Leddar1.NbDet; i++)
    {
      if (Leddar1.Detections[i].Segment == CurSegment)
      {
        Distance = Leddar1.Detections[i].Distance;
        Amplitude = Leddar1.Detections[i].Amplitude;
        break;
      }
    }  
    
    lcd.setCursor(0,0);
    lcd.print("Segment: ");
    lcd.print(CurSegment,10);
    lcd.print("  ");
    
    lcd.setCursor(0,1);
    lcd.print("Distance:        ");
    lcd.setCursor(10,1);
    lcd.print(Distance); 
  }
  else
  {
    if (ErrorCount < 10)
    {
      ErrorCount++;
    }
    else{      
      lcd.setCursor(0,0);
      lcd.print("Error:          ");
      lcd.setCursor(0,1);
      lcd.print("No Leddar Found ");
    }
  }
  
}
