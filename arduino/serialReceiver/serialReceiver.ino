/*
  Software serial multple serial test
 
 Receives from the hardware serial, sends to software serial.
 Receives from software serial, sends to hardware serial.
 
 The circuit: 
 * RX is digital pin 10 (connect to TX of other device)
 * TX is digital pin 11 (connect to RX of other device)
 
 Note:
 Not all pins on the Mega and Mega 2560 support change interrupts, 
 so only the following can be used for RX: 
 10, 11, 12, 13, 50, 51, 52, 53, 62, 63, 64, 65, 66, 67, 68, 69
 
 Not all pins on the Leonardo support change interrupts, 
 so only the following can be used for RX: 
 8, 9, 10, 11, 14 (MISO), 15 (SCK), 16 (MOSI).
 
 created back in the mists of time
 modified 25 May 2012
 by Tom Igoe
 based on Mikal Hart's example
 
 This example code is in the public domain.
 
 */
#include <SoftwareSerial.h>

SoftwareSerial mySerial(10, 11); // RX, TX

int led = 13;

void setup()  
{
  Serial.begin(115200);
  // set the data rate for the SoftwareSerial port
  mySerial.begin(57600);
  mySerial.println("Hello, world?");
  pinMode(led, OUTPUT);
}

void loop() // run over and over
{

/*
  int a = random(100);

  digitalWrite(led, HIGH);
  mySerial.write(a);
  digitalWrite(led, LOW);
  delay(200);
*/
  /*
  if (mySerial.available())
    Serial.println(int(mySerial.read()));
  */
  if (mySerial.available())
    Serial.write(mySerial.read());
  
  /*
  if (Serial.available())
    mySerial.write(Serial.read());
    */
  
}

