#include <Arduino.h>

#include "enigma_utils.h"
#include "enigma_cipher.h"

//object/structure creation
Adafruit_NeoPixel LED(NUM_LEDS, LED__PIN, NEO_GRB + NEO_KHZ800);
//insert ioexpander objects here

RotorHardware_t rotorConfig = {.numRotors= NUMROTORS, .sensorOffset = 9, .numSteps = 40,.enPin = MOTOR_EN_PIN, .dirPins={DIR0_PIN, DIR1_PIN, DIR2_PIN}, .directions={0,0,0}, .stepPins={STEP0_PIN,STEP1_PIN,STEP2_PIN}};

void setup() 
{
  #if SERIALDEBUG
    Serial.begin(115200);
  #endif

  //hardware config
  configRotorsPins(&rotorConfig);

}

void loop() 
{
  //local vars
  uint8_t ui8_CurrentState = STATE_STARTUP;
  //infinite loop
  while(1)
  {
    //Read inputs

    /* FSM */
    switch(ui8_CurrentState)
    {
      /************************STARTUP***********************
       * - System initialisation, self-tests
       * - Display cool HEIG animation
       * Exit condition: Init finished
      */
      case STATE_STARTUP:
      break;

      /************************STANDBY***********************
       * - Wait for user to insert the rotors
       * Exit condition: User closes one of the lids
      */
      default:
      case STATE_STANDBY:
      break;
      
      /************************SYSTEM_ID***********************
       * Executes system identification (rotors)
       * Exit condition: Lid open | ID finished OK
      */
      case STATE_SYSTEM_ID:
      break;

      /************************OPERATION***********************
       * Main machine operation
       * Reads keyboard, ciphers and displays to LEDS
       * Move rotors on each press
       * Exit condition: Lid open 
      */
      case STATE_OPERATION:
      break;

    }
    /* End of FSM*/
    
  }
}
