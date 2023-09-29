// speedPlot.ino

// thie millisDelay library is available from https://www.forward.com.au/pfod/ArduinoProgramming/TimingDelaysInArduino.html
#include <millisDelay.h>
// the SpeedStepper library is available from https://www.forward.com.au/pfod/Robots/SpeedStepper/index.html
#include <SpeedStepper.h>
#include <limits.h>  // for max,min values
#include <pfodBufferedStream.h>



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

SpeedStepper stepperTop(STEP_PIN, DIR_PIN);
SpeedStepper stepperBot(STEP_PIN2, DIR_PIN2);

int distVal1 = 0;
int distVal2 = 0;

float speedVal = 50.f;
//const int limit_switches[4] = {/*topMotorlim1, bottomMotorlim1, topMotorlim2,  bottomMotorlim2*/0,1,2,0};
void setup() {


  pinMode(LIM_SWITCH_TOP_PIN1, INPUT_PULLDOWN);
  pinMode(LIM_SWITCH_TOP_PIN2, INPUT_PULLDOWN);
  pinMode(LIM_SWITCH_BOT_PIN1, INPUT_PULLDOWN);
  pinMode(LIM_SWITCH_BOT_PIN2, INPUT_PULLDOWN);
  pinMode(13, OUTPUT);
  delay(10);
  pinMode(OPEN_BUTTON, INPUT_PULLUP);
  pinMode(CLOSE_BUTTON, INPUT_PULLUP);
  pinMode(SPEED_POT, INPUT);
  pinMode(DIST_POT, INPUT);
  Serial.begin(9600);
  Serial.println("hello");
  usbMIDI.setHandleNoteOn(myNoteOn);
  usbMIDI.setHandleControlChange(OnControlChange);
  attachInterrupt(digitalPinToInterrupt(OPEN_BUTTON), openPressed, FALLING);
  attachInterrupt(digitalPinToInterrupt(CLOSE_BUTTON), closePressed, FALLING);
  digitalWrite(13, HIGH);
  bool findLimit = 0;
  stepperTop.setAcceleration(50000);
  stepperBot.setAcceleration(50000);
  stepperTop.setSpeed(50);
  stepperBot.setSpeed(-50);

  while (findLimit == 0)
  {
    stepperTop.run();  
    if (digitalRead(LIM_SWITCH_TOP_PIN1) == 1)
    {
      Serial.println("lim switch top close");
      stepperTop.setPlusLimit(stepperTop.getCurrentPosition());
      Serial.println(stepperTop.getPlusLimit());
      scaledPlusLimit = stepperTop.getCurrentPosition() * 1024;
      midiScaledPlusLimit = stepperTop.getCurrentPosition() * 127;
      findLimit++;
    }
  }
  findLimit = 0;
  while (findLimit == 0)
  {
    //stepperTop.run();
    stepperBot.run();
  if (digitalRead(LIM_SWITCH_BOT_PIN1) == 1)
    {
      stepperBot.stopAndSetHome();
      stepperBot.setMinusLimit(stepperBot.getCurrentPosition());
      Serial.println("lim switch bottom close");
      findLimit++;

    }
  }
  //stepperTop.goHome();
  Serial.println("MOVIN ON");
  stepperTop.setSpeed(-50);
  stepperBot.setSpeed(50);
  
  findLimit = 0;
  while (findLimit ==0)
  {
    stepperTop.run();
    //stepperBot.run();
    //Serial.println("TESTING");
    if (digitalRead(LIM_SWITCH_TOP_PIN2) == 1)
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
      if (digitalRead(LIM_SWITCH_BOT_PIN2) == 1)
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
  stepperTop.setSpeed(50);
  stepperBot.setSpeed(-50);
  digitalWrite(13, LOW);
}



void loop() {
  stepperTop.run();
  stepperBot.run();
  //Serial.println(stepperTop.getCurrentPosition());
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


void midiOpen()
{
  stepperTop.setSpeed(-speedVal);
  stepperBot.setSpeed(speedVal);
}

void midiClose()
{
  stepperTop.setSpeed(speedVal);
  stepperBot.setSpeed(-speedVal);
}

void myNoteOn(byte channel, byte note, byte velocity)
{
  if (note == 60)
  {
    midiOpen();
  }
  else if (note == 61)
  {
    midiClose();
  }
}


void OnControlChange(byte channel, byte control, byte value)
{
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
    stepperTop.setPlusLimit(midiScaledPlusLimit / value);
    stepperTop.setMinusLimit(midiScaledPlusLimit / (127 - value) );
    stepperBot.setPlusLimit(midiScaledPlusLimit / value);
    stepperBot.setMinusLimit(midiScaledPlusLimit / (127 - value) );
  }
}
