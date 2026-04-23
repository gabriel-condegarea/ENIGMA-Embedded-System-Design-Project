#ifndef HARDWARE_MAPPING_H
#define HARDWARE_MAPPING_H

#include <stdint.h>

uint8_t keyboard_to_letter(uint8_t row, uint8_t col);
int8_t letter_to_led_index(uint8_t letter);

#endif