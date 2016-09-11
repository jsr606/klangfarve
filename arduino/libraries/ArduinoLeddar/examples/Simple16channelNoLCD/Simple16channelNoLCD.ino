/*
 Simple Leddar(TM) Example - Without LCD
 Language: Arduino
 
 This program lists the detections read on the serial port of the Arduino.
 Can be used with Arduino IDE's Serial Monitor.
 
   Shields used:
 * RS-485 Shield
 
 Created 01 Jun. 2015
 by Pier-Olivier Hamel
 
 This example code is in the public domain.
*/
#include <Leddar.h>


Leddar16 Leddar1(115200,1);
//Baudrate = 115200
//Modbus slave ID = 01

void setup()
{
	//Initialize Leddar 
	Leddar1.init();
}



void loop()
{
        
	char result = Leddar1.getDetections();
	if (result >= 0)
	{
		for (int i = 0; i < Leddar1.NbDet; i++)
		{
                    Serial.print("Segment: ");
                    Serial.print(Leddar1.Detections[i].Segment);
                    Serial.print("      Distance: ");
		    Serial.print(Leddar1.Detections[i].Distance);
                    Serial.print("\n");
		}  

	}
	else
	{
		Serial.print("Error: "); 
                Serial.print((int)result);
                Serial.print("\n");
	}

	delay(50);
}
