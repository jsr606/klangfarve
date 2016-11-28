// viera collaro
// RGBW input box
// leddar one sensor
// DMX color control

// simplified electronics version: PIR in stead of leddarOne and no potetiometers

#include <avr/pgmspace.h>
#include <DmxSimple.h>
#include <EEPROM.h>

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

const int led = 13;
const int pir = 12;

int mode = 1;
int nextMode = -1;
int changingMode = -1;

int dayCount;

int colorToFlip;

#define NO -1
#define INIT 0
#define CHANGING 1
#define FINISHED 2
#define ONECOLOR 0
#define TWOCOLORS 1
#define WHITEOUTSIDE 2
#define WHITEINSIDE 3
#define COLORPICKER -1

unsigned long lastFeedback = millis(), lastTick = millis();
const int feedbackFrequency = 200;
boolean feedback = false;
byte seconds = 0, minutes = 0, hours = 0, days = 0;

boolean peoplePresent = false;
unsigned long lastPerson = millis();
unsigned long lastFlip = millis();
boolean flipped = false;

const long fadeCycleLength = 10000;

#define flipTime 100
byte flipCount = 0;
#define maxFlips 10

int speedRandomizer;
float normalizedDistance = 0.75;

void setup()
{
  // init DMX
  pinMode(jrde, OUTPUT);
  pinMode(jdi,  OUTPUT);
  pinMode(jro,  INPUT);

  digitalWrite(jrde, HIGH);
  DmxSimple.usePin(jdi);
  DmxSimple.maxChannel(9);

  pinMode (led, OUTPUT);
  pinMode (pir, INPUT);

  Serial.begin(9600);

  Serial.print("number of colors: "); Serial.println(numberOfColors);

  // reset day count
  // int day0 = 0;
  // EEPROM.put(0, day0);

  EEPROM.get(0, dayCount);
  dayCount ++;
  EEPROM.put(0, dayCount);
  Serial.print("today is day number "); Serial.println(dayCount);
  randomSeed(dayCount);

  speedRandomizer = random(100)-50;
  Serial.print("toadys speed randomizer is "); Serial.println(speedRandomizer);
  
  // select mode of the day
  int ran = random(30);
  Serial.print("toadys random number is "); Serial.println(ran);
  
  if (ran == 0) {
    // super special case, only one color

    colorToFlip = -1; // dont flip

    Serial.println("super special day, only one color ");
    mode = ONECOLOR;
  } else {
    if (ran < 20) {
      Serial.println("two colors");
      mode = TWOCOLORS;

      colorToFlip = 0;

      for (int i = 0; i < 2; i++) {
        currentColor[i] = random(numberOfColors);
        nextColor[i] = (currentColor[i] + 1) % numberOfColors;
        fadeProgression[i] = 0;
        Serial.print("start colors ");
        Serial.print(currentColor[i]);
        Serial.print(", ");
        Serial.println(nextColor[i]);
      }
    } else {
      if ((ran % 2) == 0) {
        mode = WHITEOUTSIDE;

        colorToFlip = 0;

        currentColor[0] = random(numberOfColors);
        nextColor[0] = (currentColor[0] + 1) % numberOfColors;
        currentColor[1] = 7;
        nextColor[1] = 7;

        Serial.print("white outside, color inside: ");
        Serial.print(currentColor[0]);
        Serial.print(", ");
        Serial.println(nextColor[0]);

      } else {
        mode = WHITEINSIDE;

        colorToFlip = 1;

        currentColor[1] = random(numberOfColors);
        nextColor[1] = (currentColor[1] + 1) % numberOfColors;
        currentColor[0] = 7;
        nextColor[0] = 7;

        Serial.print("white inside, color outside: ");
        Serial.print(currentColor[1]);
        Serial.print(", ");
        Serial.println(nextColor[1]);
      }
    }
  }

  byte startAnimationDelay = 100;

  sendDMX(0, 255,0,0,0);
  delay(startAnimationDelay);
  sendDMX(0, 0,255,0,0);
  delay(startAnimationDelay);
  sendDMX(0, 0,0,255,0);
  delay(startAnimationDelay);
  sendDMX(0, 0,0,0,255);
  delay(startAnimationDelay);

  sendDMX(1, 255,0,0,0);
  delay(startAnimationDelay);
  sendDMX(1, 0,255,0,0);
  delay(startAnimationDelay);
  sendDMX(1, 0,0,255,0);
  delay(startAnimationDelay);
  sendDMX(1, 0,0,0,255);
  delay(startAnimationDelay);
  
}

void loop()
{

  tick();
  getPIR();

  switch (mode) {
    case COLORPICKER:
      //colorPicker();
      break;
    case ONECOLOR:
      {
        // I: en farve
        // read fadespeedMultiplier for both fades from pot 1

        int p = 512+speedRandomizer;
        fadeSpeedMultiplier[0] = map_float(p, 0, 1023, 0.05, 1.5);
        fadeSpeedMultiplier[1] = map_float(p, 0, 1023, 0.05, 1.5);

        fadeColor(0);
        fadeColor(1);

        break;

      }
    case TWOCOLORS:
      {
        // II: to farver

        //Serial.print("changing mode: ");
        //Serial.println(changingMode);

        int p = 512+speedRandomizer;
        fadeSpeedMultiplier[0] = map_float(p, 0, 1023, 0.05, 1.5);
        p = 512-speedRandomizer;
        fadeSpeedMultiplier[1] = map_float(p, 0, 1023, 0.05, 1.5);

        fadeColor(0);
        fadeColor(1);

        break;
      }

    case WHITEINSIDE:
      {
        int p = 512+speedRandomizer;
        fadeSpeedMultiplier[0] = map_float(p, 0, 1023, 0.05, 1.5);
        p = 512-speedRandomizer;
        fadeSpeedMultiplier[1] = map_float(p, 0, 1023, 0.05, 1.5);

        fadeColor(1);
        sendColor(0);
        
        break;
      }

    case WHITEOUTSIDE:
      {
        int p = 512+speedRandomizer;
        fadeSpeedMultiplier[0] = map_float(p, 0, 1023, 0.05, 1.5);
        p = 512-speedRandomizer;
        fadeSpeedMultiplier[1] = map_float(p, 0, 1023, 0.05, 1.5);

        fadeColor(0);
        sendColor(1);

        break;
      }

    default:
      break;
  }

  delay(50);
}


void getPIR() {

    if (digitalRead(pir) == true) {
      digitalWrite(led, HIGH);
      peoplePresent = true;
      
    } else {
      digitalWrite(led, LOW);
      peoplePresent = false;
    }

}

void fadeColor(int c) {

  // fade between colors

  feedback = false;

  if (feedback) {
    Serial.print("color: ");
    Serial.print(c);
  }

  if (minutes % 7 == 0) {
    if (peoplePresent) {
      if (flipCount > maxFlips) {
        Serial.println("maxed out on flips");
      } else {

        if (lastFlip + flipTime < millis()) {
          flipped = !flipped;
          lastFlip = millis();
          flipCount++;
        }
      }
    }
  } else {
    flipCount = 0;
  }

  /*

    if (normalizedDistance > 0.01 && normalizedDistance < 0.95) {
    //Serial.print("P ");
    if (peoplePresent == false) {
      Serial.println("person entered");
      lastPerson = millis();
      peoplePresent = true;
    }

    //fadeSpeed[c] = map_float(normalizedDistance, 0, 1, 0, 200);
    } else {

    if (peoplePresent == true) {
      Serial.print("person left after ");
      int duration = millis() - lastPerson;
      Serial.println(duration);
      peoplePresent = false;

      if (duration < 500) {
        // person passed
        if ((lastFlip + 5000 ) < millis()) { // should be 20000
          flipped = !flipped;
          lastFlip = millis();
        }
      }

    } else {
      if ((lastFlip + 10000) < millis()) {
        flipped = false;
      }
    }

    //fadeSpeed[c] = map_float(normalizedDistance, 0, 1, 0, 25);
    }
  */
  fadeSpeed[c] = map_float(constrain(normalizedDistance, 0.5, 1), 0, 1, 0, 50);
  //Serial.print("fadeSpeed ");
  //Serial.println(fadeSpeed[c]);
  fadeSpeed[c] = fadeSpeed[c] * fadeSpeedMultiplier[c];
  //Serial.print("fadeSpeed ");
  //Serial.print(c);
  //Serial.print(": ");
  //Serial.println(fadeSpeed[c]);

  fadeProgression[c] = fadeProgression[c] + fadeSpeed[c];
  //Serial.print(c);
  //Serial.print(" progression ");
  //Serial.println(fadeProgression[c]);

  if (fadeProgression[c] > fadeCycleLength) {
    currentColor[c] ++;
    currentColor[c] = currentColor[c] % numberOfColors;
    nextColor[c] = (currentColor[c] + 1) % numberOfColors;
    fadeProgression[c] = 0;

    Serial.print("color: ");
    Serial.print(c);
    Serial.print(" new color fade from color ");
    Serial.print(currentColor[c]);
    Serial.print(" to color ");
    Serial.println(nextColor[c]);
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
  //r[c] = map(r[c], 0, 255, 0, generalBrightness);
  //g[c] = map(g[c], 0, 255, 0, generalBrightness);
  //b[c] = map(b[c], 0, 255, 0, generalBrightness);
  //w[c] = map(w[c], 0, 255, 0, generalBrightness);

  if (feedback) {
    Serial.print("\t");
    Serial.print(_r1); Serial.print(",");
    Serial.print(_g1); Serial.print(",");
    Serial.print(_b1); Serial.print(",");
    Serial.print(_w1); Serial.print(" -> ");
    Serial.print(_r2); Serial.print(",");
    Serial.print(_g2); Serial.print(",");
    Serial.print(_b2); Serial.print(",");
    Serial.print(_w2); Serial.print(" = ");
    Serial.print(_r1); Serial.print(",");
    Serial.print(_g1); Serial.print(",");
    Serial.print(_b1); Serial.print(",");
    Serial.print(_w1);

    Serial.print("\t\t multiplier: "), Serial.print(fadeSpeedMultiplier[c]);
    Serial.print("\t speed: "), Serial.print(fadeSpeed[c] * 100);
    Serial.print("\t progress: "), Serial.print(fadeProgression[c]);
    Serial.print("\t brightness: "), Serial.println(generalBrightness);
  }

  /*
    if (buttonState == LOW) {
    mode = 0;
    delay(200);
    }
  */

  if (flipped && colorToFlip == c) {
    flipColor(colorToFlip);
  }

  sendDMX(c, r[c], g[c], b[c], w[c]);

}

void sendColor (int c) {

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
  //r[c] = map(r[c], 0, 255, 0, generalBrightness);
  //g[c] = map(g[c], 0, 255, 0, generalBrightness);
  //b[c] = map(b[c], 0, 255, 0, generalBrightness);
  //w[c] = map(w[c], 0, 255, 0, generalBrightness);

  sendDMX(c, r[c], g[c], b[c], w[c]);
}


void flipColor(int _c) {
  r[_c] = 255 - r[_c];
  g[_c] = 255 - g[_c];
  b[_c] = 255 - b[_c];
  w[_c] = 255 - w[_c];
}

void tick() {

  feedback = true;

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

    if (feedback && mode == ONECOLOR) Serial.print("one color: ");
    if (feedback && mode == TWOCOLORS) Serial.print("two colors: ");
    if (feedback && mode == WHITEINSIDE) Serial.print("white inside: ");
    if (feedback && mode == WHITEOUTSIDE) Serial.print("white outside: ");

    if (feedback) Serial.print(dayCount + days);
    if (feedback) Serial.print(": ");
    if (hours < 10 && feedback) Serial.print("0");
    if (feedback) Serial.print(hours);
    if (feedback) Serial.print(":");
    if (minutes < 10 && feedback) Serial.print("0");
    if (feedback) Serial.print(minutes);
    if (feedback) Serial.print(":");
    if (seconds < 10 && feedback) Serial.print("0");
    if (feedback) Serial.println(seconds);

    lastTick = millis();
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

float map_float(double x, double in_min, double in_max, double out_min, double out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

