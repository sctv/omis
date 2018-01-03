#include "Pump.h"
#include "CommandParser.h"

#define NPUMPS 2

/* PUMP 0 */
#define P01 12
#define P02 11
#define P03 10
#define P04 9
#define EN0 13
#define MA0 15
#define TPM0 0.787
#define LVR0 14.04 // 5mm glass syringe, 3mm plastic is 13.
#define STEPS0 513

/* PUMP 1 */
#define P11 8
#define P12 7
#define P13 6
#define P14 5
#define EN1 4
#define MA1 15
#define TPM1 0.787
#define LVR1 14.04
#define STEPS1 513

#define VERSION 0.32

// Reserve digital pins 2/3 for interrupts and A0/A1 for LED outputs
// See https://www.arduino.cc/en/Reference/AttachInterrupt
const byte BUTTON1 = 2; // why not #define?
volatile byte button1State = LOW;
const byte REDLED = A0;
const byte GREENLED = A1;

// Reserve analog pin A5 for sensor (possibly more in the future)
const byte SENSOR0 = A5;

CommandParser cp;

Pump pump[] { 
  Pump(STEPS0, P01, P02, P03, P04, EN0, MA0, TPM0, LVR0),
  Pump(STEPS1, P11, P12, P13, P14, EN1, MA1, TPM1, LVR1)
};

/*
 * TODO: Add support for user interaction (e.g. trigger button)
 */
// Interrupt Service Routine (ISR)

void button1Pressed ()
{
  button1State = !button1State;
}  
/*
 * void button2Pressed ()
{
  button2State != button2State;
} 
*/

void setup() {
  // Set up serial communications
  // TODO go faster with serial
  Serial.begin(115200);
  while(!Serial) {
    ; //Wait for connect
  }
  Serial.println("Syringe pump communications established");
  Serial.println("This is version " + String(VERSION) + ".");


  // Setup hardware interface components
  pinMode (REDLED, OUTPUT);  
  pinMode (GREENLED, OUTPUT);  
  pinMode (BUTTON1, INPUT_PULLUP); // internal pull-up resistor
 
  attachInterrupt (digitalPinToInterrupt(BUTTON1), button1Pressed , CHANGE);  // attach interrupt handler 
 
  // Set the initial pump as pump(0)
  cp.whichPump = 0;
  Serial.println("Controlling pump #0");

  // Set an initial speed
  for (int i = 0; i < NPUMPS; i++ ) {
    pump[i].setSpeed(25);
  }

  // Set sensor pin(s)
  pinMode (SENSOR0, INPUT);

}

void loop() {
  // the validate() function of the CommandParser checks to see if a command is
  //   ready for parsing and that the user input follows the basic structure of a command.
  if(cp.validate()){
    cp.parse();
    // "sp" is a global command to switch operations to a different pump
    if(cp.operation == "sp") {
      if ( (cp.argument.toInt() < 0) || (cp.argument.toInt() >= NPUMPS) ) {
        Serial.println("Error! Default to pump #0");
        cp.whichPump = 0;
      }
      else {
        cp.whichPump = cp.argument.toInt();
        Serial.println("Controlling pump #" + cp.argument);
      }
    }
    // "re" is a global command to read from the sensor pin(s)
    else if(cp.operation == "re") {
      Serial.println(analogRead(SENSOR0));
    }
    else {
      pump[cp.whichPump].operate(cp.operation, cp.argument);

    }
    cp.refresh();
    
  }
  // Always check if a step should be made
  for (int i = 0; i < NPUMPS; i++ ) {
    pump[i].tryStep();
  }

  digitalWrite(GREENLED, !button1State);
}

// Using serial events for capturing user requests
void serialEvent() {
  cp.read();
}

