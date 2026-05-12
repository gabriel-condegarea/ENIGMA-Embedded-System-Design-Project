#pragma once

#include <string.h>
#include <stdint.h>
#include <stdio.h>

//const
#define ROTATE 26

#define OPT_DEBUG 1

//structures
struct Rotor 
{
    char            name[5];        //name in roman numerals
    int             offset;         //position offset
    int             stellung;   //ring position from wiring
    int             turnnext;       
    const char      *cipher;
    const char      *turnover;
    const char      *notch;
};

struct Enigma {
    int             numrotors;
    const char      *reflector;
    struct Rotor    rotors[8];
};


//function prototypes
struct Rotor new_rotor(struct Enigma *machine, int rotornumber, int offset, int stellung);
int str_index(const char *str, int character);
void rotor_cycle(struct Rotor *rotor);
int rotor_forward(struct Rotor *rotor, int index);
int rotor_reverse(struct Rotor *rotor, int index);
uint8_t cycleAllRotors(struct Enigma *machine, uint8_t inChar);
uint8_t enigma_encrypt(struct Enigma *machine, uint8_t inChar);

