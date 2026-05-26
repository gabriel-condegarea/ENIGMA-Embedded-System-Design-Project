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
RotorHardware_t rotorConfig = {.numRotors= NUMROTORS, .sensorOffset = 22, 
                                .numSteps = 20,.enPin = MOTOR_EN_PIN, 
                                .dirPins={DIR2_PIN, DIR1_PIN, DIR0_PIN}, 
                                .directions={0,0,0}, 
                                .stepPins={STEP2_PIN,STEP1_PIN,STEP0_PIN}
                            };
uint32_t ui32_msCounter = 0;
States CurrentState = STATE_STARTUP;

//cipher vals
extern const char *alpha;
extern const char *reflectors[];
extern const char *rotor_names[];
extern char plugboard[];


void setup() 
{
  #if SERIALDEBUG
    Serial.begin(115200);
  #endif

  Keyboard.begin(); //USB-HID device

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
  else
  {
    #if SERIALDEBUG
      Serial.println("ENIGMA: Welcome!");
    #endif
  }

}

void loop() 
{
  //local vars

  //hardware
  uint8_t swiBig = 1, swiSmall = 1; //limit switches detection
  //Keypress detection
  int8_t letterIndex = -1, letterIndex_Old = -1;
  uint8_t newPress = 0;
  
  
  //cipher
  struct Enigma machine = {}; // initialized to defaults
  char character;

  uint16_t histPointer = 0;  //number of rotor moves
  uint8_t availBackspace = 0;  //number of subsequent backspaces available
  uint8_t posHistory[HIST_BUFSIZE][3] = {0};

  uint8_t r, i; //loop operators

  //rotor ID
  bool b_identValid = false;
  uint8_t ui8_identCounter = 0; //number of ident counter

  //start animation
  char letterSequence[7] = {"HEIGVD"};

  //timing
  uint32_t ui32_identStartTime = 0;
  States prevState = CurrentState;

  //infinite loop
  while(1)
  {
    //Read inputs (switches, keyboard)
    //Switches
    swiBig = ioex1.read(PCA95x5::Port::P16);
    swiSmall = ioex1.read(PCA95x5::Port::P17);

    //Keyboard press detection
    letterIndex = readKeyboard();
    //Serial.println(letterIndex);
    if(letterIndex != letterIndex_Old && (letterIndex != -1))
    {
      newPress = 1;
      Serial.println(alpha[letterIndex]);
    }else newPress = 0;

    letterIndex_Old = letterIndex;

    //Plugboard update
    scannerPlugboard();   //180 ms

    //check for state transition
    if(CurrentState != prevState && CurrentState != STATE_ERROR)
    {
      prevState = CurrentState; //update previous
      #if SERIALDEBUG
        Serial.print("Current state is now = ");
        Serial.println(CurrentState);
      #endif
    }
    


    /* FSM */
    switch(CurrentState)
    {
      /************************STARTUP***********************
       * - System initialisation, self-tests
       * - Display cool HEIG animation
       * Exit condition: Init finished
      */
      case STATE_STARTUP:
        //display HEIG text
        for(int i = 0; i<6; i++)
        {
          sendLED(letterSequence[i], &LED, 255,0,0);  //write letter
          delay(1000);
        }
        sendLED(-1, &LED, 255,0,0); //clear

        //init machine structure
        machine.reflector = reflectors[1];  //configure reflectors
        machine.numrotors = NUMROTORS;

        //exit logic:
        //both switches pressed -> directly to ID
        if(swiBig == 0 && swiSmall == 0)
        {
          CurrentState = STATE_SYSTEM_ID;
          ui32_identStartTime = ui32_msCounter+IDENT_DELAY;  //set delay
          digitalWrite(MOTOR_EN_PIN, 0);  //enable torque
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
        //disable torque 
        digitalWrite(MOTOR_EN_PIN, HIGH);
        //exit logic
        if(swiBig == 0 && swiSmall == 1)  CurrentState = STATE_POS_SEL;//only big lid closed
        else if(swiBig == 0 && swiSmall == 0)  //both switches pressed
        {
          CurrentState = STATE_SYSTEM_ID;
          ui32_identStartTime = ui32_msCounter+IDENT_DELAY; //delay - anormal order of operations
          digitalWrite(MOTOR_EN_PIN, 0);  //enable torque
        }
      break;

      /************************ POS_SEL ***********************
       * - Wait for user to insert the rotors
       * Exit condition: User closes the small lid
      */
      case STATE_POS_SEL:
        //disable torque 
        digitalWrite(MOTOR_EN_PIN, HIGH);
        //exit logic
        if(swiBig ==  1) CurrentState = STATE_ROTOR_SEL; //big lid opens
        else if(swiBig == 0 && swiSmall == 0)  //both switches pressed
        {
          CurrentState = STATE_SYSTEM_ID;
          ui32_identStartTime = ui32_msCounter;  //no delay - normal way to go
          digitalWrite(MOTOR_EN_PIN, 0);  //enable torque
        }
      break;
      
      /************************SYSTEM_ID***********************
       * Executes system identification (rotors)
       * Exit condition: Lid open | ID finished OK
      */
      case STATE_SYSTEM_ID:
        //digitalWrite(MOTOR_EN_PIN, 0);  //enable torque
        if(ui32_msCounter >= ui32_identStartTime && !b_identValid) 
        {
          b_identValid = rotorID(&rotorConfig, &adc);

          if(b_identValid)  //identification OK
          {
            histPointer = 0; //reset move counter
            //configure rotors 
            for(r = 0; r<rotorConfig.numRotors; r++)  //all rotors
            {
              machine.rotors[0] = new_rotor(&machine, rotorConfig.ident[r][0], 
                                                      rotorConfig.ident[r][1], 
                                                      rotorConfig.ident[r][2]);

              
              posHistory[0][r] = rotorConfig.ident[r][0];  //store initial position in buffer

              #if SERIALDEBUG
                Serial.println("Initial position: ");
                //Possible to add a way to print stellung
                printPosition(Serial, &machine);
                
              #endif
            } 
          }
          else ui8_identCounter++;

          //if ident fails twice
          if(ui8_identCounter <= 2) CurrentState = STATE_ERROR; 
        }

        //exit conditions
        if(swiBig == 1)
        {
          CurrentState = STATE_ROTOR_SEL; //big lid open
          ui8_identCounter = 0; //reset counter
        } 
        else if(swiSmall == 1)
        {
          CurrentState = STATE_POS_SEL;  //small lid open
          ui8_identCounter = 0; //reset counter
        } 
        else if(b_identValid)
        {
          CurrentState = STATE_OPERATION;  //Ident OK
          ui8_identCounter = 0; //reset counter
        } 
      break;

      /************************OPERATION***********************
       * Main machine operation
       * Reads keyboard, ciphers and displays to LEDS
       * Move rotors on each press
       * Exit condition: Lid open 
      */
      case STATE_OPERATION:
        //copy logic from Tests/Algorithmes
        
        if(newPress)  //new key press
        {
          newPress = 0; //rst flag
          switch(letterIndex)
          {
            case -1:  //error case
              CurrentState = STATE_ERROR; 
              #if SERIALDEBUG
                Serial.println("Invalid letter");
              #endif
              break;
            case 25: //backspace
                if(availBackspace > 0)
                {
                  availBackspace--;
                  histPointer--;

                  #if SERIALDEBUG
                    Serial.print(availBackspace);
                    Serial.println(" backspaces left");
                  #endif
                  //send to USB
                  Keyboard.write(8);
                  
                  for(r = 0; r<machine.numrotors; r++)  //move back one move
                  {
                    machine.rotors[r].offset = posHistory[histPointer][r];
                  }
                  moveAllRotors(&machine, &rotorConfig);  //physically move rotors
                }
                #if SERIALDEBUG
                  else Serial.println("Cannot delete more letters");
                #endif

              break;
            case 26: 
                //send to USB
                Keyboard.write(32); 
              break;
            default:  //normal letter
              character = alpha[letterIndex];
              #if SERIALDEBUG
                Serial.print("Input letter is ");
                Serial.println(character);
              #endif

              //rotors cycling
              cycleAllRotors(&machine);    
              moveAllRotors(&machine, &rotorConfig);  //physically move rotors

              histPointer++; //move one step in the history buffer 
              if(availBackspace < HIST_BUFSIZE) availBackspace++; //increment delete if not maxed out
              for(r = 0; r<machine.numrotors;r++)
              {
                posHistory[histPointer % HIST_BUFSIZE][r] = machine.rotors[r].offset;  //store history
              } 

              #if SERIALDEBUG
                Serial.println("Position after cycling:");
                printPosition(Serial, &machine);
              #endif

              character = enigma_encrypt(&machine, character);

              #if SERIALDEBUG
                Serial.print("Output letter is ");
                Serial.println(character);
              #endif

              //send via USB
              Keyboard.write(character);
              break;
          }
        }
        //exit conditions
        if(swiBig == 1) CurrentState = STATE_ROTOR_SEL; //big lid open
        else if(swiSmall == 1) CurrentState = STATE_POS_SEL;  //small lid open
      break;

      /************************ ERROR ***********************
       * Error state, stuck here :/
      */
      case STATE_ERROR:
        sendLED('E', &LED, 255,0,0);  //RED E
        #if SERIALDEBUG
          Serial.println("Engima has encountered an error :(");
          Serial.print("State at the time of error: ");
          Serial.println(prevState);
        #endif
        delay(1000);
        //exit conditions
        //None
      break;      
    }
    /* End of FSM*/
    
  }
}
