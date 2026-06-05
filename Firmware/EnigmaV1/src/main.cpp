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
                                .numSteps = 40, .enPin = MOTOR_EN_PIN, 
                                .dirPins={DIR2_PIN, DIR1_PIN, DIR0_PIN}, 
                                .directions={0,1,1}, 
                                .stepPins={STEP2_PIN,STEP1_PIN,STEP0_PIN}
                            };
uint32_t ui32_msCounter = 0;
States CurrentState = STATE_STARTUP;

uint8_t opt_debug = VERBOSE;

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

  #if USBKEYBOARD
    Keyboard.begin(); //USB-HID device
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
  bool b_firstLetterPressed = false;
  uint8_t ui8_identCounter = 0; //number of ident counter

  //start animation
  char letterSequence[] = {"HEIGVD"};//{"QWERTZUIOPLKJHGFDSAYXCVBNM"};
  uint16_t colorStep;
  uint8_t colour[3];

  //timing
  uint32_t ui32_identStartTime = 0, ui32_lastIdent = 0;
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
      //Serial.println(letterIndex);
      if(!b_firstLetterPressed) b_firstLetterPressed = true;
    }
    else newPress = 0; 

    if(b_firstLetterPressed)  //after the first press
    {
      if(letterIndex == -1) sendLED(-1, &LED, 1,0,0,0);  //clear display if no press
    }
    

    letterIndex_Old = letterIndex;

    //Plugboard update
    scannerPlugboard();   //180 ms
    //Serial.println(plugboard);

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
        colorStep = 767/sizeof(letterSequence);
        for(int i = 0; i<sizeof(letterSequence)-1; i++)
        {
          fade(colorStep*i, 255, &LED, colour);
          //sendLED(letterSequence[i], &LED,1, colour[0],colour[1],colour[2]);  //write letter
          sendLED(letterSequence[i], &LED,1, 255,0,0);  //write letter
          delay(500);
        }
        sendLED(-1, &LED, 1, 255,0,0); //clear

        //init machine structure
        machine.reflector = reflectors[1];  //configure reflectors
        machine.numrotors = NUMROTORS;

        //exit logic:
        //both switches pressed -> directly to ID
        if(swiBig == 0 && swiSmall == 0)
        {
          CurrentState = STATE_SYSTEM_ID;
          ui32_identStartTime = ui32_msCounter+IDENT_DELAY;  //set delay
          __enableTorque;
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
        __disableTorque;
        //exit logic
        if(swiBig == 0 && swiSmall == 1)  CurrentState = STATE_POS_SEL;//only big lid closed
        else if(swiBig == 0 && swiSmall == 0)  //both switches pressed
        {
          CurrentState = STATE_SYSTEM_ID;
          ui32_identStartTime = ui32_msCounter+IDENT_DELAY; //delay - anormal order of operations
          __enableTorque;
        }
      break;

      /************************ POS_SEL ***********************
       * - Wait for user to insert the rotors
       * Exit condition: User closes the small lid
      */
      case STATE_POS_SEL:
        //disable torque 
        __disableTorque;
        //exit logic
        if(swiBig ==  1) CurrentState = STATE_ROTOR_SEL; //big lid opens
        else if(swiBig == 0 && swiSmall == 0)  //both switches pressed
        {
          CurrentState = STATE_SYSTEM_ID;
          ui32_identStartTime = ui32_msCounter; 
          __enableTorque;
        }
      break;
      
      /************************SYSTEM_ID***********************
       * Executes system identification (rotors)
       * Exit condition: Lid open | ID finished OK
      */
      case STATE_SYSTEM_ID:
        //__enableTorque;  //enable torque
        if((ui32_msCounter >= ui32_identStartTime) 
            && (!b_identValid) 
            && (ui32_msCounter >= (ui32_lastIdent+IDENT_MINTIME))) 
        {
          b_identValid = rotorID(&rotorConfig, &adc);

          Serial.println("ident done");
          ui32_lastIdent = ui32_msCounter;

          // if(!b_identValid)
          // {
          //   //manual ident override
          //   rotorConfig.ident[0][0] = 3;  //rotors 423
          //   rotorConfig.ident[1][0] = 2;
          //   rotorConfig.ident[2][0] = 4;

          //   rotorConfig.ident[0][1] = 3;  //start GVD
          //   rotorConfig.ident[1][1] = 21;
          //   rotorConfig.ident[2][1] = 6;

          //   rotorConfig.ident[0][2] = 7;  //stellung AAA
          //   rotorConfig.ident[1][2] = 4;
          //   rotorConfig.ident[2][2] = 8;
          // }

          // b_identValid = true;

          if(b_identValid)  //identification OK
          {
            histPointer = 0; //reset move counter
            availBackspace = 0;

            //send OK to LEDS
            b_firstLetterPressed = false;
            sendLED('O', &LED, 1, 0, 255, 0);
            sendLED('K', &LED, 0, 0, 255, 0);
            //configure rotors 
            for(r = 0; r<rotorConfig.numRotors; r++)  //all rotors
            {
              machine.rotors[r] = new_rotor(&machine, rotorConfig.ident[r][0], 
                                                      rotorConfig.ident[r][1], 
                                                      rotorConfig.ident[r][2]);              
              posHistory[0][r] = rotorConfig.ident[r][1];  //store initial position in buffer
            } 

            #if SERIALDEBUG
              Serial.println("Initial position: ");
              //Possible to add a way to print stellung
              printPosition(Serial, &machine);
              
            #endif
          }
          else
          {
            Serial.println("Ident failed");
            ui8_identCounter++;
          } 

          //if ident fails twice
          if(ui8_identCounter == 2) CurrentState = STATE_ERROR; 
        }

        //exit conditions
        if(swiBig == 1) //big lid open
        {
          CurrentState = STATE_ROTOR_SEL; 
          ui8_identCounter = 0; //reset counter
          b_identValid = false;
        } 
        else if(swiSmall == 1) //small lid open
        {
          CurrentState = STATE_POS_SEL;  
          ui8_identCounter = 0; //reset counter
          b_identValid = false;
        } 
        else if(b_identValid)
        {
          // sendLED('O', &LED, 1, 0, 255, 0);
          // sendLED('K', &LED, 0, 0, 255, 0);
          CurrentState = STATE_OPERATION;  //Ident OK
          ui8_identCounter = 0; //reset counter
          b_identValid = false; // for next time
          __disableTorque;  //power-saving
        } 
      break;

      /************************OPERATION***********************
       * Main machine operation
       * Reads keyboard, ciphers and displays to LEDS
       * Move rotors on each press
       * Exit condition: Lid open 
      */
      case STATE_OPERATION:
      
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
            case 26: //backspace  
                if(availBackspace > 0)
                {
                  availBackspace--;
                  histPointer--;
                  
                  // debug: print history buffer
                  // Serial.print("History buffer: ptr: ");
                  // Serial.println(histPointer%HIST_BUFSIZE);
                  // for(i = 0; i<HIST_BUFSIZE; i++)
                  // {
                  //   Serial.print(i);
                  //   Serial.print(": ");
                  //   Serial.print(posHistory[i][2]);
                  //   Serial.print("|");
                  //   Serial.print(posHistory[i][1]);
                  //   Serial.print("|");
                  //   Serial.println(posHistory[i][0]);
                  // }
                  
                  #if SERIALDEBUG
                    Serial.println("Reading history from: ");
                    Serial.println(histPointer%HIST_BUFSIZE);
                    Serial.print(availBackspace);
                    Serial.println(" backspaces left");
                  #endif
                  //send to USB
                  #if USBKEYBOARD
                    Keyboard.write(8);
                  #endif
                  
                  for(r = 0; r<machine.numrotors; r++)  //move back one move
                  {
                    machine.rotors[r].offset = posHistory[histPointer%HIST_BUFSIZE][r];
                  }
                  __enableTorque;
                  moveAllRotors(&machine, &rotorConfig);  //physically move rotors
                  __disableTorque;  //disable torque to conserve power and cool motors

                  printPosition(Serial, &machine);
                }
                #if SERIALDEBUG
                  else Serial.println("Cannot delete more letters");
                #endif

              break;
            case 27:  //space 
                //send to USB
                #if USBKEYBOARD
                  Keyboard.write(32); 
                #endif
              break;
            default:  //normal letter
              character = alpha[letterIndex];
              #if SERIALDEBUG
                Serial.print("Input letter is ");
                Serial.println(character);
              #endif

              //software rotors cycling
              cycleAllRotors(&machine);    

              //History buffer
              histPointer++; //move one step in the history buffer 
              if(availBackspace < (HIST_BUFSIZE-1)) availBackspace++; //increment delete if not maxed out
              for(r = 0; r<machine.numrotors;r++)
              {
                posHistory[histPointer % HIST_BUFSIZE][r] = machine.rotors[r].offset;  //store history
              } 



              #if SERIALDEBUG
                Serial.println("Position after cycling:");
                printPosition(Serial, &machine);
              #endif

              //encrypt character
              character = enigma_encrypt(&machine, character);
              sendLED(character, &LED, 1, 252, 186, 3);
              
              #if SERIALDEBUG
                Serial.print("Output letter is ");
                Serial.println(character);
              #endif

              __enableTorque;
              moveAllRotors(&machine, &rotorConfig);  //physically move rotors
              __disableTorque;

              //send via USB
              #if USBKEYBOARD
                //Swap y-z
                if(character == 'Z') character = 'Y';//z
                else if (character == 'Y') character = 'Z'; //y 
                Keyboard.write(character);
              #endif 

              Serial.print(availBackspace);
              Serial.println(" backspaces left");
              
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
        sendLED('E', &LED, 1, 255,0,0);  //RED E
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
