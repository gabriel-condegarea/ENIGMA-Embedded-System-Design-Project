#include <Arduino.h>

#include "enigma_utils.h"
#include "enigma_cipher.h"

//object/structure creation
Adafruit_NeoPixel LED(NUM_LEDS, LED__PIN, NEO_GRB + NEO_KHZ800);
Adafruit_MCP3008 adc;
PCA9555 ioex0; // Adresse I2C 0x20 (K7 - Port 0: Q A Y W E D C S X)
PCA9555 ioex1; // Adresse I2C 0x21 (K8 - Port 1: Z U J I K O L P M N H G F B T R V)

//Timer
RPI_PICO_Timer ITimer(0);

//global vars
RotorHardware_t rotorConfig = {.numRotors= NUMROTORS, .sensorOffset = 22, .numSteps = 40,.enPin = MOTOR_EN_PIN, .dirPins={DIR0_PIN, DIR1_PIN, DIR2_PIN}, .directions={0,0,0}, .stepPins={STEP0_PIN,STEP1_PIN,STEP2_PIN}};
uint32_t ui32_msCounter = 0;
States CurrentState = STATE_STARTUP;

//cipher vals
extern const char *alpha;
extern const char *reflectors[];
extern const char *rotor_names[];


void setup() 
{
  #if SERIALDEBUG
    Serial.begin(115200);
  #endif

  bool status = true;

  //hardware config
  configKeyboardPins();
  if(!initLEDS(&LED))
  {
    status = false;
    #if SERIALDEBUG
      Serial.println("Error initalising WS2812B");
    #endif
  }

  configRotorsPins(&rotorConfig);

  if(!adc.begin(SPI_SCK_PIN,SPI_MOSI_PIN,SPI_MISO_PIN,ADC_CS_PIN)) //initialise ADC
  {
    status = false;
    #if SERIALDEBUG
      Serial.println("Error initalising MCP3008");
    #endif
  }


  if(!configIOEX(&Wire1, &ioex0, &ioex1))
  {
    status = false;
    #if SERIALDEBUG
      Serial.println("Error initalising PC9555");
    #endif
  }
  initPlugboardScanner(); //init plugboard

  //Timer
  if(!ITimer.attachInterruptInterval(1000, timerHandlerMillis))
  {
    status = false;
    #if SERIALDEBUG
      Serial.println("Error starting timer");
    #endif
  }

  if(!status)
  {
    #if SERIALDEBUG
      Serial.println("Enigma has reached an error state, please reset the system");
    #endif
    CurrentState = STATE_ERROR;
  }

}

void loop() 
{
  //local vars
  //cipher
  struct Enigma machine = {}; // initialized to defaults
  int8_t i, character, index;
  uint8_t r;

  uint8_t swiBig = 1, swiSmall = 1; //limit switches detection

  //Keypress detection
  int8_t letterIndex = -1, letterIndex_Old = -1;
  uint8_t newPress = 0;

  bool b_identValid = false;

  //timing
  uint32_t ui32_identStartTime = 0;

  //infinite loop
  while(1)
  {
    //Read inputs (switches, keyboard)
    //Switches
    swiBig = ioex1.read(PCA95x5::Port::P16);
    swiSmall = ioex1.read(PCA95x5::Port::P17);

    //Keyboard press detection
    letterIndex = readKeyboard();
    if(letterIndex != letterIndex_Old)
    {
      newPress = 1;
    }else newPress = 0;

    letterIndex_Old = letterIndex;

    //Plugboard update
    scannerPlugboard();


    /* FSM */
    switch(CurrentState)
    {
      /************************STARTUP***********************
       * - System initialisation, self-tests
       * - Display cool HEIG animation
       * Exit condition: Init finished
      */
      case STATE_STARTUP:
        //display HEIG
        for(int i = 0; i<4; i++)
        {
          sendLED(1)
        }

        //create and init machine structure
        machine.reflector = reflectors[1];  //configure reflectors

        //exit logic:
        //both switches pressed -> directly to ID
        if(swiBig == 0 && swiSmall == 0)
        {
          CurrentState = STATE_SYSTEM_ID;
          ui32_identStartTime = ui32_msCounter+IDENT_DELAY;  //set delay
        }
        else if(swiBig == 0 && swiSmall == 1) CurrentState = STATE_POS_SEL; //big one pressed, small not
        else if(swiBig == 1 && swiSmall == 1) CurrentState = STATE_ROTOR_SEL; //none pressed

      break;

      /************************ ROTOR_SEL ***********************
       * - Wait for user to insert the rotors
       * Exit condition: User closes the big lid
      */
      default:
      case STATE_ROTOR_SEL:
        
        //exit logic
        if(swiBig == 0 && swiSmall == 1)  CurrentState = STATE_POS_SEL;//only big lid closed
        else if(swiBig == 0 && swiSmall == 0)  //both switches pressed
        {
          CurrentState = STATE_SYSTEM_ID;
          ui32_identStartTime = ui32_msCounter+IDENT_DELAY; //delay - anormal order of operations
        }
      break;

      /************************ POS_SEL ***********************
       * - Wait for user to insert the rotors
       * Exit condition: User closes the small lid
      */
      case STATE_POS_SEL:
        
        //exit logic
        if(swiBig ==  1) CurrentState = STATE_ROTOR_SEL; //big lid opens
        else if(swiBig == 0 && swiSmall == 0)  //both switches pressed
        {
          CurrentState = STATE_SYSTEM_ID;
          ui32_identStartTime = ui32_msCounter;  //no delay - normal way to go
        }
      break;
      
      /************************SYSTEM_ID***********************
       * Executes system identification (rotors)
       * Exit condition: Lid open | ID finished OK
      */
      case STATE_SYSTEM_ID:

        if(ui32_msCounter >= ui32_identStartTime && !b_identValid) 
        {
          b_identValid = rotorID(&rotorConfig, &adc);

          if(b_identValid)  //identification OK
          {
            
            for(r = 0; r<rotorConfig.numRotors; r++)  //all rotors
            {
              machine.rotors[0] = new_rotor(&machine, rotorConfig.ident[r][0], 
                                                      rotorConfig.ident[r][1], 
                                                      rotorConfig.ident[r][2]);
            }

          }
        }

        //exit conditions
        if(swiBig == 1) CurrentState = STATE_ROTOR_SEL; //big lid open
        else if(swiSmall == 1) CurrentState = STATE_POS_SEL;  //small lid open

      break;

      /************************OPERATION***********************
       * Main machine operation
       * Reads keyboard, ciphers and displays to LEDS
       * Move rotors on each press
       * Exit condition: Lid open 
      */
      case STATE_OPERATION:
        //copy logic from Tests/Algorithmes

        //exit conditions
        if(swiBig == 1) CurrentState = STATE_ROTOR_SEL; //big lid open
        else if(swiSmall == 1) CurrentState = STATE_POS_SEL;  //small lid open

      break;

      /************************ ERROR ***********************
       * Error state, stuck here :/
      */
      case STATE_ERROR:
        sendLED(5, &LED, 255,0,0);  //RED E
        //exit conditions
        //None
      break;      

    }
    /* End of FSM*/
    
  }
}
