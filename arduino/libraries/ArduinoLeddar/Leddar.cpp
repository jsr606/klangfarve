// *****************************************************************************
//
/// \file    Leddar.cpp
///
/// \brief   Arduino library for communicating with a Leddar(TM)
///          via Modbus RTU protocol over UART/RS485.	
///
//  This file is part of the Leddar Library.
//  
//  
// The LeddarTM Arduino Library is licensed under the terms of the MIT License,
// as is without any warranty. See License.txt for more details.
//
// Last edited: 25 / Aug / 2015
// Copyright© LeddarTech, Inc. 2015
// *****************************************************************************

#include "Leddar.h"
#include "Arduino.h"

#define ARRAYSIZE(v) (sizeof(v)/sizeof(v[0]))        // helper macro to get the number of elements of an array

// Table of CRC values for high–order byte
static byte CRC_HI[] =
{
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
    0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
    0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81,
    0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
    0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
    0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
    0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
    0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
    0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
    0x40
};

// Table of CRC values for low–order byte
static byte CRC_LO[] =
{
    0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7, 0x05, 0xC5, 0xC4,
    0x04, 0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
    0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD,
    0x1D, 0x1C, 0xDC, 0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
    0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32, 0x36, 0xF6, 0xF7,
    0x37, 0xF5, 0x35, 0x34, 0xF4, 0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
    0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE,
    0x2E, 0x2F, 0xEF, 0x2D, 0xED, 0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
    0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 0x61, 0xA1, 0x63, 0xA3, 0xA2,
    0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
    0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68, 0x78, 0xB8, 0xB9, 0x79, 0xBB,
    0x7B, 0x7A, 0xBA, 0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
    0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0, 0x50, 0x90, 0x91,
    0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
    0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98, 0x88,
    0x48, 0x49, 0x89, 0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
    0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83, 0x41, 0x81, 0x80,
    0x40
};


// *****************************************************************************
// Function: CRC16
//
/// \brief   Compute a CRC16 using the Modbus recipe.
///
/// \param   aBuffer  Array containing the data to use.
/// \param   aLength  Number of byte in aBuffer.
/// \param   aCheck   If true, the two bytes after aLength in aBuffer are
///                   supposed to contain the CRC and we verify that it is
///                   the same as what was just computed.
///
/// \return  If aCheck is false, always returns true, if aCheck is true,
///          return true if the CRC compares ok, false otherwise.
// *****************************************************************************
bool CRC16(byte *aBuffer, byte aLength, bool aCheck) 
{
	byte lCRCHi = 0xFF; // high byte of CRC initialized
	byte lCRCLo = 0xFF; // low byte of CRC initialized
	int i;
	
	for (i = 0; i<aLength; ++i) 
	{
		int lIndex = lCRCLo ^ aBuffer[i]; // calculate the CRC
		lCRCLo = lCRCHi ^ CRC_HI[lIndex];
		lCRCHi = CRC_LO[lIndex];
	}
	
	if (aCheck) 
	{
		return ( aBuffer[aLength] == lCRCLo ) && ( aBuffer[aLength+1] == lCRCHi );
	}
	else 
	{
		aBuffer[aLength] = lCRCLo;
		aBuffer[aLength+1] = lCRCHi;
		return true;
	}
}

// #####################################################################
// #####################################################################
//
// 16-channel Sensors (M16, IS16, Evaluation Kit)
//
// #####################################################################
// #####################################################################

// *********************************************************************
// Function: init
//
/// \brief   Initializes the serial port
//
// **********************************************************************
void Leddar16::init() 
{
	if (TxEn_Pin)
	pinMode(TxEn_Pin, OUTPUT);
	
	SerialPort.begin(BaudRate);
	while (!Serial) 
	{
		// wait for serial port to connect. Needed for Leonardo only
	}
}
	
// *********************************************************************
// Function: getDetections
//
/// \brief   Retrieves detections from the Leddar and stores them 
///			 in the Detections[] array
// **********************************************************************
char Leddar16::getDetections() 
{
	unsigned long startTime;

	clearDetections();
	sendRequest();				//Sends the request for detections on serial port
	startTime = millis();
	
	while (!SerialPort.available())
	{
		// wait up to 1000ms
		if (millis()-startTime > 1000)
		{
			return ERR_LEDDAR_NO_RESPONSE;
		}
	}
	
	return parseResponse();     // Parses the data available on the serial port
}	
	
// *********************************************************************
// Function: sendRequest
//
/// \brief   sends a "Get Detections" Request to the Leddar M16
//
// **********************************************************************

char Leddar16::sendRequest() 
{
	unsigned char sendData[4] = {0};
	unsigned int i = 0;	
	
	//clear serial buffer
	while (SerialPort.available())
	{
		SerialPort.read();
		
		if (++i > 250) return ERR_LEDDAR_SERIAL_PORT;
	}
	
	// Enable TX if necessary
	if (TxEn_Pin)
	{
		digitalWrite(TxEn_Pin, TxEn_Action); 
	}
	
	//send message on uart
	sendData[0] = SlaveAddress;
	sendData[1] = 0x41;
	CRC16(sendData, 2, false);
	for (i = 0; i<4; i++)
	{
		SerialPort.write(sendData[i]);
	}
	SerialPort.flush();
	
	// Disable TX if necessary
	if (TxEn_Pin)
	{
		digitalWrite(TxEn_Pin, 1-TxEn_Action); 
	}
	
	return 0;
}

// *********************************************************************
// Function: parseResponse
//
/// \brief   Parses the response from the Leddar M16 and stores the
///          detections in the Detections[] Array
// **********************************************************************
char Leddar16::parseResponse()
{
	unsigned int crc = 0xFFFF;
	unsigned char receivedData[256] = {0};
	unsigned int i = 0;
	unsigned int len = 0;
	unsigned char msgFound = 0;
	unsigned long startTime = millis();
	
	NbDet = 0;
	clearDetections();

	startTime = millis();
	// Wait maximum 5ms for each byte
	while (millis()-startTime < 10) 
	{
		if (SerialPort.available())
		{
			receivedData[len++] = SerialPort.read();
			startTime = millis();
		}
	}
	
	// Check length of message. should be at least 10 bytes 
	// even when no detections are found
	if (len < 10)
	{
		return ERR_LEDDAR_SHORT_RESPONSE;
	}
	
	if (len < 256 && receivedData[1] == 0x41 && receivedData[0] == SlaveAddress)
	{
		// Check CRC
		if (!CRC16(receivedData, len-2, true))
		{
			return ERR_LEDDAR_BAD_CRC; //invalid CRC
		}
		
		i = 3;
		for (NbDet = 0; (NbDet < ARRAYSIZE(Detections)) && (i < len-11); NbDet++)
		{
			// For each detection:
			// Bytes 0 and 1 = distance in cm
			// Bytes 2-3 are amplitude*64
			// Byte 4 is Segment number*16
			Detections[NbDet].Segment = receivedData[i+4]/16;
			if (Detections[NbDet].Segment > 15) return -2;   // Invalid Reading
			
			Detections[NbDet].Distance = ((unsigned int)receivedData[i+1])*256 + receivedData[i];
			Detections[NbDet].Amplitude = ((float)receivedData[i+3])*4+ ((float)receivedData[i+2])/64;
			Detections[NbDet].Segment = receivedData[i+4]/16;    
			
			i += 5;
		}
		
		if (NbDet != receivedData[2])
		{
			return ERR_LEDDAR_NB_DETECTIONS;
		}
		
		// Finally get the timestamp, the LED power and status
		TimeStamp = ((unsigned long)receivedData[len-5])<<24 ;
		TimeStamp += ((unsigned long)receivedData[len-6])<<16;
		TimeStamp += ((unsigned long)receivedData[len-7])<<8;
		TimeStamp += receivedData[len-8];					
		
		LEDPower = receivedData[len-4];
		Status = receivedData[len-3];
		
		return NbDet;
	}
	else 
		return ERR_LEDDAR_BAD_RESPONSE; // Invalid response
} 



// *********************************************************************
// Function: clearDetections
//
/// \brief   Clears the Detections[] array.
// **********************************************************************
void Leddar16::clearDetections() 
{
	memset(Detections, 0, sizeof Detections);
}


// #####################################################################
// #####################################################################
//
// LeddarOne
//
// #####################################################################
// #####################################################################

// *********************************************************************
// Function: init
//
/// \brief   Initializes the serial port
//
// **********************************************************************
void LeddarOne::init() 
{
	if (TxEn_Pin)
    pinMode(TxEn_Pin, OUTPUT);
	
	SerialPort.begin(BaudRate);
	while (!Serial) 
	{
		// wait for serial port to connect. Needed for Leonardo only
	}
}


// *********************************************************************
// Function: getDetections
//
/// \brief   Retrieves detections from the LeddarOne and stores them 
///			 in the Detections[] array
// **********************************************************************
char LeddarOne::getDetections() 
{
	unsigned int crc = 0xFFFF;
	unsigned char dataBuffer[19] = {0};
	unsigned int i = 0;
	unsigned int len = 0;
	unsigned char msgFound = 0;
	unsigned long startTime = millis();
	unsigned char detcount = 0;

	clearDetections();
	sendRequest();
	startTime = millis();
	

	while (!SerialPort.available())
	{
		//wait up to 1000ms
		if (millis()-startTime > 1000)
		{
			return ERR_LEDDAR_NO_RESPONSE;
		}
	}
	
	return parseResponse();

}
	
char LeddarOne::sendRequest()
{
	unsigned char dataBuffer[19] = {0};
	unsigned int i = 0;
	unsigned long startTime = millis();
	


	// clear serial buffer
	while (SerialPort.available())
	{
		SerialPort.read();
		
		if (++i > 250) return ERR_LEDDAR_SERIAL_PORT;
	}

	// Enable TX if necessary
	if (TxEn_Pin)
	{
		digitalWrite(TxEn_Pin, TxEn_Action); 
	}

	// Send message on UART:
	dataBuffer[0] = SlaveAddress;
	dataBuffer[1] = 0x04;
	dataBuffer[2] = 0;
	dataBuffer[3] = 20;
	dataBuffer[4] = 0;
	dataBuffer[5] = 10;
	CRC16(dataBuffer, 6, false);
	
	for (i = 0; i<8; i++)
	{
		SerialPort.write(dataBuffer[i]);
	}
	SerialPort.flush();

	// Disable TX if necessary
	if (TxEn_Pin)
	{
		digitalWrite(TxEn_Pin, 1-TxEn_Action); 
	}
}

char LeddarOne::parseResponse()
{
	unsigned int crc = 0xFFFF;
	unsigned char dataBuffer[25] = {0};
	unsigned int i = 0;
	unsigned int len = 0;
	unsigned long startTime;
	unsigned char detcount = 0;
	startTime = millis();
	
	// Wait maximum 5ms for each byte
	while (millis()-startTime < 10) 
	{
		if (SerialPort.available())
		{
			if (len >= 25) return ERR_LEDDAR_BAD_RESPONSE; 
			dataBuffer[len++] = SerialPort.read();
			startTime = millis();
		}
		if (len == 25 && !SerialPort.available())
		{
			break;
		}
	}

	if (len == 25 && dataBuffer[1] == 0x04 && dataBuffer[0] == SlaveAddress)
	{
		// Check CRC
		if (!CRC16(dataBuffer, len-2, true))
		{
			return ERR_LEDDAR_BAD_CRC; //invalid CRC
		}
		
		NbDet = dataBuffer[10];
		
		if (NbDet > ARRAYSIZE(Detections))
		{
			return ERR_LEDDAR_NB_DETECTIONS;
		}
		
		//Timestamp
	    TimeStamp = ((unsigned long)dataBuffer[5]) << 24;
		TimeStamp += ((unsigned long)dataBuffer[6]) << 16 ;
		TimeStamp += ((unsigned long)dataBuffer[3])<<8; 
		TimeStamp += dataBuffer[4];

		
		// Internal Temperature
		Temperature = dataBuffer[7];
		Temperature += ((float)dataBuffer[8])/256;
		
		i = 11;
		for (detcount = 0; detcount < NbDet; detcount++)
		{
			// For each detection:
			// Bytes 0 and 1 = distance in cm
			// Bytes 2-3 are amplitude*256
			Detections[detcount].Distance = ((unsigned int)dataBuffer[i])*256 + dataBuffer[i+1];
			Detections[detcount].Amplitude = ((float)dataBuffer[i+2])+ ((float)dataBuffer[i+3])/256;
			
			i += 4;
		}
	
		return NbDet;
	}
	else 
		return ERR_LEDDAR_BAD_RESPONSE; // Invalid response
}	
	
	
// *********************************************************************
// Function: clearDetections
//
/// \brief   Clears the Detections[] array.
// **********************************************************************
void LeddarOne::clearDetections() 
{
	memset(Detections, 0, sizeof Detections);
}
		
