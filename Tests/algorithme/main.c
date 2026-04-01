/* Copyright 2013 Cory Lutton                                               */
/*                                                                          */
/* Licensed under the Apache License, Version 2.0 (the "License");          */
/* you may not use this file except in compliance with the License.         */
/* You may obtain a copy of the License at                                  */
/*                                                                          */
/*    http://www.apache.org/licenses/LICENSE-2.0                            */
/*                                                                          */
/* Unless required by applicable law or agreed to in writing, software      */
/* distributed under the License is distributed on an "AS IS" BASIS,        */
/* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied  */
/* See the License for the specific language governing permissions and      */
/* limitations under the License.                                           */

/*The rotors (alternatively wheels or drums, Walzen in German)
formed the heart of an Enigma machine. Each rotor was a disc
approximately 10 cm (3.9 in) in diameter made from hard rubber
or bakelite with brass spring-loaded pins on one face arranged
in a circle; on the other side are a corresponding number
of circular electrical contacts. The pins and contacts represent
the alphabet, typically the 26 letters A to Z.

Setting Wiring                      Notch   Window  Turnover
Base    ABCDEFGHIJKLMNOPQRSTUVWXYZ
I       EKMFLGDQVZNTOWYHXUSPAIBRCJ  Y       Q       R
II      AJDKSIRUXBLHWTMCQGZNPYFVOE  M       E       F
III     BDFHJLCPRTXVZNYEIWGAKMUSQO  D       V       W
IV      ESOVPZJAYQUIRHXLNFTGKDCMWB  R       J       K
V       VZBRGITYUPSDNHLXAWMJQOFECK  H       Z       A
VI      JPGVOUMFYQBENHZRDKASXLICTW  H/U     Z/M     A/N
VII     NZJHGRCXMYSWBOUFAIVLPEKQDT  H/U     Z/M     A/N
VIII    FKQHTLXOCBJSPDZRAMEWNIUYGV  H/U     Z/M     A/N

With the exception of the early Enigma models A and B,
the last rotor came before a reflector (German: Umkehrwalze,
meaning reversal rotor), a patented feature distinctive of the
Enigma family amongst the various rotor machines designed
in the period. The reflector connected outputs of the
last rotor in pairs, redirecting current back through the
rotors by a different route. The reflector ensured that
Enigma is self-reciprocal: conveniently, encryption was
the same as decryption. However, the reflector also gave
Enigma the property that no letter ever encrypted to itself.
This was a severe conceptual flaw and a cryptological mistake
subsequently exploited by codebreakers.

Setting     Wiring
Base        ABCDEFGHIJKLMNOPQRSTUVWXYZ
A           EJMZALYXVBWFCRQUONTSPIKHGD
B           YRUHQSLDPXNGOKMIEBFZCWVJAT
C           FVPJIAOYEDRZXWGCTKUQSBNMHL
*/
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "enigma.h" //constants, definitions etc.


//global vars
extern const char *alpha;
// extern const char *rotor_ciphers[];
// extern const char *rotor_notches[];
// extern const char *rotor_turnovers[];
extern const char *reflectors[];
extern const char *rotor_names[];
// extern const char *plugboard;





int opt_debug = 0;

int main(int argc, char *argv[])
{
    struct Enigma machine = {}; // initialized to defaults
    int i, character, index;

    // Command line options
    
    int opt_r1 = 1, opt_r2 = 2, opt_r3 = 3; //rotor choice
    int opt_o1 = 0, opt_o2 = 0, opt_o3 = 0; //rotor offset (start pos)
    int opt_s1 = 0, opt_s2 = 0, opt_s3 = 0; //ring setting (stellung)

    // Command Parsing
    for (i = 1; i < argc; i++)  //for each parameter
    {
        if (strcmp(argv[i], "-d") == 0) opt_debug = 1;

        if (strcmp(argv[i], "-r") == 0)     //rotor position
        {   
            opt_r3 = atoi(&argv[i+1][0])/100;   //in order as written
            opt_r2 = atoi(&argv[i+1][1])/10;
            opt_r1 = atoi(&argv[i+1][2]);
            i++;
        }
        if (strcmp(argv[i], "-o") == 0)     //start position   
        {
            opt_o3 = str_index(alpha,argv[i+1][0]);
            opt_o2 = str_index(alpha,argv[i+1][1]);
            opt_o1 = str_index(alpha,argv[i+1][2]);
            i++;
        }

        if (strcmp(argv[i], "-s") == 0)     //stellung  
        {
            opt_s3 = str_index(alpha,argv[i+1][0]);
            opt_s2 = str_index(alpha,argv[i+1][1]);
            opt_s1 = str_index(alpha,argv[i+1][2]);
            i++;
        }
    }

    if(opt_debug) {
        printf("Rotors set to : %d %d %d \n", opt_r3, opt_r2, opt_r1);
        printf("Offsets set to: %d %d %d \n", opt_o3, opt_o2, opt_o1);
        printf("Setting set to: %d %d %d \n", opt_s3, opt_s2, opt_s1);
    }


    // Configure an enigma machine
    machine.reflector = reflectors[1];
    machine.rotors[0] = new_rotor(&machine, opt_r1, opt_o1, opt_s1);
    machine.rotors[1] = new_rotor(&machine, opt_r2, opt_o2, opt_s2);
    machine.rotors[2] = new_rotor(&machine, opt_r3, opt_o3, opt_s3);

    printf("Start Position:\n|%s|%s|%s|\n| %c | %c | %c |\n\n",
            machine.rotors[2].name, machine.rotors[1].name, machine.rotors[0].name,
            alpha[machine.rotors[2].offset],alpha[machine.rotors[1].offset],alpha[machine.rotors[0].offset]);

    while((character = getchar())!=EOF)     //for each char
    {

        if (!isalpha(character)) 
        {
            printf("%c", character);
            continue;
        }

        //TODO KEYBOARD SEEK COMES HERE
        character = toupper(character);
  
        if(opt_debug) 
        {
            printf("Input character ******** %c \n", character);
        }


        //rotors cycling
        cycleAllRotors(&machine, character);
        
        //TODO motor move comes here


        if(opt_debug)
        {
            printf("Position after cycling:\n|%s|%s|%s|\n| %c | %c | %c |\n\n",
                machine.rotors[2].name, machine.rotors[1].name, machine.rotors[0].name,
                alpha[machine.rotors[2].offset],alpha[machine.rotors[1].offset],alpha[machine.rotors[0].offset] );
        }

        character = enigma_encrypt(&machine, character);

        //TODO LED light comes HERE

        if(opt_debug) 
        {
           printf("Output character ******** ");
        }
        putchar(character);

        if(opt_debug) printf("\n\n");
    }

    return 0;
}