#pragma once

#include <string.h>
#include <stdint.h>
#include <stdio.h>

//const
#define ROTATE 26


//structures
struct Rotor 
{
    char            name[5];        //name in roman numerals
    uint8_t         offset;         //position offset
    uint8_t         stellung;   //ring position from wiring
    int8_t          turnnext;       
    const char      *cipher;
    const char      *turnover;
    const char      *notch;
    uint8_t         realPos;
};

struct Enigma {
    uint8_t             numrotors;
    const char      *reflector;
    struct Rotor    rotors[8];
};


//function prototypes
struct Rotor new_rotor(struct Enigma *machine, uint8_t rotornumber, uint8_t offset, uint8_t stellung);
int8_t str_index(const char *str, uint8_t character);
void rotor_cycle(struct Rotor *rotor);
uint8_t rotor_forward(struct Rotor *rotor, uint8_t index);
uint8_t rotor_reverse(struct Rotor *rotor, uint8_t index);
uint8_t cycleAllRotors(struct Enigma *machine);
uint8_t enigma_encrypt(struct Enigma *machine, uint8_t inChar);

