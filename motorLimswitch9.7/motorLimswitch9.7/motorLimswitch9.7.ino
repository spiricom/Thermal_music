// speedPlot.ino

// thie millisDelay library is available from https://www.forward.com.au/pfod/ArduinoProgramming/TimingDelaysInArduino.html
#include <millisDelay.h>
// the SpeedStepper library is available from https://www.forward.com.au/pfod/Robots/SpeedStepper/index.html
#include "SpeedStepper.h"
#include <limits.h>  // for max,min values
#include <pfodBufferedStream.h>
#include "TeensyDMX.h"
namespace teensydmx = ::qindesign::teensydmx;
teensydmx::Sender dmxTx{Serial5};

float lights[2][3];
// Data for 3 channels.
//uint8_t data[3]{0x44, 0x88, 0xcc};
/*
   (c)2019 Forward Computing and Control Pty. Ltd.
   NSW Australia, www.forward.com.au
   This code is not warranted to be fit for any purpose. You may only use it at your own risk.
   This generated code may be freely used for both private and commercial use
   provided this copyright is maintained.
*/

// stepperPlot.ino
// this sketch will plot speed and position
// on the Arduino IDE MY_SERIAL Plotter
// The plot will start 10sec after the board is programmed/reset
// It illustrates a number of speed changes, hitting a position limit and going home.
// See the speedStepperPlot.jpg in this directory for sample output
// max time to computeNewStep for Arduino UNO is 208us, for Arduino Mega is 216us
// max time time to computeNewStep for SparkFun Redboard Turbo is 154us
//

// ========== SETTINS FOR MOTOR STUFF ========
#define LIM_SWITCH_TOP_PIN1 5
#define LIM_SWITCH_TOP_PIN2 6
#define LIM_SWITCH_BOT_PIN1 7
#define LIM_SWITCH_BOT_PIN2 8

#define OPEN_BUTTON 11
#define CLOSE_BUTTON 12
#define SPEED_POT A0
#define DIST_POT A1

int32_t calibratedRange1 = 0;
int32_t calibratedRange2 = 0;
int32_t scaledPlusLimit = 0;
int32_t scaledPlusLimit2 = 0;
int32_t midiScaledPlusLimit = 0;
int32_t midiScaledPlusLimit2 = 0;
// change these pin definitions to match you motor driver
// NOTE carefully do not use pins 6,7,8,9,10,11 on ESP32 / ESP8266
const int STEP_PIN = 1;
const int DIR_PIN = 2;

const int STEP_PIN2 = 3;
const int DIR_PIN2 = 4;

uint16_t counter = 0;

SpeedStepper stepperTop(STEP_PIN, DIR_PIN);
SpeedStepper stepperBot(STEP_PIN2, DIR_PIN2);


uint8_t addressMap[4] = {0, 1, 3, 2};
int addressPin0 = 20;
int addressPin1 = 21;
uint8_t myAddress = 0;

int32_t toppluslimit = 0;
int32_t topminuslimit = 0;
int32_t bottompluslimit = 0;
int32_t bottomminuslimit = 0;

int32_t closeOffset = 2;

int distVal1 = 0;
int distVal2 = 0;
int initSpeed = 200;
float speedVal = 50.f;
//const int limit_switches[4] = {/*topMotorlim1, bottomMotorlim1, topMotorlim2,  bottomMotorlim2*/0,1,2,0};
void setup() {


  pinMode(addressPin0, INPUT_PULLUP);
  pinMode(addressPin1, INPUT_PULLUP);
  delay(500);
  myAddress = digitalRead(addressPin0) + (digitalRead(addressPin1) << 1);
  pinMode(addressPin0, OUTPUT);
  pinMode(addressPin1, OUTPUT);

  pinMode(LIM_SWITCH_TOP_PIN1, INPUT_PULLUP);
  pinMode(LIM_SWITCH_TOP_PIN2, INPUT_PULLUP);
  pinMode(LIM_SWITCH_BOT_PIN1, INPUT_PULLUP);
  pinMode(LIM_SWITCH_BOT_PIN2, INPUT_PULLUP);
  pinMode(13, OUTPUT);
  delay(10);
  pinMode(OPEN_BUTTON, INPUT_PULLUP);
  pinMode(CLOSE_BUTTON, INPUT_PULLUP);
  pinMode(SPEED_POT, INPUT);
  pinMode(DIST_POT, INPUT);
  Serial.begin(9600);
  Serial.println("hello");
  Serial.print("myAddress = ");
  Serial.println(myAddress);
  

  dmxTx.begin();



  usbMIDI.setHandleNoteOn(myNoteOn);
  usbMIDI.setHandleControlChange(OnControlChange);
  attachInterrupt(digitalPinToInterrupt(OPEN_BUTTON), openPressed, FALLING);
  attachInterrupt(digitalPinToInterrupt(CLOSE_BUTTON), closePressed, FALLING);
  digitalWrite(13, HIGH);
  bool findLimit = 0;
  stepperTop.setAcceleration(500000);
  stepperBot.setAcceleration(500000);
  //stepperTop.setAcceleration(10);
  //stepperBot.setAcceleration(10);
  stepperTop.setSpeed(-initSpeed);
  stepperBot.setSpeed(-initSpeed);

  
//  while(1)
//  {
//    Serial.println("top close top open bottom open bottom close");
//    Serial.println(digitalRead(LIM_SWITCH_TOP_PIN1));
//    Serial.println(digitalRead(LIM_SWITCH_TOP_PIN2));
//    Serial.println(digitalRead(LIM_SWITCH_BOT_PIN1));
//    Serial.println(digitalRead(LIM_SWITCH_BOT_PIN2));
//  }
   
  while (findLimit == 0)
  {
    stepperTop.run();
    //stepperBot.run();
    //Serial.println(findLimit);
    int buttonPressed = digitalRead(LIM_SWITCH_TOP_PIN2);
    //Serial.println(buttonPressed);
    
   
    if (buttonPressed == 0)
    {
      stepperTop.stopAndSetHome();
      stepperTop.setMinusLimit(stepperTop.getCurrentPosition());
      
       Serial.println("top open limit");
      //stepperTop.stopAndSetHome();
      findLimit++;
    }
  }
  findLimit = 0;
  while (findLimit == 0)
  {
    //stepperTop.run();
    stepperBot.run();
   
    if (digitalRead(LIM_SWITCH_BOT_PIN1) == 0)
    {
      stepperBot.stopAndSetHome();
      Serial.println(stepperBot.getCurrentPosition());
      stepperBot.setMinusLimit(stepperBot.getCurrentPosition()+closeOffset);
      Serial.println("lim switch bottom close");

      Serial.println(stepperBot.getMinusLimit());
      findLimit++;

    }
  }
  //stepperTop.goHome();
  Serial.println("MOVIN ON");
  stepperTop.setSpeed(initSpeed);
  stepperBot.setSpeed(initSpeed);
  
  findLimit = 0;
  while (findLimit ==0)
  {
    stepperTop.run();  
   
    if (digitalRead(LIM_SWITCH_TOP_PIN1) == 0)
    {
      Serial.println("lim switch top close");
      stepperTop.setPlusLimit(stepperTop.getCurrentPosition()-closeOffset);
      Serial.println(stepperTop.getCurrentPosition());
      Serial.println(stepperTop.getPlusLimit());
      scaledPlusLimit = (stepperTop.getCurrentPosition()) * 1024;
      midiScaledPlusLimit = (stepperTop.getCurrentPosition()) * 127;
      findLimit++;
    }
  }
  
  findLimit = 0;
  while (findLimit == 0)
  {
    //stepperTop.run();
    stepperBot.run();
 
    if (digitalRead(LIM_SWITCH_BOT_PIN2) == 0)
    {
     Serial.println(stepperBot.getCurrentPosition());
      stepperBot.setPlusLimit(stepperBot.getCurrentPosition());
      Serial.println("bottom open limit");
      
      scaledPlusLimit2 = stepperBot.getCurrentPosition() * 1024;
      midiScaledPlusLimit2 = stepperBot.getCurrentPosition() * 127;
      findLimit++;
    }
  }
  Serial.println("DONE,BRO");
  calibratedRange1 = stepperTop.getPlusLimit();
  calibratedRange2 = stepperBot.getPlusLimit();

  toppluslimit = calibratedRange1;
  topminuslimit = 0;
  bottompluslimit = calibratedRange2;
  bottomminuslimit = 0;
  
  stepperBot.setSpeed(-initSpeed);
  //stepperTop.setSpeed(50);
  //stepperBot.setSpeed(-50);
  //stepperBot.stop();
  stepperTop.stop();
  digitalWrite(13, LOW);
  
  //set up the DMX lighting control pin

  //dmxTx.set(4, 255);
  //dmxTx.set(8, 255);

  
  //limit switch interrupts
  attachInterrupt(digitalPinToInterrupt(LIM_SWITCH_TOP_PIN2), topOpenLimitHit, FALLING);
  attachInterrupt(digitalPinToInterrupt(LIM_SWITCH_TOP_PIN1), topClosedLimitHit, FALLING);
  attachInterrupt(digitalPinToInterrupt(LIM_SWITCH_BOT_PIN2), bottomOpenLimitHit, FALLING);
  attachInterrupt(digitalPinToInterrupt(LIM_SWITCH_BOT_PIN1), bottomClosedLimitHit, FALLING);
}

void newDMXmessage(uint8_t light, float H, float S, float V)
{
    uint8_t redTemp = 0;
    uint8_t greenTemp = 0;
    uint8_t blueTemp = 0;
    HsvToRgb(H, S, V, redTemp, greenTemp, blueTemp);
    Serial.println(light * 4 + 1);
    dmxTx.set(light * 4 + 1, redTemp);
    dmxTx.set(light * 4 + 2, greenTemp);
    dmxTx.set(light * 4 + 3, blueTemp);
}

void  HsvToRgb(double hue, double saturation, double value, uint8_t& red, uint8_t& green, uint8_t& blue)
{
  double r, g, b;

  auto i = static_cast<int>(hue * 6);
  auto f = hue * 6 - i;
  auto p = value * (1 - saturation);
  auto q = value * (1 - f * saturation);
  auto t = value * (1 - (1 - f) * saturation);

  switch (i % 6)
  {
  case 0: r = value , g = t , b = p;
    break;
  case 1: r = q , g = value , b = p;
    break;
  case 2: r = p , g = value , b = t;
    break;
  case 3: r = p , g = q , b = value;
    break;
  case 4: r = t , g = p , b = value;
    break;
  case 5: r = value , g = p , b = q;
    break;
  }

  red = static_cast<uint8_t>(r * 255);
  green = static_cast<uint8_t>(g * 255);
  blue = static_cast<uint8_t>(b * 255);
}

void loop() {
  stepperTop.run();
  stepperBot.run();
 // Serial.println(digitalRead(LIM_SWITCH_TOP_PIN2));
  //Serial.println(stepperBot.getCurrentPosition());
  usbMIDI.read();
}

void openPressed()
{
  static unsigned long last_open_interrupt_time = 0;
  unsigned long open_interrupt_time = millis();
  // If interrupts come faster than 200ms, assume it's a bounce and ignore
  if (open_interrupt_time - last_open_interrupt_time > 200)
  {
    float _speedVal = analogRead(SPEED_POT);
    //int distVal = scaledPlusLimit / analogRead(DIST_POT);
    int _distVal1 = scaledPlusLimit / analogRead(DIST_POT);
    int _distVal2 = scaledPlusLimit2 / analogRead(DIST_POT);
    //Serial.println(analogRead(SPEED_POT));
    Serial.println("open");
    stepperTop.setSpeed(-_speedVal);
    stepperBot.setSpeed(_speedVal);

    //stepperTop.setCurrentPosition(stepperTop.getPlusLimit());
  }
  last_open_interrupt_time = open_interrupt_time;


}

void closePressed()
{
  static unsigned long last_close_interrupt_time = 0;
  unsigned long close_interrupt_time = millis();
  // If interrupts come faster than 200ms, assume it's a bounce and ignore
  if (close_interrupt_time - last_close_interrupt_time > 200)
  {
    float _speedVal = analogRead(SPEED_POT);
    float _distVal1 = scaledPlusLimit / analogRead(DIST_POT);
    float _distVal2 = scaledPlusLimit2 / analogRead(DIST_POT);

    stepperTop.setSpeed(_speedVal);
    stepperBot.setSpeed(-_speedVal);
    Serial.println("close");
    //stepperTop.setCurrentPosition(scaledPlusLimit-distVal);
  }
  last_close_interrupt_time = close_interrupt_time;
}


void midiOpen(int value)
{
  if (value != 0)
  {
    int scaledValue = (127.0f - (float)value) * ((float)calibratedRange1 / 127.0f) + (float)topminuslimit;
    Serial.println(scaledValue);
    if ((scaledValue >= topminuslimit) && (scaledValue <= toppluslimit))
    {
      stepperTop.setMinusLimit(scaledValue);
      stepperTop.setPlusLimit(toppluslimit);
    }
    scaledValue = (float)value * ((float)calibratedRange2 / 127.0f) + (float)bottomminuslimit;
    if ((scaledValue >= bottomminuslimit) && (scaledValue <= bottompluslimit))
    {
      stepperBot.setPlusLimit(scaledValue);
      stepperBot.setMinusLimit(bottomminuslimit);
    }
  }
  
  
  
  stepperTop.setSpeed(-speedVal);
  stepperBot.setSpeed(speedVal);
}

void midiClose(int velocity)
{
  stepperTop.setPlusLimit(toppluslimit);
  stepperBot.setMinusLimit(bottomminuslimit);
  stepperTop.setSpeed(speedVal);
  stepperBot.setSpeed(-speedVal);
}

void myNoteOn(byte channel, byte note, byte velocity)
{
  //Serial.println(channel);
  //Serial.println(addressMap[myAddress]);
  //if (channel != addressMap[myAddress])
  {
  //  return;
  }
  if (note == 60)
  {
    midiOpen(velocity);
  }
  else if (note == 61)
  {
    midiClose(velocity);
  }
}


void OnControlChange(byte channel, byte control, byte value)
{
  //if (channel != addressMap[myAddress])
  {
  //  return;
  }
  if (control == 7 ) //speed
  {
    speedVal = 8.f * value;
  }
  else if (control  == 8) // accel
  {
    stepperTop.setAcceleration(value * 80.f);
    stepperBot.setAcceleration(value * 80.f);
  }
  else if (control == 9) //open/close amount(distance)
  {
    int scaledValue = (127.0f - value) * (calibratedRange1 / 127.0f) + topminuslimit;

    if ((scaledValue >= topminuslimit) && (scaledValue <= toppluslimit))
    {
      stepperTop.setMinusLimit(scaledValue);
      stepperTop.setPlusLimit(toppluslimit);
    }
    scaledValue = value * (calibratedRange2 / 127.0f) + bottomminuslimit;
    if ((scaledValue >= bottomminuslimit) && (scaledValue <= bottompluslimit))
    {
      stepperBot.setPlusLimit(scaledValue);
      stepperBot.setMinusLimit(bottomminuslimit);
    }
  }

  else if (control  == 10) // top position
  {
    
    int scaledValue = ((127.0f - (float)value) * ((float)calibratedRange1 / 127.0f)) + topminuslimit;
    Serial.println(scaledValue);
    Serial.println(stepperTop.getCurrentPosition());
    if ((scaledValue > stepperTop.getCurrentPosition()) && (scaledValue <= toppluslimit))
    {
      stepperTop.setPlusLimit(scaledValue);
      stepperTop.setSpeed(speedVal);
      Serial.println("forward");

    }
    else if ((scaledValue < stepperTop.getCurrentPosition()) && (scaledValue >= topminuslimit))
    {
      stepperTop.setMinusLimit(scaledValue);
      stepperTop.setSpeed(-speedVal);
      Serial.println("backweard");

    }
  }

  else if (control  == 11) // bottom position
  {
    int scaledValue = ((float)value * ((float)calibratedRange2 / 127.0f))  + bottomminuslimit;
    Serial.println(scaledValue);
    Serial.println(stepperBot.getCurrentPosition());
    if ((scaledValue > stepperBot.getCurrentPosition()) && (scaledValue <= bottompluslimit))
    {
      stepperBot.setPlusLimit(scaledValue);
      stepperBot.setSpeed(speedVal);
      Serial.println("forward");

    }
    else if ((scaledValue < stepperBot.getCurrentPosition()) && (scaledValue >= bottomminuslimit))
    {
      stepperBot.setMinusLimit(scaledValue);
      stepperBot.setSpeed(-speedVal);
      Serial.println("backweard");
    }
  }

  else if (control  == 12) // both position
  {
    int32_t scaledValue = (127.0f - (float)value) * (((float)calibratedRange1 / 127.0f)) +topminuslimit;
    Serial.println(calibratedRange1);  
   Serial.println("topminuslimit"); 
    Serial.println(topminuslimit);
    Serial.println("toppluslimit"); 
    Serial.println(toppluslimit);
    Serial.println("topscaledvalue");
    Serial.println(scaledValue);
    Serial.println(stepperTop.getCurrentPosition());
    
    if ((scaledValue > stepperTop.getCurrentPosition()) && (scaledValue <= toppluslimit))
    {
      stepperTop.setPlusLimit(scaledValue);
      stepperTop.setSpeed(speedVal);
      Serial.println("forward");
    }
    else if ((scaledValue < stepperTop.getCurrentPosition()) && (scaledValue >= topminuslimit))
    {
      stepperTop.setMinusLimit(scaledValue);
      stepperTop.setSpeed(-speedVal);
      Serial.println("backweard");
    }

    scaledValue = ((float)value * ((float)calibratedRange2 / 127.0f)) + bottomminuslimit;
    Serial.println("bottomminuslimit");
    Serial.println(bottomminuslimit);
    Serial.println("bottompluslimit"); 
    Serial.println(bottompluslimit);
    Serial.println("bottomscaledvalue");
    Serial.println(scaledValue);
    Serial.println(stepperBot.getCurrentPosition());
    
    if ((scaledValue > stepperBot.getCurrentPosition()) && (scaledValue <= bottompluslimit))
    {
      stepperBot.setPlusLimit(scaledValue);
      stepperBot.setSpeed(speedVal);
      Serial.println("forward");

    }
    else if ((scaledValue < stepperBot.getCurrentPosition()) && (scaledValue >= bottomminuslimit))
    {
      stepperBot.setMinusLimit(scaledValue);
      stepperBot.setSpeed(-speedVal);
      Serial.println("backweard");

    }
  }

  if (control  >= 16 && control <= 34)
  {
    uint8_t HorSorV = (control - 16) % 3;
    uint8_t whichLight = (control - 16) / 3;
    Serial.println(whichLight);
    Serial.println(HorSorV);
    lights[whichLight][HorSorV] = value/127.0f;
    
    newDMXmessage(whichLight, lights[whichLight][0], lights[whichLight][1], lights[whichLight][2]);

  }

 

}

void topOpenLimitHit(void)
{
  if (stepperTop.getSpeed() < 0)
  {
    //stepperTop.stopAndSetHome();
    Serial.println(stepperTop.getSpeed());
    stepperTop.hardStop();
    stepperTop.setMinusLimit(stepperTop.getCurrentPosition());
    stepperTop.setPlusLimit(stepperTop.getCurrentPosition() + calibratedRange1);
    Serial.println("limit switch top open");
    toppluslimit = stepperTop.getPlusLimit();
    topminuslimit = stepperTop.getMinusLimit();
  
    Serial.println();
    Serial.println();
  }
}
void topClosedLimitHit(void)
{
  if (stepperTop.getSpeed() > 0)
  {
    Serial.println("lim switch top close");
    Serial.println(stepperTop.getPlusLimit());
    stepperTop.hardStop();
    stepperTop.setPlusLimit(stepperTop.getCurrentPosition());
    stepperTop.setMinusLimit(stepperTop.getCurrentPosition() - calibratedRange1);
    //scaledPlusLimit = stepperTop.getCurrentPosition() * 1024;
    //midiScaledPlusLimit = stepperTop.getCurrentPosition() * 127;
    toppluslimit = stepperTop.getPlusLimit();
    topminuslimit = stepperTop.getMinusLimit();
  }
}
void bottomOpenLimitHit(void)
{
  //Serial.println(stepperBot.getCurrentPosition());
  
  if (stepperBot.getSpeed() > 0)
  {
    Serial.println(stepperBot.getPlusLimit());
    stepperBot.setPlusLimit(stepperBot.getCurrentPosition());
    stepperBot.setMinusLimit(stepperBot.getCurrentPosition() - calibratedRange2);
    stepperBot.hardStop();
    Serial.println("lim switch bottom open");
    //scaledPlusLimit2 = stepperBot.getCurrentPosition() * 1024;
    //midiScaledPlusLimit2 = stepperBot.getCurrentPosition() * 127;
    bottompluslimit = stepperBot.getPlusLimit();
    bottomminuslimit = stepperBot.getMinusLimit();
  }

  
}
void bottomClosedLimitHit (void)
{
  if (stepperBot.getSpeed() < 0)
  {
    //stepperBot.stopAndSetHome();
    stepperBot.hardStop();
    Serial.println(stepperBot.getSpeed());
    stepperBot.setMinusLimit(stepperBot.getCurrentPosition());
    stepperBot.setPlusLimit(stepperBot.getCurrentPosition() + calibratedRange2);
    Serial.println("lim switch bottom close");
    bottompluslimit = stepperBot.getPlusLimit();
    bottomminuslimit = stepperBot.getMinusLimit();
  }
}
