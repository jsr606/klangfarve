// *****************************************************************************
//
/// \file    Leddar.h
///
/// \brief   Arduino library for communicating with a Leddar(TM)  via Modbus RTU
///          protocol over UART/RS485.  Supports 16-channel and single 
///          channel sensors.
///
//  This file is part of the Leddar Library.
//  
//  
// The LeddarTM Arduino Library is licensed under the terms of the MIT License,
// as is without any warranty. See License.txt for more details.
//
// Copyright© LeddarTech, Inc. 2015
// *****************************************************************************

#ifndef Leddar_h
#define Leddar_h

/* _____STANDARD INCLUDES____________________________________________________ */
#include "Arduino.h"

/* _____PROJECT INCLUDES_____________________________________________________ */


/* _____LIBRARY DEFINITIONS__________________________________________________ */


#define DEFAULT_BAUD_RATE 115200

// Default address for the slave
#define DEFAULT_ADDRESS 0x01

//Error codes
#define ERR_LEDDAR_BAD_CRC -1
#define ERR_LEDDAR_NO_RESPONSE -2
#define ERR_LEDDAR_BAD_RESPONSE -3
#define ERR_LEDDAR_SHORT_RESPONSE -4
#define ERR_LEDDAR_SERIAL_PORT -5

// Number of detections does not match 
#define ERR_LEDDAR_NB_DETECTIONS -6;


/* _____CLASS DEFINITIONS____________________________________________________ */

/// Represents a measurement:
struct Detection
{
	unsigned char Segment;
	unsigned int Distance;
	unsigned int Amplitude;
	
	// Default constructor
	Detection() : Segment(0), Distance(0xFFFF), Amplitude(0) { }
};

/// Class to connect to a 16-channel Leddar module (Evaluation Kit, M16, IS16):
class Leddar16
{
public:
	Leddar16() 
	: BaudRate(DEFAULT_BAUD_RATE), SlaveAddress(DEFAULT_ADDRESS), NbDet(0), TxEn_Pin(0), TxEn_Action(0), SerialPort(Serial) 
	{ }
	
	Leddar16( unsigned long Baud, unsigned char Addr = DEFAULT_ADDRESS, HardwareSerial& Port = Serial, unsigned char Pin = 0, unsigned char Action = 0) 
	: BaudRate(Baud), SlaveAddress(Addr), NbDet(0), TxEn_Pin(Pin), TxEn_Action(Action)  , SerialPort(Port)
	{ }
	
	Leddar16( unsigned long Baud, unsigned char Addr, unsigned char Pin , unsigned char Action ) 
	: BaudRate(Baud), SlaveAddress(Addr), NbDet(0), TxEn_Pin(Pin), TxEn_Action(Action)  , SerialPort(Serial)
	{ }
	
	void init();
	char getDetections();
	char sendRequest();
	char parseResponse();
	void clearDetections();

public:
	unsigned char NbDet;
	Detection Detections[50];
	unsigned long TimeStamp;
	unsigned char LEDPower;
	unsigned char Status;   

private:
	unsigned long BaudRate;
	unsigned char SlaveAddress;
	unsigned char TxEn_Pin;
	unsigned char TxEn_Action;
	HardwareSerial& SerialPort;
};

// Class to connect to a LeddarOne sensor:
class LeddarOne
{
public:
	LeddarOne() 
	: BaudRate(DEFAULT_BAUD_RATE), SlaveAddress(DEFAULT_ADDRESS), NbDet(0), TxEn_Pin(0), TxEn_Action(0), SerialPort(Serial) 
	{ }
	
	LeddarOne( unsigned long Baud, unsigned char Addr = DEFAULT_ADDRESS, HardwareSerial& Port = Serial, unsigned char Pin = 0, unsigned char Action = 0) 
	: BaudRate(Baud), SlaveAddress(Addr), NbDet(0), TxEn_Pin(Pin), TxEn_Action(Action)  , SerialPort(Port)
	{ }
	
	LeddarOne( unsigned long Baud, unsigned char Addr, unsigned char Pin, unsigned char Action) 
	: BaudRate(Baud), SlaveAddress(Addr), NbDet(0), TxEn_Pin(Pin), TxEn_Action(Action)  , SerialPort(Serial)
	{ }	
	
	void init();
	char getDetections();
	char sendRequest();
	char parseResponse();
	void clearDetections();

public:
	unsigned char NbDet;
	Detection Detections[3];
	unsigned long TimeStamp;
	float Temperature;

private:
	unsigned long BaudRate;
	unsigned char SlaveAddress;
	unsigned char TxEn_Pin;
	unsigned char TxEn_Action;
	HardwareSerial& SerialPort;
};

#endif