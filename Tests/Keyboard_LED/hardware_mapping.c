/**
* hardware_mapping.c
* Enigma Project — Hardware mapping layer
*
* Two functions:
*   keyboard_to_letter(row, col)  → char ('A'-'Z', or special)
*   letter_to_led_index(letter)   → int (0-25, index in WS2812B chain)
*
* Keyboard matrix: ROW0-ROW5 × COL0-COL4 (28 switches, 26 letters + 2 special)
*
* Physical layout:
*   Phys row 0: Q(R0C0) W(R0C1) E(R0C2) R(R0C3) T(R0C4) | Z(R3C0) U(R3C1) I(R3C2) O(R3C3) P(R3C4)
*   Phys row 1: A(R1C0) S(R1C1) D(R1C2) F(R1C3) G(R1C4) | H(R4C0) J(R4C1) K(R4C2) L(R4C3)
*   Phys row 2: Y(R2C0) X(R2C1) C(R2C2) V(R2C3) B(R2C4) | N(R5C0) M(R5C1) BKSP(R5C2)
*   Phys row 3: SPACE(R5C3)
*
* LED chain (D1→D26, index 0→25):
*   Q  W  E  R  T  Z  U  I  O  P  L  K  J  H  G  F  D  S  A  Y  X  C  V  B  N  M
*   0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25
*/

#include "hardware_mapping.h"

/* Special key return values (outside A-Z range) */
#define KEY_BACKSPACE  0x08
#define KEY_SPACE      0x20
#define KEY_INVALID    0xFF

/**
* keyboard_to_letter
*
* Converts a matrix (row, col) position to its corresponding character.
*
* @param row  Matrix row index (0-5), from ROW0-ROW5 scan lines
* @param col  Matrix column index (0-4), from COL0-COL4 scan lines
* @return     Uppercase letter 'A'-'Z', KEY_BACKSPACE, KEY_SPACE, or KEY_INVALID
*/
uint8_t keyboard_to_letter(uint8_t row, uint8_t col)
{
   /* Lookup table [row][col] — KEY_INVALID means no switch at that position */
   static const uint8_t key_map[6][5] = {
       /* COL:  0      1      2      3      4   */
       /* ROW0 */ { 'Q',   'W',   'E',   'R',   'T'          },
       /* ROW1 */ { 'A',   'S',   'D',   'F',   'G'          },
       /* ROW2 */ { 'Y',   'X',   'C',   'V',   'B'          },
       /* ROW3 */ { 'Z',   'U',   'I',   'O',   'P'          },
       /* ROW4 */ { 'H',   'J',   'K',   'L',   KEY_INVALID  },
       /* ROW5 */ { 'N',   'M',   KEY_BACKSPACE, KEY_SPACE, KEY_INVALID },
   };

   if (row > 5 || col > 4) return KEY_INVALID;
   return key_map[row][col];
}


/**
* letter_to_led_index
*
* Converts an uppercase letter to its index in the WS2812B chain (0-based).
* D1 is index 0, D26 is index 25.
*
* Chain order: Q W E R T Z U I O P L K J H G F D S A Y X C V B N M
*
* @param letter  Uppercase letter 'A'-'Z'
* @return        Index 0-25, or -1 if letter is invalid
*/
int8_t letter_to_led_index(uint8_t letter)
{
   /* led_index['A'-'A'] = index of letter in chain */
   static const int8_t led_index[26] = {
       /* A  B   C   D   E   F   G   H   I   J   K   L   M */
          18, 23, 21, 16,  2, 15, 14, 13,  7, 12, 11, 10, 25,
       /* N   O   P   Q   R   S   T   U   V   W   X   Y   Z */
          24,  8,  9,  0,  3, 17,  4,  6, 22,  1, 20, 19,  5
   };

   if (letter < 'A' || letter > 'Z') return -1;
   return led_index[letter - 'A'];
}