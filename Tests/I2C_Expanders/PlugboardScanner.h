ifndef PLUGBOARD_SCANNER_H
#define PLUGBOARD_SCANNER_H

#include <Arduino.h>

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

    // Initialise le module plugboard.
    // À appeler après l'initialisation I2C et après l'initialisation des PCA9555.
    void initPlugboardScanner();

// Scanne le plugboard et met à jour la string de substitution.
void scannerPlugboard();

// Retourne la string actuelle du plugboard.
// Exemple : "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
const char *getPlugboardString();

// Applique la substitution du plugboard à une lettre.
// Exemple : si A <-> G, alors applyPlugboard('A') retourne 'G'.
char applyPlugboard(char lettre);

#endif