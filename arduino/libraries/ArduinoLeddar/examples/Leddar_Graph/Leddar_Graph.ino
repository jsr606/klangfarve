/*
 Simple Leddar(TM) Example
 Language: Arduino
 
 This program displays on a LCD screen a chart of the
 distances on each segment. Each column is a segment
 and the height of the line in each column represents 
 the distance of the object detected.
 
 
   Shields used:
 * RS-485 Shield
 * LCD Shield
 
 
 Created 22 Jan. 2014
 by Pier-Olivier Hamel

 
 This example code is in the public domain.

 */
#include <Leddar.h>
#include <LiquidCrystal.h>
#include <math.h>


#define SCALE 50    // Number of cm per pixel
#define OFFSET 0    // Offset in cm for the closest pixel

Leddar16 Leddar1(115200,1);
//Baudrate = 115200
//Modbus slave ID = 01
// NOTE: If your RS-485 shield has a Tx Enable (or DE) pin, 
// use: Leddar Leddar1(115200,1, TxEnablePinNumber, 1);

LiquidCrystal lcd(8, 9, 4, 5, 6, 7); 

unsigned char ErrorCount = 0;
byte CustomChar[15] =
{
  B00000, B00000, B00000, B00000,B00000, B00000,B00000, B11111, B00000, B00000, B00000, B00000,B00000, B00000, B00000
};

void setup()
{
  
  // Initialize LCD
  lcd.begin(16, 2);
  //Display startup screen
  lcd.print("Leddar(TM) Graph");
  lcd.setCursor(0,1);
  lcd.print("Starting...");
  
  //Program custom characters in the LCD
  for (int i = 0; i < 8; i++)
  {
    lcd.createChar(i, CustomChar+i);
  }
  
  //Initialize Leddar 
  Leddar1.init();
  
  delay(1000);
  
}



void loop()
{
  unsigned int seg = 0;
  unsigned int Learned[16];
  float Distance =0;
  char PrintChar = 0;

  
  //Initialize the array to 0xFF
  memset(Learned, 0xFF,32);
  
  if (Leddar1.getDetections() >= 0)
  {
    ErrorCount = 0;
    //Clear the LCD
    lcd.clear();
    
    
    // Take the closest measurement for each segment and save it.
    for (int i = 0; i < Leddar1.NbDet; i++)
    {
      seg = Leddar1.Detections[i].Segment;
      if (Leddar1.Detections[i].Distance < Learned[seg])
      {
         Learned[seg] = Leddar1.Detections[i].Distance;
      }
    }
    
    //Then Print each detection on the LCD
    for (int i =0; i< 16; i++)
    {
      //Transform distances from arc of circle to "horizontal distance"
      Distance = Learned[i]*sin(1.17809+((float)i)*0.05236);
      
      if (Distance > OFFSET) Distance-= OFFSET;
        Distance/= SCALE;
      
      
      if (Distance >= 8)
      {
        if(Distance > 15) 
        {
          Distance = 15;
        }
        lcd.setCursor(i,0);
        Distance-= 8;
      }
      else
      {
        lcd.setCursor(i,1);
      }
      
      PrintChar = (char) floor(Distance);
      lcd.print(PrintChar);
    }
      
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

  //pause 50ms
  delay(50);
  
}
