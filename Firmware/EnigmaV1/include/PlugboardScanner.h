#ifndef PLUGBOARD_SCANNER_H
#define PLUGBOARD_SCANNER_H

#include <Arduino.h>
#include "enigma_cipher.h"

// ============================================================================
// ENIGMA Project - Plugboard Scanner
// ============================================================================
// Module chargé de scanner les connexions physiques du plugboard
// et de générer une string de substitution de 26 lettres.
//
// Exemple :
//   Sans connexion : ABCDEFGHIJKLMNOPQRSTUVWXYZ
//   Avec A <-> G  : GBCDEFAHIJKLMNOPQRSTUVWXYZ
// ============================================================================

// String actuelle du plugboard.
// Elle est mise à jour par scannerPlugboard().
//
// Exemple :
//   "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
extern char plugboardString[27];

// Initialise le module plugboard.
// À appeler après l'initialisation I2C et après l'initialisation des PCA9555.
void initPlugboardScanner();

// Scanne le plugboard et met à jour plugboardString.
void scannerPlugboard();

// Applique la substitution du plugboard à une lettre.
// Exemple : si A <-> G, alors applyPlugboard('A') retourne 'G'.
char applyPlugboard(char lettre);

#endif