/**
 * Tests/Keyboard_LED/Keyboard_LED.ino
 * Enigma Project — Keyboard to LED test
 *
 * Reads the keyboard matrix and returns the alphabet index of the pressed key.
 * Lights the corresponding WS2812B LED using the same alphabet index.
 *
 * Required functions for this test:
 *   - int8_t readKeyboard(void)      // returns A=0, B=1, ..., Z=25
 *   - void sendLED(uint8_t index)    // lights LED A=0, B=1, ..., Z=25
 *
 * Requires:
 *   - Adafruit NeoPixel library
 *
 * Adjust pin definitions below to match your actual wiring.
 */

#include <Adafruit_NeoPixel.h>
#include <stdint.h>

/* ───────────────────────────────────────────────
 * PIN CONFIGURATION
 * ─────────────────────────────────────────────── */

/* Keyboard matrix — ROW pins */
#define ROW0_PIN 1
#define ROW1_PIN 3
#define ROW2_PIN 28
#define ROW3_PIN 19
#define ROW4_PIN 26
#define ROW5_PIN 21

/* Keyboard matrix — COL pins */
#define COL0_PIN 20
#define COL1_PIN 22
#define COL2_PIN 27
#define COL3_PIN 2
#define COL4_PIN 0

/* WS2812B data pin */
/* NOTE: PIN_LED est réservé par le package arduino-pico (GPIO25), on utilise LED_DATA_PIN */
#define LED_DATA_PIN 18

/* ───────────────────────────────────────────────
 * CONSTANTS
 * ─────────────────────────────────────────────── */
#define NUM_ROWS 6
#define NUM_COLS 5
#define NUM_KEYS (NUM_ROWS * NUM_COLS)
#define NUM_LEDS 26

/* ───────────────────────────────────────────────
 * GLOBALS
 * ─────────────────────────────────────────────── */
Adafruit_NeoPixel leds(NUM_LEDS, LED_DATA_PIN, NEO_GRB + NEO_KHZ800);

const uint8_t row_pins[NUM_ROWS] = {
    ROW0_PIN, ROW1_PIN, ROW2_PIN, ROW3_PIN, ROW4_PIN, ROW5_PIN};

const uint8_t col_pins[NUM_COLS] = {
    COL0_PIN, COL1_PIN, COL2_PIN, COL3_PIN, COL4_PIN};

// /*
//  * key index = row * NUM_COLS + col
//  *
//  * Keyboard layout:
//  *   ROW0: Q W E R T
//  *   ROW1: A S D F G
//  *   ROW2: Y X C V B
//  *   ROW3: Z U I O P
//  *   ROW4: H J K L [invalid]
//  *   ROW5: N M [backspace] [space] [invalid]
//  *
//  * key_to_led[index] gives the LED index to light.
//  * Invalid keys map to -1.
//  */
// const int8_t key_to_led[NUM_KEYS] = {
//     /* ROW0 */ 0, 1, 2, 3, 4,
//     /* ROW1 */ 18, 17, 16, 15, 14,
//     /* ROW2 */ 19, 20, 21, 22, 23,
//     /* ROW3 */ 5, 6, 7, 8, 9,
//     /* ROW4 */ 13, 12, 11, 10, -1,
//     /* ROW5 */ 24, 25, 26, 27, -1};

/* TODO: REVOIR ICI SI TOUT EST CORRECT */

/*
 * key index = row * NUM_COLS + col
 *
 * Keyboard layout:
 *   ROW0: Q W E R T
 *   ROW1: A S D F G
 *   ROW2: Y X C V B
 *   ROW3: Z U I O P
 *   ROW4: H J K L [invalid]
 *   ROW5: N M [backspace] [space] [invalid]
 *
 * key_to_alpha[index] gives the alphabet index:
 *   A = 0, B = 1, ..., Z = 25
 *
 * Invalid/special keys map to -1.
 */
const int8_t key_to_alpha[NUM_KEYS] = {
    /* ROW0: Q W E R T */ 16, 22, 4, 17, 19,
    /* ROW1: A S D F G */ 0, 18, 3, 5, 6,
    /* ROW2: Y X C V B */ 24, 23, 2, 21, 1,
    /* ROW3: Z U I O P */ 25, 20, 8, 14, 15,
    /* ROW4: H J K L invalid */ 7, 9, 10, 11, -1,
    /* ROW5: N M backspace space invalid */ 13, 12, -1, -1, -1};

/* ───────────────────────────────────────────────
 * FUNCTION: readKeyboard
 *
 * Scans the keyboard matrix once.
 * Returns:
 *   -1 if no key is pressed
 *   alphabet index 0..25 if a valid letter key is pressed
 * ─────────────────────────────────────────────── */
int8_t readKeyboard(void)
{
    for (uint8_t row = 0; row < NUM_ROWS; row++)
    {
        /* Activate current row */
        pinMode(row_pins[row], OUTPUT);
        digitalWrite(row_pins[row], LOW);

        delayMicroseconds(10);

        for (uint8_t col = 0; col < NUM_COLS; col++)
        {
            // if (digitalRead(col_pins[col]) == LOW)
            // {
            //     /* Release current row before returning */
            //     digitalWrite(row_pins[row], HIGH);
            //     pinMode(row_pins[row], INPUT);
            //
            //     return (int8_t)(row * NUM_COLS + col);
            // }

            /* TODO: REVOIR SI TOUT EST CORRECT ICI */

            if (digitalRead(col_pins[col]) == LOW)
            {
                /* Release current row before returning */
                digitalWrite(row_pins[row], HIGH);
                pinMode(row_pins[row], INPUT);

                uint8_t key_index = row * NUM_COLS + col;

                return key_to_alpha[key_index];
            }
        }

        /* Release current row */
        digitalWrite(row_pins[row], HIGH);
        pinMode(row_pins[row], INPUT);
    }

    return -1;
}

/* ───────────────────────────────────────────────
 * FUNCTION: sendLED
 *
 * Turns off all LEDs, then lights the LED corresponding
 * to the given key index.
 * Invalid/special keys do not light any LED.
 * ─────────────────────────────────────────────── */

// void sendLED(uint8_t index)
// {
//     leds.clear();
//
//     if (index < NUM_KEYS)
//     {
//         int8_t led_index = key_to_led[index];
//
//         if (led_index >= 0 && led_index < NUM_LEDS)
//         {
//             leds.setPixelColor(led_index, leds.Color(255, 200, 50));
//         }
//     }
//
//     leds.show();
// }

/* TODO: REVOIR SI TOUT CORRECT */

void sendLED(uint8_t letter_index)
{
    leds.clear();

    if (letter_index < NUM_LEDS)
    {
        leds.setPixelColor(letter_index, leds.Color(255, 200, 50));
    }

    leds.show();
}

/* ───────────────────────────────────────────────
 * SETUP
 * ─────────────────────────────────────────────── */
void setup(void)
{
    Serial.begin(115200);

    /* Columns as input pull-up */
    for (uint8_t col = 0; col < NUM_COLS; col++)
    {
        pinMode(col_pins[col], INPUT_PULLUP);
    }

    /* Rows idle as input (high impedance) */
    for (uint8_t row = 0; row < NUM_ROWS; row++)
    {
        pinMode(row_pins[row], INPUT);
    }

    leds.begin();
    leds.setBrightness(80);
    leds.clear();
    leds.show();

    Serial.println("Keyboard_LED test ready.");
}

/* ───────────────────────────────────────────────
 * LOOP
 * ─────────────────────────────────────────────── */
void loop(void)
{
    int8_t key = readKeyboard();

    if (key != -1)
    {
        Serial.print("Letter index pressed: ");
        Serial.println(key);

        sendLED((uint8_t)key);

        /* Simple debounce */
        delay(200);
    }
    else
    {
        leds.clear();
        leds.show();
    }

    delay(10);
}