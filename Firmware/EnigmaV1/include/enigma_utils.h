#pragma once
#include <Arduino.h>
#include <Wire.h>

#include <Adafruit_NeoPixel.h>
#include <Adafruit_MCP3008.h>
#include <PCA95x5.h>

//Keyboard function
#include <Keyboard.h>

//timers
#include <RPi_Pico_TimerInterrupt.h>
#include <RPi_Pico_ISR_Timer.hpp>


#include "enigma_cipher.h"
#include "PlugboardScanner.h"

/* Constants and configuration */
//Config
/* Parameters */
#define MAG_THR 15 //threshold for magnet detection
#define IDENT_DELAY 3000 //delay for anormal ident order
#define SERIALDEBUG 1

//State machine states
typedef enum States {STATE_STARTUP, STATE_ROTOR_SEL, STATE_POS_SEL, STATE_SYSTEM_ID, STATE_OPERATION, STATE_ERROR}States;


/* Constants */
//Keyboard 
#define NUM_ROWS 6
#define NUM_COLS 5
#define NUM_KEYS (NUM_ROWS * NUM_COLS)
#define NUM_LEDS 26

//Rotor ID
#define NUMROTORS 3
#define ALLROTORS for(r = 0; r < rcfg->numRotors; r++)


/* Structures */
typedef struct RotorHardware_t
{
  uint8_t numRotors;  //number of rotors installed
  uint8_t sensorOffset; //offset from sensor to displayed lettre
  uint8_t numSteps;   //number of steps for a letter rotation
  uint8_t enPin;        //general enable pin
  uint8_t dirPins[3];   //direction pins
  uint8_t directions[3];//turning direction
  uint8_t stepPins[3];  //step pins
  uint8_t ident[3][3];  //{rotor#, offset, stellung}
}RotorHardware_t;

/* Function prototypes */
//Keyboard / LED
int8_t readKeyboard(void);
void sendLED(uint8_t index, Adafruit_NeoPixel* leds, uint8_t r, uint8_t g, uint8_t b);
void configKeyboardPins(void);
bool initLEDS(Adafruit_NeoPixel* leds);

//plugboard
bool configIOEX(TwoWire* Wire, PCA9555* io0, PCA9555* io1);

//Rotors
bool configRotorsPins(RotorHardware_t* rcfg);
bool rotorID(RotorHardware_t* rcfg, Adafruit_MCP3008* adc);
bool moveAllRotors(struct Enigma *machine, RotorHardware_t* rcfg);

bool timerHandlerMillis(struct repeating_timer *t);

void printPosition(SerialUSB Serial, struct Enigma* machine);


/* Hardware definitions */
// Matrix rows
#define ROW0_PIN        1
#define ROW1_PIN        3
#define ROW2_PIN        28
#define ROW3_PIN        19
#define ROW4_PIN        26
#define ROW5_PIN        21

// Matrix columns
#define COL0_PIN        20
#define COL1_PIN        22
#define COL2_PIN        27
#define COL3_PIN        2
#define COL4_PIN        0

// Stepper motors
#define STEP0_PIN       4
#define DIR0_PIN        5

#define STEP1_PIN       7
#define DIR1_PIN        8

#define STEP2_PIN       9
#define DIR2_PIN        13

#define MOTOR_EN_PIN    6

// LED / misc
#define LED__PIN      18

// SPI
#define SPI_SCK_PIN     10
#define SPI_MOSI_PIN    11
#define SPI_MISO_PIN    12
#define ADC_CS_PIN      16

// I2C
#define I2C_SDA_PIN     14
#define I2C_SCL_PIN     15


