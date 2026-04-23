#include <stdio.h>
#include "hardware_mapping.h"

int main()
{
    uint8_t letter = keyboard_to_letter(0, 0);
    printf("Letter: %c\n", letter);

    int8_t led = letter_to_led_index(letter);
    printf("LED index: %d\n", led);

    return 0;
}