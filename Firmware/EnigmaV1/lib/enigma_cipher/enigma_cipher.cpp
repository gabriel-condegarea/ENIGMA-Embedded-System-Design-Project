#include "enigma_cipher.h"

#include <Arduino.h>

//eww global vars

const char *alpha = 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
const char *rotor_ciphers[] = {
    "EKMFLGDQVZNTOWYHXUSPAIBRCJ",
    "AJDKSIRUXBLHWTMCQGZNPYFVOE",
    "BDFHJLCPRTXVZNYEIWGAKMUSQO",
    "ESOVPZJAYQUIRHXLNFTGKDCMWB",
    "VZBRGITYUPSDNHLXAWMJQOFECK",
    "JPGVOUMFYQBENHZRDKASXLICTW",
    "NZJHGRCXMYSWBOUFAIVLPEKQDT",
    "FKQHTLXOCBJSPDZRAMEWNIUYGV"
};

const char *rotor_notches[] = {"Q", "E", "V", "J", "Z", "ZM", "ZM", "ZM"};

const char *rotor_turnovers[] = {"R", "F", "W", "K", "A", "AN", "AN", "AN"};

const char *reflectors[] = {
    "EJMZALYXVBWFCRQUONTSPIKHGD",
    "YRUHQSLDPXNGOKMIEBFZCWVJAT",
    "FVPJIAOYEDRZXWGCTKUQSBNMHL"
};

const char *rotor_names[] = {"I", "II", "III", "IV", "V", "VI", "VII", "VIII"};  //rotor roman numerals

char plugboard[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";  


extern uint8_t opt_debug;

/*
 * Produce a rotor object
 * Setup the correct offset, cipher set and turn overs.
 */
struct Rotor new_rotor(struct Enigma *machine, uint8_t rotornumber, uint8_t offset, uint8_t stellung) 
{
    struct Rotor r;
    strcpy(r.name, rotor_names[rotornumber-1]);
    r.offset = offset;
    r.realPos = offset;
    r.turnnext = 0;
    r.stellung = stellung;
    r.cipher = rotor_ciphers[rotornumber - 1];
    r.turnover = rotor_turnovers[rotornumber - 1];
    r.notch = rotor_notches[rotornumber - 1];
    //machine->numrotors++;     //probably not a great idea if we go through this multiple times

    return r;
}

/*
 * Return the index position of a character inside a string
 * if not found then -1
 **/
int8_t str_index(const char *str, uint8_t character) 
{
    char *pos;
    int index;
    pos = strchr(str, character);

    // pointer arithmetic
    if (pos){
        index = (int) (pos - str);
    } else {
        index = -1;
    }

    return index;
}

/*
 * Cycle a rotor's offset but keep it in the array.
 */
void rotor_cycle(struct Rotor *rotor)
{
    rotor->offset++;
    rotor->offset = rotor->offset % ROTATE;

    // Check if the notch is active, if so trigger the turnnext
    if(str_index(rotor->turnover, alpha[rotor->offset]) >= 0) {
        rotor->turnnext = 1;
    }
}

/*
 * Pass through a rotor, right to left, cipher to alpha.
 * returns the exit index location.
 */
uint8_t rotor_forward(struct Rotor *rotor, uint8_t index) 
{

    // In the alpha side, out the cipher side
    index = (ROTATE + index + rotor->offset - rotor->stellung)  % ROTATE;   //add offset + stellung
    index = str_index(alpha, rotor->cipher[index]); //find corresponding letter

    index = (ROTATE + index - rotor->offset + rotor->stellung) % ROTATE;  //remove offset + stellung
    return index;
}

/*
 * Pass through a rotor, left to right, alpha to cipher.
 * returns the exit index location.
 */
uint8_t rotor_reverse(struct Rotor *rotor, uint8_t index)
{

    // In the cipher side, out the alpha side
    index = (ROTATE + index + rotor->offset - rotor->stellung) % ROTATE;   //add offset
    index = str_index(rotor->cipher, alpha[index]);
    index = (ROTATE + index - rotor->offset + rotor->stellung) % ROTATE;  //remove offset
    return index;
}

uint8_t cycleAllRotors(struct Enigma *machine)
{
    //local vars
    uint8_t i = 0, outChar = 0;

    // Cycle first rotor before pushing through,
    rotor_cycle(&machine->rotors[0]);

    // Double step the rotor
    if(str_index(machine->rotors[1].notch,
                alpha[machine->rotors[1].offset]) >= 0 ) {
        rotor_cycle(&machine->rotors[1]);
    }

    // Stepping the rotors

    for(i=0; i < machine->numrotors - 1; i++)    //for each rotor
    {
        outChar = alpha[machine->rotors[i].offset];

        if(machine->rotors[i].turnnext)  //if notch is aligned basically
        {
            machine->rotors[i].turnnext = 0;
            rotor_cycle(&machine->rotors[i+1]);
            if(opt_debug)
            {
                Serial.print("Cycling rotor: ");
                Serial.println(i+1);
                Serial.print("Turnover rotor ");
                Serial.println(i);
                Serial.print("Character is ");
                Serial.println(alpha[outChar]);
            }
        }
    }

    return(outChar);
}

extern SerialUSB Serial;

//TODO maybe delete all the printfs
uint8_t enigma_encrypt(struct Enigma *machine, uint8_t inChar)
{
    //local vars
    uint8_t index = 0, character = 0;
    int8_t i = 0; 

    char printBuf[100];

    index = str_index(alpha, inChar);   //TODO replace with -'A'
    //plugboard 1
    index = str_index(alpha, plugboard[index]);

    if(opt_debug)
    {
        Serial.print("After plugboard: ");
        Serial.println(alpha[index]);
    }


    // Pass through all the rotors forward
    for(i=0; i < machine->numrotors; i++)    //each rotor
    {   
        if(opt_debug)
        {
            sprintf(printBuf, "In rotor %s as %c; ", machine->rotors[i].name, alpha[index]);
            Serial.print(printBuf);
        } 
        index = rotor_forward(&machine->rotors[i], index);
        if(opt_debug)
        {
            sprintf(printBuf, "Out rotor as %c", alpha[index]); 
            Serial.println(printBuf);
        } 
    }

    
    //Pass through the reflector
    if(opt_debug) 
    {   
        Serial.print("After rotors: ");
        Serial.println(index);
        sprintf(printBuf, "Into reflector as %c; ", alpha[index]);
        Serial.print(printBuf);
        sprintf(printBuf, "Out of reflector as %c\n", machine->reflector[index]);
        Serial.print(printBuf);
    }

    // Inbound 
    character = machine->reflector[index];
    // Outbound
    index = str_index(alpha, character);

    // Pass back through the rotors in reverse
    for(i = machine->numrotors - 1; i >= 0; i--) 
    {
        if(opt_debug)
        {
            sprintf(printBuf, "In rotor %s as %c; ", machine->rotors[i].name, alpha[index]);
            Serial.print(printBuf);
        } 

        index = rotor_reverse(&machine->rotors[i], index);

        if(opt_debug)
        {
            sprintf(printBuf, "Out rotor as %c", alpha[index]); 
            Serial.println(printBuf);
        } 
    }

    // if(opt_debug)
    // {
    //     Serial.print("Back through rotors: ");
    //     Serial.println(alpha[index]);
    // }

    // Pass through Plugboard the 2nd time
    index = str_index(alpha, plugboard[index]);

    if(opt_debug)
    {
        Serial.print("Back through plugboard: ");
        Serial.println(alpha[index]);
    }

    character = alpha[index];

    return(character);
}