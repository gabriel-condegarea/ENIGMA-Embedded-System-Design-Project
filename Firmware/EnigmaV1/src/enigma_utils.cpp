#include "enigma_utils.h"

extern uint32_t ui32_msCounter;

/* Keyboard Utilities */
const uint8_t row_pins[NUM_ROWS] = {
    ROW0_PIN, ROW1_PIN, ROW2_PIN, ROW3_PIN, ROW4_PIN, ROW5_PIN};

const uint8_t col_pins[NUM_COLS] = {
    COL0_PIN, COL1_PIN, COL2_PIN, COL3_PIN, COL4_PIN};


//convert key pressed to letter
const int8_t key_to_alpha[NUM_KEYS] =
{
    /* ROW0: Q W E R T */ 16, 22, 4, 17, 19,
    /* ROW1: A S D F G */ 0, 18, 3, 5, 6,
    /* ROW2: Y X C V B */ 24, 23, 2, 21, 1,
    /* ROW3: Z U I O P */ 25, 20, 8, 14, 15,
    /* ROW4: H J K L invalid */ 7, 9, 10, 11, -1,
    /* ROW5: N M backspace space invalid */ 13, 12, 26, 27, -1};

//convert lettre to LED index
const uint8_t alpha_to_LED[NUM_LEDS] = {18,23,21,16,2,15,14,13,7,12,11,10,25,24,8,9,0,3,17,4,6,22,1,20,19,5};

/* ───────────────────────────────────────────────
 * FUNCTION: readKeyboard
 *
 * Scans the keyboard matrix once.
 * Returns:
 * -1 if no valid letter key is pressed
 * alphabet index 0..25 if a valid letter key is pressed
 * 26, 27 for backspace and space
 * ─────────────────────────────────────────────── */
int8_t readKeyboard(void)
{
  uint8_t read = 0;
  for (uint8_t col = 0; col < NUM_COLS; col++)  //each collumn
  {

    /* Activate current row */
    digitalWrite(col_pins[col], 1);
    // Serial.print("col ");
    // Serial.println(col);
    // delayMicroseconds(10);

    for (uint8_t row = 0; row < NUM_ROWS; row++)  
    { 
      read = digitalRead(row_pins[row]);  //read value
      if (read == 1)    //if pressed
      {
        /* Release current row before returning */
        digitalWrite(col_pins[col], 0);

        uint8_t key_index = row * NUM_COLS + col;
        return key_to_alpha[key_index];
      }
    }

    /* Release current row */
    digitalWrite(col_pins[col], 0);
  }

  return -1;
}

/* ───────────────────────────────────────────────
 * FUNCTION: sendLED
 *
 * Turns off all LEDs, then lights the LED corresponding
 * to the given alphabet index.
 *
 * Example:
 *   sendLED(0)  -> lights A
 *   sendLED(1)  -> lights B
 *   sendLED(25) -> lights Z
 * ─────────────────────────────────────────────── */

void sendLED(int8_t letter, Adafruit_NeoPixel *leds, uint8_t r, uint8_t g, uint8_t b)
{
  uint8_t index = 0;
  leds->clear();

  if(letter == -1)
  {
    leds->show();
    return;   //-1 clears display
  } 

  if((letter>= 'A') && (letter <= 'Z')) //support alpha input
  {
    letter -= 'A';  //A maps to 0
  }
  else if (letter > 26) return; //invalid letter

  index = alpha_to_LED[letter];

  if (index < NUM_LEDS) leds->setPixelColor(index, leds->Color(r, g, b));

  leds->show();
}

/* ───────────────────────────────────────────────
 * FUNCTION: configKeyboardPins
 *
 * Configures the pins used for keyboard operation
 * ─────────────────────────────────────────────── */
void configKeyboardPins(void)
{
  uint8_t col = 0, row = 0;

  /* Columns as outputs */
  for (col = 0; col < NUM_COLS; col++)
  {
    pinMode(col_pins[col], OUTPUT);
  }

  /* Rows as input */
  for (row = 0; row < NUM_ROWS; row++)
  {
    pinMode(row_pins[row], INPUT_PULLDOWN);
  }
}

/* ───────────────────────────────────────────────
 * FUNCTION: initLEDS
 * ─────────────────────────────────────────────── */
bool initLEDS(Adafruit_NeoPixel *leds)
{
  if (!leds->begin())
    return (false);
  leds->setBrightness(80);
  leds->clear();
  leds->show();

  return (true);
}

/* ───────────────────────────────────────────────
 * FUNCTION: configIOEX
 * ─────────────────────────────────────────────── */
bool configIOEX(TwoWire *wireInt, PCA9555 *io0, PCA9555 *io1)
{
  bool status = true;
  // config I2C pins
  status &= wireInt->setSCL(I2C_SCL_PIN);
  status &= wireInt->setSDA(I2C_SDA_PIN);
  wireInt->begin();

  // IO Expander config
  io0->attach(*wireInt, 0x20);
  io0->polarity(PCA95x5::Polarity::ORIGINAL_ALL);
  io0->direction(PCA95x5::Direction::IN_ALL);

  io1->attach(*wireInt, 0x21);
  io1->polarity(PCA95x5::Polarity::ORIGINAL_ALL);
  io1->direction(PCA95x5::Direction::IN_ALL);

  return (status);
}

/* ───────────────────────────────────────────────
 * FUNCTION: configRotorsPins
 *
 * Configures the pins used for stepper operation
 * ─────────────────────────────────────────────── */
bool configRotorsPins(RotorHardware_t *rcfg)
{
  if (rcfg == NULL)
    return false;

  uint8_t r = 0;

  pinMode(rcfg->enPin, OUTPUT);

  ALLROTORS
  {
    pinMode(rcfg->dirPins[r], OUTPUT);
    pinMode(rcfg->stepPins[r], OUTPUT);

    digitalWrite(rcfg->stepPins[r], 0);
    digitalWrite(rcfg->dirPins[r], rcfg->directions[r]); // set normal direction
  }

  return (true);
}

/* ───────────────────────────────────────────────
 * FUNCTION:rotorID
 *
 * Rotor "homing" sequence to determine
 * rotor#, stellung and start position for each
 * rotor. Returns false if error.
 * ─────────────────────────────────────────────── */
bool rotorID(RotorHardware_t *rcfg, Adafruit_MCP3008 *adc)
{
  if (rcfg == NULL)
    return (false);

  // local vars
  uint8_t l = 0, i = 0, r = 0;
  int16_t magnetData[NUMROTORS][2][26] = {0}; // raw sensor data
  int16_t lap_avg[NUMROTORS][2] = {0};        // average reading over a lap
  uint8_t firstMagnetIdx[NUMROTORS][2] = {0}; // index of first magnet
  uint8_t numMagnets[NUMROTORS][2] = {0};     // number of magnets found
  uint8_t rotorNum[NUMROTORS] = {0};          // read number of rotor

  uint8_t corrFirstMagnet[NUMROTORS] = {0};   // corrected index for edge cases
  uint8_t stellungCorrector[NUMROTORS] = {0}; // corrected stellung for edge cases
  uint8_t digit = 0;

  // Offset half a letter
  // set direction
  ALLROTORS digitalWrite(rcfg->dirPins[r], rcfg->directions[r]);

  for (i = 0; i < (rcfg->numSteps / 2); i++) // advance half a letter
  {
    ALLROTORS digitalWrite(rcfg->stepPins[r], 1);
    delay(5);
    ALLROTORS digitalWrite(rcfg->stepPins[r], 0);
  }

  for (l = 0; l < 26; l++) // move one lap
  {
    for (i = 0; i < rcfg->numSteps; i++) // step one letter
    {
      ALLROTORS
      {
        digitalWrite(rcfg->stepPins[r], 1);
        magnetData[r][0][l] += adc->readADC(2 * r);
        magnetData[r][1][l] += adc->readADC(2 * r + 1);
      }
      delay(4);
      ALLROTORS digitalWrite(rcfg->stepPins[r], 0);
    }

    ALLROTORS
    {
      magnetData[r][0][l] /= rcfg->numSteps; // compute average of letter
      magnetData[r][1][l] /= rcfg->numSteps;

      lap_avg[r][0] += magnetData[r][0][l]; // add to turn average
      lap_avg[r][1] += magnetData[r][1][l];
    }

    delay(50);
  }

  // Offset back half a letter
  // set direction
  ALLROTORS digitalWrite(rcfg->dirPins[r], !(rcfg->directions[r]));

  for (i = 0; i < (rcfg->numSteps / 2); i++) // advance half a letter
  {
    ALLROTORS digitalWrite(rcfg->stepPins[r], 1);
    delay(5);
    ALLROTORS digitalWrite(rcfg->stepPins[r], 0);
  }

  // set direction correctly again
  ALLROTORS digitalWrite(rcfg->dirPins[r], (rcfg->directions[r]));

  ALLROTORS
  {
    lap_avg[r][0] /= 26; // calculate averages for the whole turn
    lap_avg[r][1] /= 26;
  }

  // parse data
  for (l = 0; l < 26; l++)
  {
    ALLROTORS
    {
      // rotor magnet
      magnetData[r][0][l] -= lap_avg[r][0]; // normalise to average
      if (magnetData[r][0][l] < MAG_THR && magnetData[r][0][l] > (-MAG_THR))
        magnetData[r][0][l] = 0;
      if (magnetData[r][0][l] >= MAG_THR)
        magnetData[r][0][l] = 1;
      if (magnetData[r][0][l] <= -MAG_THR)
        magnetData[r][0][l] = -1;

      // magnet detected
      if (magnetData[r][0][l] != 0)
      {
        if (numMagnets[r][0] == 0) // if it's the first
        {
          firstMagnetIdx[r][0] = l;
        }

        numMagnets[r][0]++;

        if (numMagnets[r][0] > 3)
          return (false);
      }

      // text magnet
      magnetData[r][1][l] -= lap_avg[r][1];
      if (magnetData[r][1][l] < MAG_THR && magnetData[r][1][l] > (-MAG_THR))
        magnetData[r][1][l] = 0;
      if (magnetData[r][1][l] >= MAG_THR)
        magnetData[r][1][l] = 1;
      if (magnetData[r][1][l] <= -MAG_THR)
        magnetData[r][1][l] = -1;

      // magnet detected
      if (magnetData[r][1][l] != 0)
      {
        firstMagnetIdx[r][1] = l;
        numMagnets[r][1]++;
        if (numMagnets[r][1] > 1)
          return (false);
      }
    }
  }

  // calculate offsets and store to rotorConfig
  ALLROTORS
  {
    // error detection and correction
    corrFirstMagnet[r] = firstMagnetIdx[r][0]; // normal case
    if (firstMagnetIdx[r][0] == 0)             // started on a magnet
    {
      if (magnetData[r][0][24] != 0) // it was the 2nd magnet
      {
        stellungCorrector[r] = 2;
        corrFirstMagnet[r] = 24;
      }
      else if (magnetData[r][0][25] != 0) // it was the 1st magnet
      {
        stellungCorrector[r] = 1;
        corrFirstMagnet[r] = 25;
      }
    }

    // parse rotor number
    for (i = corrFirstMagnet[r]; i < (corrFirstMagnet[r] + 3); i++)
    {
      digit = magnetData[r][0][i % 26] == -1 ? 0 : 1;
      rotorNum[r] |= (digit << (i - corrFirstMagnet[r]));
    }

    rcfg->ident[r][0] = rotorNum[r];
    rcfg->ident[r][2] = (26 + firstMagnetIdx[r][0] - firstMagnetIdx[r][1] - stellungCorrector[r]) % 26;
    rcfg->ident[r][1] = (26 + rcfg->sensorOffset - firstMagnetIdx[r][0] + rcfg->ident[r][2] + stellungCorrector[r]) % 26;

    if (numMagnets[r][0] != 3 || numMagnets[r][1] != 1)
    {
#if SERIALDEBUG
      Serial.print("Rotor ");
      Serial.print(r);
      Serial.print(": ");
      Serial.println("Not enough magnets found");
      Serial.print(numMagnets[r][0]);
      Serial.print(", ");
      Serial.println(numMagnets[r][1]);
#endif
      return (false); // not enough magnets found
    }
  }

// Print data
#if SERIALDEBUG
  // convert to letter
  char letter[2] = {0};
  sprintf(letter, "%c", rcfg->ident[0][1] + 'A');

  Serial.println("Identification finished!");
  Serial.print("Rotor number: ");
  Serial.println(rcfg->ident[0][0], BIN);
  Serial.print("Start Letter: ");
  Serial.println(letter);
  Serial.print("Stellung: ");
  Serial.println(rcfg->ident[0][2]);
#endif

  return (true);
}

/************* Function moveAllRotors ***************
 * Moves all the rotors to match the software
 * position
 */
bool moveAllRotors(struct Enigma *machine, RotorHardware_t *rcfg)
{
  // local vars
  uint8_t r = 0, i = 0, m = 0;
  uint8_t ui8_move = 0;
  int8_t deltaPos[NUMROTORS] = {0};

  ALLROTORS
  {
    deltaPos[r] = (machine->rotors[r].offset - machine->rotors[r].realPos); // compute offset, correct sign

    // during operation machine should only move in the normal direction
    // unless it's a backspace, in which case up to -2 is possible 
    // negative probably means Z-A transition
    // other cases are rare enough to bother making them perfect.
    if(deltaPos[r] < -2)  //not a backspace
    {
      deltaPos[r] += 26;  //
    }

    if (deltaPos[r] != 0) // move needed
    {
      digitalWrite(rcfg->dirPins[r], deltaPos[r] >= 0 ? rcfg->directions[r] : !rcfg->directions[r]);  //write direction
      deltaPos[r] < 0 ? -deltaPos[r] : deltaPos[r];     // compute absolute value
      ui8_move >= deltaPos[r] ? ui8_move : deltaPos[r]; // find maximum number of moves for all rotors
    }
  }

  //TODO:Advanced : if rotor ends up on 'A', home using strongest magnet position
  //that would be dope but a bit hard maybe
  for (m = 0; m < ui8_move; m++) // for the max # of moves
  {
    for (i = 0; i < (rcfg->numSteps); i++) // advance a lettre if needed
    {
      ALLROTORS if (deltaPos[r] != 0) digitalWrite(rcfg->stepPins[r], 1);
      delay(5);
      ALLROTORS if (deltaPos[r] != 0) digitalWrite(rcfg->stepPins[r], 0);
    }

    ALLROTORS deltaPos[r]--; // decrement # of moves
  }

  ALLROTORS
  {
    digitalWrite(rcfg->dirPins[r], rcfg->directions[r]);    // set normal direction
    machine->rotors[r].realPos = machine->rotors[r].offset; // update position variable
  }

  return (true);
}

// Timer handler
bool timerHandlerMillis(struct repeating_timer *t)
{
  ui32_msCounter++;
  return (true);
}

//helper function, print current position through serial
void printPosition(SerialUSB Serial, struct Enigma* machine)
{
  const char* alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"; 

  char buf[50] = {0};

  sprintf(buf, "|%s|%s|%s|\n| %c | %c | %c |",
                machine->rotors[2].name, machine->rotors[1].name, machine->rotors[0].name,
                alphabet[machine->rotors[2].offset],alphabet[machine->rotors[1].offset],alphabet[machine->rotors[0].offset] );

  Serial.println(buf);
}