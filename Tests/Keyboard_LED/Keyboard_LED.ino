/**
 * Tests/Keyboard_LED/Keyboard_LED.ino
 * Enigma Project — Keyboard to LED test
 *
 * Reads the keyboard matrix and lights the corresponding WS2812B LED.
 *
 * Required functions for this test:
 *   - int8_t readKeyboard(void)
 *   - void sendLED(uint8_t index)
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
#define PIN_ROW0 1
#define PIN_ROW1 3
#define PIN_ROW2 28
#define PIN_ROW3 19
#define PIN_ROW4 26
#define PIN_ROW5 21

/* Keyboard matrix — COL pins */
#define PIN_COL0 20
#define PIN_COL1 22
#define PIN_COL2 27
#define PIN_COL3 2
#define PIN_COL4 0

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
    PIN_ROW0, PIN_ROW1, PIN_ROW2, PIN_ROW3, PIN_ROW4, PIN_ROW5};

const uint8_t col_pins[NUM_COLS] = {
    PIN_COL0, PIN_COL1, PIN_COL2, PIN_COL3, PIN_COL4};

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
 * key_to_led[index] gives the LED index to light.
 * Invalid/special keys map to -1.
 */
const int8_t key_to_led[NUM_KEYS] = {
    /* ROW0 */ 0, 1, 2, 3, 4,
    /* ROW1 */ 18, 17, 16, 15, 14,
    /* ROW2 */ 19, 20, 21, 22, 23,
    /* ROW3 */ 5, 6, 7, 8, 9,
    /* ROW4 */ 13, 12, 11, 10, -1,
    /* ROW5 */ 24, 25, -1, -1, -1};

/* ───────────────────────────────────────────────
 * FUNCTION: readKeyboard
 *
 * Scans the keyboard matrix once.
 * Returns:
 *   -1 if no key is pressed
 *   key index (0..29) if a key is pressed
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
            if (digitalRead(col_pins[col]) == LOW)
            {
                /* Release current row before returning */
                digitalWrite(row_pins[row], HIGH);
                pinMode(row_pins[row], INPUT);

                return (int8_t)(row * NUM_COLS + col);
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
void sendLED(uint8_t index)
{
    leds.clear();

    if (index < NUM_KEYS)
    {
        int8_t led_index = key_to_led[index];

        if (led_index >= 0 && led_index < NUM_LEDS)
        {
            leds.setPixelColor(led_index, leds.Color(255, 200, 50));
        }
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
        Serial.print("Key pressed -> key index: ");
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