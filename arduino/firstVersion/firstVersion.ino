// viera collaro
// RGBW input box
// leddar one sensor
// DMX color control

// with color picker mode

#include <SoftwareSerial.h>
#include <Leddar.h>
#include <avr/pgmspace.h>
#include <DmxSimple.h>

const byte numberOfColors = 8;
const byte data[numberOfColors * 4] PROGMEM = {
  196, 228,   0,  64,     // 0 gul
  255,   0,   0,  23,     // 1 rød
  255,   0, 105,  10,     // 2 magenta
  255,   0, 255,  37,     // 3 lilla
  4  ,  42, 255,   0,     // 4 blå
  0  , 255,  14,   0,     // 5 grøn
  246, 146,   0,   0,     // 6 orange
  215, 161, 106, 255      // 7 hvid
};

byte currentColor [2] = {0, 0};
byte nextColor [2] = {1, 1};
unsigned long fadeProgression [2] = {0, 0};
boolean fadeCompleted [2] = {false, false};
float fadeSpeed [2] = {0, 0};
float fadeSpeedMultiplier [2] = {1, 1};
byte r [2] = {0, 0};
byte g [2] = {0, 0};
byte b [2] = {0, 0};
byte w [2] = {0, 0};

float generalBrightness = 1;

// DMX shield
const int jrde =  2;
const int jdi  =  3;
const int jro  =  4;

// since RS485 shield uses TX and RX, we use soft serial for feedback via second arduino board
SoftwareSerial mySerial(10, 11); // RX, TX
LeddarOne Leddar1(115200, 1);

const int led = 13;
const int push = A4;
const int slider = 5;
const int lightSensor = A5;
const int pot [] = {A0, A1, A2, A3};

boolean buttonState = false;
boolean sliderState = false;

unsigned int maxDistance = 0;
unsigned int Distance = 0;
float normalizedDistance = 0;

int mode = 1;
int nextMode = -1;
int changingMode = -1;
const int NO = -1, INIT = 0, CHANGING = 1, FINISHED = 2;

unsigned long lastFeedback = millis(), lastTick = millis();
const int feedbackFrequency = 200;
boolean feedback = false;
unsigned int seconds = 0, minutes = 0, hours = 0, days = 0;

boolean peoplePresent = false;
unsigned long lastPerson = millis();
unsigned long lastFlip = millis();
boolean flipped = false;

const long fadeCycleLength = 10000;

void setup()
{
  // init DMX
  pinMode(jrde, OUTPUT);
  pinMode(jdi,  OUTPUT);
  pinMode(jro,  INPUT);

  digitalWrite(jrde, HIGH);
  DmxSimple.usePin(jdi);
  DmxSimple.maxChannel(9);

  Serial.begin(115200); // serial for RS485 control of leddar one sensor
  mySerial.begin(57600); // software serial for feedback

  Leddar1.init(); //Initialize Leddar

  pinMode (led, OUTPUT);
  pinMode (push, INPUT_PULLUP);
  pinMode (slider, INPUT_PULLUP);

  // hold down key on boot for color picker mode
  //if (digitalRead(push) == LOW) mode = -1;

  mySerial.print("number of colors: "); mySerial.println(numberOfColors);
}

void loop()
{

  tick();
  buttonState = digitalRead(push);
  sliderState = digitalRead(slider);
  getLeddar();

  generalBrightness = analogRead(pot[3]) / 4; // max = 255

  if (buttonState == LOW && sliderState == HIGH) {
    mySerial.print("im in state: ");
    mySerial.print(mode);
    mySerial.print("\t my changeState is ");
    mySerial.println(changingMode);
  }

  switch (mode) {
    case -1:
      colorPicker();
      break;
    case 0:
      {
        // I: en farve
        // read fadespeedMultiplier for both fades from pot 1
        int p = analogRead(pot[0]);
        fadeSpeedMultiplier[0] = map_float(p, 0, 1023, 0.05, 1.5);
        fadeSpeedMultiplier[1] = map_float(p, 0, 1023, 0.05, 1.5);

        fadeColor(0);
        fadeColor(1);

        if (buttonState == LOW and sliderState == LOW) {
          mySerial.println("changing mode once both fades are done");
          changingMode = 0;
        }

        if (changingMode == FINISHED) {
          mySerial.println("now changing mode");
          mode = 1;
          changingMode = -1;
        }

        break;
      }
    case 1:
      {
        // II: to farver

        //mySerial.print("changing mode: ");
        //mySerial.println(changingMode);

        int p = analogRead(pot[0]);
        fadeSpeedMultiplier[0] = map_float(p, 0, 1023, 0.05, 1.5);
        p = analogRead(pot[1]);
        fadeSpeedMultiplier[1] = map_float(p, 0, 1023, 0.05, 1.5);

        fadeColor(0);
        fadeColor(1);

        if (buttonState == LOW and sliderState == LOW) {
          mySerial.println("changing mode once both fades are done");
          changingMode = CHANGING;
        }

        if (changingMode == FINISHED) {
          mySerial.println("now changing mode");
          nextColor[0] = 7;
          nextColor[1] = 0;
          mode = 0;
          changingMode = -1;
        }
        break;
      }

    case 2:
      {
        // fade to next color
        break;
      }

    default:
      break;
  }

  delay(50);
}

void fadeColor(int c) {

  // fade between colors

  feedback = false;

  if (feedback) {
    mySerial.print("color: ");
    mySerial.print(c);
  }

  // speed changes according to distance from sensor

  //mySerial.print("normalized distance: ");
  //mySerial.println(normalizedDistance);

  if (normalizedDistance > 0.1 && normalizedDistance < 0.95) {
    //mySerial.print("P ");
    if (peoplePresent == false) {
      mySerial.println("person entered");
      lastPerson = millis();
      peoplePresent = true; 
    }
      
    fadeSpeed[c] = map_float(normalizedDistance,0,1,0,200);
  } else {

    if (peoplePresent == true) {
      mySerial.print("person left after ");
      int duration = millis() - lastPerson;
      mySerial.println(duration);
      peoplePresent = false;

      if (duration < 500) {
        // person passed
        if ((lastFlip + 20000 )< millis()) {
          flipped = !flipped;
          lastFlip = millis();
        }
      }
      
    }
    
   fadeSpeed[c] = map_float(normalizedDistance,0,1,0,25);
  }
  fadeSpeed[c] = fadeSpeed[c] * fadeSpeedMultiplier[c];
  //mySerial.print("fadeSpeed ");
  //mySerial.print(c);
  //mySerial.print(": ");
  //mySerial.println(fadeSpeed[c]);

  if (fadeCompleted[c] == false) {
    //mySerial.print("progression ");
    //mySerial.println(fadeProgression[c]);
    fadeProgression[c] = fadeProgression[c] + fadeSpeed[c];
  }

  //nextColor[c] = (currentColor[c] + 1) % numberOfColors;

  if (fadeProgression[c] > fadeCycleLength) {
    if (changingMode == NO) {
      currentColor[c] ++;
      currentColor[c] = currentColor[c] % numberOfColors;
      nextColor[c] = (currentColor[c] + 1) % numberOfColors;
      fadeProgression[c] = 0;

      mySerial.print("color: ");
      mySerial.print(c);
      mySerial.print(" new color fade from color ");
      mySerial.print(currentColor[c]);
      mySerial.print(" to color ");
      mySerial.println(nextColor[c]);
    } else if (changingMode == CHANGING) {
      fadeCompleted[c] = true;
      mySerial.print(c);
      mySerial.println(" has ended");
    }
    if (fadeCompleted[0] && fadeCompleted[1]) {
      mySerial.println("both fades have ended");
      fadeProgression[0] = 0;
      fadeProgression[1] = 0;
      fadeCompleted[0] = false;
      fadeCompleted[1] = false;
      changingMode = FINISHED;
    }

  }

  byte _r1 = (byte) pgm_read_word_near(&data[currentColor[c] * 4]);
  byte _g1 = (byte) pgm_read_word_near(&data[currentColor[c] * 4 + 1]);
  byte _b1 = (byte) pgm_read_word_near(&data[currentColor[c] * 4 + 2]);
  byte _w1 = (byte) pgm_read_word_near(&data[currentColor[c] * 4 + 3]);

  byte _r2 = (byte) pgm_read_word_near(&data[nextColor[c] * 4]);
  byte _g2 = (byte) pgm_read_word_near(&data[nextColor[c] * 4 + 1]);
  byte _b2 = (byte) pgm_read_word_near(&data[nextColor[c] * 4 + 2]);
  byte _w2 = (byte) pgm_read_word_near(&data[nextColor[c] * 4 + 3]);

  r[c] = map(fadeProgression[c], 0, fadeCycleLength, _r1, _r2);
  g[c] = map(fadeProgression[c], 0, fadeCycleLength, _g1, _g2);
  b[c] = map(fadeProgression[c], 0, fadeCycleLength, _b1, _b2);
  w[c] = map(fadeProgression[c], 0, fadeCycleLength, _w1, _w2);

  // apply general brightness
  r[c] = map(r[c], 0, 255, 0, generalBrightness);
  g[c] = map(g[c], 0, 255, 0, generalBrightness);
  b[c] = map(b[c], 0, 255, 0, generalBrightness);
  w[c] = map(w[c], 0, 255, 0, generalBrightness);

  if (feedback) {
    mySerial.print("\t");
    mySerial.print(_r1); mySerial.print(",");
    mySerial.print(_g1); mySerial.print(",");
    mySerial.print(_b1); mySerial.print(",");
    mySerial.print(_w1); mySerial.print(" -> ");
    mySerial.print(_r2); mySerial.print(",");
    mySerial.print(_g2); mySerial.print(",");
    mySerial.print(_b2); mySerial.print(",");
    mySerial.print(_w2); mySerial.print(" = ");
    mySerial.print(_r1); mySerial.print(",");
    mySerial.print(_g1); mySerial.print(",");
    mySerial.print(_b1); mySerial.print(",");
    mySerial.print(_w1);

    mySerial.print("\t\t multiplier: "), mySerial.print(fadeSpeedMultiplier[c]);
    mySerial.print("\t speed: "), mySerial.print(fadeSpeed[c] * 100);
    mySerial.print("\t progress: "), mySerial.print(fadeProgression[c]);
    mySerial.print("\t brightness: "), mySerial.println(generalBrightness);
  }


  if (buttonState == LOW) {
    mode = 0;
    delay(200);
  }

  if (flipped && c == 0) {
    flipColor(c);
  }

  sendDMX(c, r[c], g[c], b[c], w[c]);

}


void flipColor(int _c) {
  r[_c] = 255-r[_c];
  g[_c] = 255-g[_c];
  b[_c] = 255-b[_c];
  w[_c] = 255-w[_c];
}

void tick() {

  feedback = false;

  if (lastTick + 1000 < millis()) {

    seconds ++;
    if (seconds > 59) {
      minutes ++;
      seconds = 0;

      if (minutes > 59) {
        hours ++;
        minutes = 0;

        if (hours > 23) {
          days ++;
          hours = 0;
        }
      }

    }

    if (minutes < 10 && feedback) mySerial.print("0");
    if (feedback) mySerial.print(minutes);
    if (feedback) mySerial.print(":");
    if (seconds < 10 && feedback) mySerial.print("0");
    if (feedback) mySerial.println(seconds);

    lastTick = millis();

  }

  if (lastFeedback + feedbackFrequency < millis()) {
    feedback = true;
    lastFeedback = millis();
  } else {
    feedback = false;
  }

}

void sendDMX(int _lamp, int _r, int _g, int _b, int _w) {
  // Update DMX
  if (_lamp == 0) {
    DmxSimple.write(1, _r);          // R
    DmxSimple.write(2, _g);          // G
    DmxSimple.write(3, _b);          // B
    DmxSimple.write(4, _w);          // W
  }
  if (_lamp == 1) {
    DmxSimple.write(5, _r);          // R
    DmxSimple.write(6, _g);          // G
    DmxSimple.write(7, _b);          // B
    DmxSimple.write(8, _w);          // W
  }
}

void getLeddar() {
  char result = Leddar1.getDetections();
  if (result >= 0)
  {
    // Show the first detection only
    Distance = Leddar1.Detections[0].Distance;
    //Amplitude = Leddar1.Detections[0].Amplitude;
  } else {
    mySerial.print("Error: "); Serial.println((int)result);
  }
  //mySerial.print("leddar distance: "); mySerial.println(Distance);
  
  maxDistance = max(Distance, maxDistance);
  if (maxDistance > 0) maxDistance--; // normalize max distance to prevent error reading maxing out max distance
  normalizedDistance = float(Distance) / float(maxDistance);
  //mySerial.print("max distance: "); mySerial.println(maxDistance);
  
}

void colorPicker() {
  byte r = analogRead(pot[0]) / 4;
  byte g = analogRead(pot[1]) / 4;
  byte b = analogRead(pot[2]) / 4;
  byte w = analogRead(pot[3]) / 4;

  /*
    byte l = analogRead(lightSensor);
    mySerial.print("lightsensor: ");
    mySerial.println(l);
  */

  if (digitalRead(slider) == LOW) {
    DmxSimple.write(1, r);          // R
    DmxSimple.write(2, g);          // G
    DmxSimple.write(3, b);          // B
    DmxSimple.write(4, w);          // W
  } else {
    DmxSimple.write(5, r);          // R
    DmxSimple.write(6, g);          // G
    DmxSimple.write(7, b);          // B
    DmxSimple.write(8, w);          // W
  }

  if (buttonState == false) {
    mySerial.print(r);
    mySerial.print(",");
    mySerial.print(g);
    mySerial.print(",");
    mySerial.print(b);
    mySerial.print(",");
    mySerial.println(w);
    delay(100);
  }
}

float map_float(double x, double in_min, double in_max, double out_min, double out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

