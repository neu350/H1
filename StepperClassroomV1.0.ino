#include <AccelStepper.h>
#include <CmdMessenger.h>

#define MS1 10 // 4 --> 10
#define MS2 11 // 5 --> 11
#define MS3 12 // 6 --> 12
#define DIR 8 // 2 --> 8
#define STP 9 // 1 --> 9
#define ENA 13 // 7 --> 13

// Define a stepper and the pins it will use
AccelStepper stepper(1, 8, 9);

// Define global variables
int pos = 1600; // May not be needed for current modes
float spd = 363.64; // default to 8.8sec/360deg
float accel = 6000; // default to very high number, almost no acceleration. Not used with runSpeed() or current modes in v1.0
float dire = 1; // default direction to be counter-clockwise. dire can be set to 1 or -1 to change direction by commands
float dura = 1000; // default duration of 1000 milliseconds
int prot = 1; // not currently used, planned to use it for protocols, may have future use
int mode = 1; // used to select between modes of operation, default to back/forth movement
unsigned long millistime = 0; // used for timing
unsigned long lasttime = 0; // used for timing
bool ticktock = true; // used for alternating functions

// Initialize CommandMessenger to handle commands over serial port
CmdMessenger cmdMessenger = CmdMessenger(Serial);

// Set the commands CommandMessenger will accept, order matters
enum
{
  kSetSpeed,
  kSetAccel,
  kSetDir,
  kSetDur,
  kRun,
  kStop,
  kRunProt,
  kStatus,
};

// Attach functions to each of the CommandMessenger commands
void attachCommandCallbacks()
{
  cmdMessenger.attach(kSetSpeed, OnSetSpeed);
  cmdMessenger.attach(kSetAccel, OnSetAccel);
  cmdMessenger.attach(kSetDir, OnSetDir);
  cmdMessenger.attach(kSetDur, OnSetDur);
  cmdMessenger.attach(kRun, OnRun);
  cmdMessenger.attach(kStop, OnStop);
  cmdMessenger.attach(kRunProt, OnRunProt);
}

// Command to run, to go, to start movement
void OnRun()
{
  cmdMessenger.sendCmd(kStatus,"Starting");
  stepper.enableOutputs();
}

// Command to set the current mode, 1, 2, or 3 as of v1.0
void OnRunProt()
{
  cmdMessenger.sendCmd(kStatus,"Running in Protocol Mode");
  prot = cmdMessenger.readInt16Arg();
  mode = prot;
}

// Command to stop. Note that if you're running mode 3, it switches to mode 1 and stops, since mode 3 can not be stopped by this alone
void OnStop()
{
  cmdMessenger.sendCmd(kStatus,"Stopping");
  mode = 1;
  stepper.disableOutputs();
}

// Command to set speed. Reasonable values are between 100-2000. 
void OnSetSpeed()
{
  stepper.disableOutputs();
  cmdMessenger.sendCmd(kStatus,"Setting Speed");
  spd = cmdMessenger.readFloatArg();
  stepper.setSpeed(spd);
  stepper.setMaxSpeed(spd);
}

// Command to set acceleration, not currently used in v1.0
void OnSetAccel()
{
  stepper.disableOutputs();
  cmdMessenger.sendCmd(kStatus,"Setting Acceleration");
  accel = cmdMessenger.readFloatArg();
  stepper.setAcceleration(accel);
}

// Command to set direction, can accept 1 or -1 as input
void OnSetDir()
{
  stepper.disableOutputs();
  cmdMessenger.sendCmd(kStatus,"Setting Direction");
  dire = cmdMessenger.readInt16Arg();
  stepper.setSpeed(dire*spd); 
}

// Command to set duration for modes 1 and 3, accepts input as interger milliseconds
void OnSetDur()
{
  stepper.disableOutputs();
  cmdMessenger.sendCmd(kStatus,"Setting Duration");
  dura = cmdMessenger.readFloatArg();
}

void setup()
{  
  Serial.begin(115200); // start serial communication
  cmdMessenger.printLfCr();
  attachCommandCallbacks(); // attach CommandMessenger functions to commands
  
  
  pinMode(MS1, OUTPUT); //  -\
  pinMode(MS2, OUTPUT); //    > required for BigEasyDriver, sets step mode
  pinMode(MS3, OUTPUT); //  -/
  pinMode(STP, OUTPUT); // required for BigEasyDriver
  pinMode(DIR, OUTPUT); // required for BigEasyDriver
  pinMode(ENA, OUTPUT); // required for BigEasyDriver

  stepper.setPinsInverted(false, false, true);
  stepper.setEnablePin(ENA);
  stepper.setMaxSpeed(spd); // Set default speed, but not used in v1.0
  stepper.setSpeed(spd); // Set default speed
  stepper.setAcceleration(accel); // Set default acceleration, but not used in v1.0

  digitalWrite(MS1, HIGH); //Pull MS1,MS2, and MS3 to high/high/low to set logic to 1/8th microstep resolution
  digitalWrite(MS2, HIGH);
  digitalWrite(MS3, LOW);
}

void loop()
{
  cmdMessenger.feedinSerialData(); // grab newest command

  switch (mode) // which mode are we in?
  {
    // Mode 1 moves back/forth at duration set by dura, speed set by spd
    case 1:
      millistime = millis();
      if ((millistime - lasttime) >= dura)
      {
        spd = -spd;
        stepper.setSpeed(spd);
        lasttime = millistime;
      }      
    break;

    // Mode 2 rotates continuously in direction set by dire, at speed set by spd
    case 2:
      // do nothing, just rotate
    break;

    // Mode 3 rotates in direction set by dire, for duration set by dura, at speed set by spd, then pauses for duration set by dura, before moving again in the same direction... etc. 
    case 3:
      millistime = millis();
      if ((millistime - lasttime) >= dura)
      {
        if (ticktock == true)
        {
          stepper.disableOutputs();
          ticktock = false;
        }
        else if (ticktock == false)
        {
          stepper.enableOutputs();
          ticktock = true;
        }
        lasttime = millistime;
      }
    break;
  }

  // Required to tell the stepper motor to continue motion each iteration of the loop
  stepper.runSpeed();

}
