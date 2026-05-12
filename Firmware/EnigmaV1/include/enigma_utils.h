#pragma once
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

/* Constants and configuration */
//Config
#define SERIALDEBUG 1

//State machine states
#define STATE_STARTUP 0
#define STATE_STANDBY 1
#define STATE_SYSTEM_ID 2
#define STATE_OPERATION 3


//Sensor
#define MAG_THR 15 //threshold for magnet detection

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
int8_t readKeyboard(void);
void sendLED(uint8_t index, Adafruit_NeoPixel* leds);
bool configRotorsPins(RotorHardware_t* rcfg);
bool rotorID(RotorHardware_t* rcfg);
















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


