/*
 Simple Leddar(TM) Example, using a LeddarOne.
 Language: Arduino
 
 This program displays on a LCD screen the distance
 and returned signal amplitude of the detection
 
   Shields used:
 * RS-485 Shield
 * LCD Keypad Shield
 
 This example code is in the public domain.
*/
#include <SoftwareSerial.h>
#include <Leddar.h>
#include <LiquidCrystal.h>


LeddarOne Leddar1(115200,1);
//Baudrate = 115200
//Modbus slave ID = 01
// NOTE: If your RS-485 shield has a Tx Enable (or DE) pin, 
// use: Leddar Leddar1(115200,1, TxEnablePinNumber, 1);

LiquidCrystal lcd(8, 9, 4, 5, 6, 7); 

void setup()
{
	Serial.begin(115200); //Opens serial connection at 9600bps.

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
		// Show the first detection only
		Distance = Leddar1.Detections[0].Distance;
		Amplitude = Leddar1.Detections[0].Amplitude;

		//Serial.print("Distance: "); Serial.println(Distance); 
		//Serial.print("Amplitude: "); Serial.println(Amplitude); 
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
		//Serial.print("Error: "); Serial.println((int)result); 
		lcd.setCursor(0,0);
		lcd.print("Error: "); lcd.print((int)result); lcd.print("        ");
		lcd.setCursor(0,1);
		lcd.print("No LeddarOne Found");
	}
	delay(50);
}
