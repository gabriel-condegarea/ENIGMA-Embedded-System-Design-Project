#pragma once

//const
#define ROTATE 26


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