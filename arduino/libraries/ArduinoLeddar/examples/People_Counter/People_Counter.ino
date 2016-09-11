/*
 Simple People Counter Using Leddar(TM) Technology
 Language: Arduino
 
 This program learns the environment on startup and
 will count how many times a person or object passes 
 within its field of view.
 
 
   Shields used:
 * RS-485 Shield
 * LCD Keypad Shield
 
 Created 15 Jan. 2014
 by Pier-Olivier Hamel

 
 This example code is in the public domain.

 */
#include <Leddar.h>
#include <LiquidCrystal.h>


#define HIGH_THRESHOLD 30
#define LOW_THRESHOLD 15

Leddar16 Leddar1(115200,1);
//Baudrate = 115200
//Modbus slave ID = 01
// NOTE: If your RS-485 shield has a Tx Enable (or DE) pin, 
// use: Leddar Leddar1(115200,1, TxEnablePinNumber, 1);


LiquidCrystal lcd(8, 9, 4, 5, 6, 7); 
Detection TmpDet;
unsigned int Cnt = 0;
unsigned int Learned[16];
bool CanCount = true;
  

void setup()
{
  
  // Initialize LCD
  lcd.begin(16, 2);
  lcd.print("Leddar (TM)");
  lcd.setCursor(0,1);
  lcd.print("Starting...");
  
  //Initialize Leddar and display boot screen for a couple of seconds
  Leddar1.init();
  delay(2000);
  
  //Make a first attempt to get Data from the Leddar
  while (Leddar1.getDetections() < 0)
  {
    delay(100);
    if (millis() > 10000)
    {
      lcd.setCursor(0,1);
      lcd.print("Device Not Found");      
    }
  }
 
  lcd.setCursor(0,1);
  lcd.print("Count:        ");
 
  // Learn the environment
  Learn();

}



void loop()
{
  
  if (Leddar1.getDetections() > 0)
  {
    Count();
    lcd.setCursor(7,1);
    lcd.print("         ");
    lcd.setCursor(7,1);
    lcd.print(Cnt);
  }
  else
  {
    lcd.setCursor(14,1);
    lcd.print("E");
  }

  //pause 50ms
  delay(50);
  
}

void Learn()
{
  unsigned char seg = 0;
  memset(Learned, 0xFF,32);
  
  // Take the closest measurement for each segment and save it.
  for (int i = 0; i < Leddar1.NbDet; i++)
  {
    seg = Leddar1.Detections[i].Segment;
    if (Leddar1.Detections[i].Distance < HIGH_THRESHOLD)
    {
      Learned[seg] = HIGH_THRESHOLD;
    }
    else if (Leddar1.Detections[i].Distance < Learned[seg])
    {
       Learned[seg] = Leddar1.Detections[i].Distance;
    }
  }
  
  Cnt = 0;
  CanCount = true;  
}


void Count()
{
  unsigned int Distances[16];
  unsigned char seg = 0;
  
  // Initialize Distances table
   memset(Distances, 0xFF,32);
  
  //Take the closest measurement for each segment
  for (int i = 0; i < Leddar1.NbDet; i++)
  {
    seg = Leddar1.Detections[i].Segment;
    if (Leddar1.Detections[i].Distance < Distances[seg])
    {
      Distances[seg] = Leddar1.Detections[i].Distance;
      if (Distances[seg] < HIGH_THRESHOLD)
        Distances[seg] = HIGH_THRESHOLD;
    }
  }    
  
  //Then compâre with what we previously learned
  if (CanCount)
  {
    // If we are ready to count, check for an object. If any one segment
    // is closer than what we learned minus an hysteresis thershold, increment
    // the people counter
    for (int i = 0; i < 16; i++)
    {
      if (Distances[i] < Learned[i]-HIGH_THRESHOLD)
      {
        Cnt++;
        CanCount = false;
        break;
      }
    }   
  }
  //Otherwise, check if the current object is still in the door
  else
  {
    CanCount = true;
    for (int i = 0; i < 16; i++)
    {
      // if any one segment shows an object closer than what we learned minus an
      // hysteresis threshold, we still have something in the door and are not ready to
      // continue counting
      if (Distances[i] < Learned[i]-LOW_THRESHOLD)
      {
        CanCount = false;
        break;
      }
    }    
  }
  
  
}


