#include "enigma.h"

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

const char *plugboard = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";  
            //"ABCDEFGHIJKLMNOPQRSTUVWXYZ";   //TODO find a way to fill in 


extern int opt_debug;


/*
 * Produce a rotor object
 * Setup the correct offset, cipher set and turn overs.
 */
struct Rotor new_rotor(struct Enigma *machine, int rotornumber, int offset, int stellung) 
{
    struct Rotor r;
    strcpy(r.name, rotor_names[rotornumber-1]);
    r.offset = offset;
    r.turnnext = 0;
    r.stellung = stellung;
    r.cipher = rotor_ciphers[rotornumber - 1];
    r.turnover = rotor_turnovers[rotornumber - 1];
    r.notch = rotor_notches[rotornumber - 1];
    machine->numrotors++;

    return r;
}

/*
 * Return the index position of a character inside a string
 * if not found then -1
 **/
int str_index(const char *str, int character) 
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
int rotor_forward(struct Rotor *rotor, int index) 
{

    // In the alpha side, out the cipher side
    index = (ROTATE + index + rotor->offset - rotor->stellung)  % ROTATE;   //add offset
    index = str_index(alpha, rotor->cipher[index]); //find corresponding letter

    index = (ROTATE + index - rotor->offset + rotor->stellung) % ROTATE;  //remove offset
    return index;
}

/*
 * Pass through a rotor, left to right, alpha to cipher.
 * returns the exit index location.
 */
int rotor_reverse(struct Rotor *rotor, int index)
{

    // In the cipher side, out the alpha side
    index = (ROTATE + index + rotor->offset - rotor->stellung) % ROTATE;   //add offset
    index = str_index(rotor->cipher, alpha[index]);
    index = (ROTATE + index - rotor->offset + rotor->stellung) % ROTATE;  //remove offset
    return index;
}

uint8_t cycleAllRotors(struct Enigma *machine, uint8_t inChar)
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
                printf("Cycling  rotor :%d \n", i+1);
                printf("Turnover rotor :%d \n", i);
                printf("Character  is  :%c \n", outChar);
            }
        }
    }

    return(outChar);
}


uint8_t enigma_encrypt(struct Enigma *machine, uint8_t inChar)
{
    //local vars
    uint8_t index = 0, character = 0;
    int i = 0;  //TODO change to uint8_t but it breaks a for


    index = str_index(alpha, inChar); 
    //plugboard 1
    index = str_index(alpha, plugboard[index]);
    if(opt_debug)
    {
        printf("After plugboard: %c\n", alpha[index]);
    }

    // Pass through all the rotors forward
    for(i=0; i < machine->numrotors; i++)    //each rotor
    {   
        if(opt_debug) printf("In rotor %s as %c; ", machine->rotors[i].name, alpha[index]);
        index = rotor_forward(&machine->rotors[i], index);
        if(opt_debug) printf("Out rotor as %c\n", alpha[index]); 
    }

    // Pass through the reflector
    if(opt_debug) {
        printf("Into reflector as %c; ", alpha[index]);
        printf("Out of reflector as %c\n", machine->reflector[index]);
    }

    // Inbound  //TODO kinda dumb that the reflector works on characters
    character = machine->reflector[index];
    // Outbound
    index = str_index(alpha, character);

    // Pass back through the rotors in reverse
    for(i = machine->numrotors - 1; i >= 0; i--) 
    {
        if(opt_debug) printf("In rotor %s as %c; ", machine->rotors[i].name, alpha[index]);
        index = rotor_reverse(&machine->rotors[i], index);
        if(opt_debug) printf("Out rotor as %c\n", alpha[index]); 
    }

    // Pass through Plugboard the 2nd time
    index = str_index(alpha, plugboard[index]);
    if(opt_debug)
    {
        printf("After plugboard: %c\n", alpha[index]);
    }

    character = alpha[index];

    return(character);
}