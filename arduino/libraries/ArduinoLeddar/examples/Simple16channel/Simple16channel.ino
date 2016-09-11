/*
 Simple Leddar(TM) Example
 Language: Arduino
 
 This program displays on a LCD screen the distance
 and returned signal amplitude of the first detection
 in channel 8 of a Leddar(TM)
 
 
   Shields used:
 * RS-485 Shield
 * LCD Keypad Shield
 
 Created 21 Jan. 2014
 by Pier-Olivier Hamel
 
 This example code is in the public domain.
*/
#include <Leddar.h>
#include <LiquidCrystal.h>


Leddar16 Leddar1(115200,1);
//Baudrate = 115200
//Modbus slave ID = 01
// NOTE: If your RS-485 shield has a Tx Enable (or DE) pin, 
// use: Leddar16 Leddar1(115200,1, TxEnablePinNumber, 1);

LiquidCrystal lcd(8, 9, 4, 5, 6, 7); 


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

	char result = Leddar1.getDetections();
	if (result >= 0)
	{
		for (int i = 0; i < Leddar1.NbDet; i++)
		{
			if (Leddar1.Detections[i].Segment == 8)
			{
				Distance = Leddar1.Detections[i].Distance;
				Amplitude = Leddar1.Detections[i].Amplitude;
				break;
			}
		}  

		lcd.setCursor(0,0);
		lcd.print("Distance:        ");
		lcd.setCursor(10,0);
		lcd.print(Distance);
		lcd.setCursor(0,1);
		lcd.print("Amplitude:        ");
		lcd.setCursor(11,1);
		lcd.print(Amplitude);    
	}
	else
	{
		lcd.setCursor(0,0);
		lcd.print("Error: "); lcd.print((int)result); lcd.print("        ");
		lcd.setCursor(0,1);
		lcd.print("No Leddar Found");
	}

	delay(50);
}
