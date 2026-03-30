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