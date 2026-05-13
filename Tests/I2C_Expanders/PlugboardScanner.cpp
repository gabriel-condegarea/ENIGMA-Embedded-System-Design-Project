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

#include "PlugboardScanner.h"

#include <Arduino.h>
#include <PCA95x5.h>

// Les objets PCA9555 sont déclarés et initialisés ailleurs
// dans le programme principal.
extern PCA9555 ioex0; // K7 - Adresse I2C 0x20
extern PCA9555 ioex1; // K8 - Adresse I2C 0x21

struct LettrePin
{
    char lettre;
    uint8_t chip; // 0 = ioex0, 1 = ioex1
    uint8_t bit;  // bit du PCA9555
};

// Mapping selon le schéma KiCad
static LettrePin lettres[] = {
    // K7 / PCA9555 adresse 0x20
    {'Q', 0, 0},
    {'A', 0, 1},
    {'Y', 0, 2},
    {'W', 0, 3},
    {'S', 0, 4},
    {'X', 0, 5},
    {'E', 0, 6},
    {'D', 0, 7},
    {'C', 0, 8},
    {'R', 0, 9},
    {'F', 0, 10},
    {'V', 0, 11},
    {'T', 0, 12},
    {'G', 0, 13},
    {'B', 0, 14},
    {'Z', 0, 15},

    // K8 / PCA9555 adresse 0x21
    {'H', 1, 0},
    {'N', 1, 1},
    {'U', 1, 2},
    {'J', 1, 3},
    {'M', 1, 4},
    {'I', 1, 5},
    {'K', 1, 6},
    {'O', 1, 7},
    {'L', 1, 8},
    {'P', 1, 9}};

static const uint8_t NB_LETTRES = sizeof(lettres) / sizeof(lettres[0]);

// 26 lettres + caractère de fin '\0'
// Variable globale accessible depuis les autres fichiers via le .h
char plugboardString[27] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

// ---------------------------------------------------------------------------
// Initialise la string du plugboard
// ---------------------------------------------------------------------------
static void initialiserPlugboard()
{
    for (uint8_t i = 0; i < 26; i++)
    {
        plugboardString[i] = 'A' + i;
    }

    plugboardString[26] = '\0';
}

// ---------------------------------------------------------------------------
// Lie deux lettres dans la string du plugboard
// ---------------------------------------------------------------------------
static void lierLettres(char a, char b)
{
    if (a < 'A' || a > 'Z' || b < 'A' || b > 'Z')
    {
        return;
    }

    plugboardString[a - 'A'] = b;
    plugboardString[b - 'A'] = a;
}

// ---------------------------------------------------------------------------
// Met toutes les pins des deux PCA9555 en entrée
// ---------------------------------------------------------------------------
static void toutEnEntree()
{
    ioex0.direction(0xFFFF);
    ioex1.direction(0xFFFF);
}

// ---------------------------------------------------------------------------
// Prépare toutes les sorties à LOW
// Les pins ne forcent rien tant qu'elles restent configurées en entrée.
// ---------------------------------------------------------------------------
static void preparerSortiesLow()
{
    ioex0.write(0x0000);
    ioex1.write(0x0000);
}

// ---------------------------------------------------------------------------
// Met une seule lettre en sortie LOW
// Toutes les autres lettres restent en entrée.
// ---------------------------------------------------------------------------
static void activerLettreLow(uint8_t index)
{
    uint16_t dir0 = 0xFFFF;
    uint16_t dir1 = 0xFFFF;

    uint8_t chip = lettres[index].chip;
    uint8_t bit = lettres[index].bit;

    if (chip == 0)
    {
        dir0 &= ~(1 << bit);
    }
    else
    {
        dir1 &= ~(1 << bit);
    }

    preparerSortiesLow();

    ioex0.direction(dir0);
    ioex1.direction(dir1);

    delay(5);
}

// ---------------------------------------------------------------------------
// Retourne true si la lettre donnée est lue à LOW
// ---------------------------------------------------------------------------
static bool lettreEstLow(uint8_t index, uint16_t etat0, uint16_t etat1)
{
    uint8_t chip = lettres[index].chip;
    uint8_t bit = lettres[index].bit;

    if (chip == 0)
    {
        return ((etat0 >> bit) & 1) == 0;
    }

    return ((etat1 >> bit) & 1) == 0;
}

// ---------------------------------------------------------------------------
// Initialise le module plugboard
// À appeler après l'initialisation I2C et après l'initialisation des PCA9555.
// ---------------------------------------------------------------------------
void initPlugboardScanner()
{
    toutEnEntree();
    initialiserPlugboard();
}

// ---------------------------------------------------------------------------
// Scan complet du plugboard
// Met à jour plugboardString.
// ---------------------------------------------------------------------------
void scannerPlugboard()
{
    initialiserPlugboard();

    for (uint8_t i = 0; i < NB_LETTRES; i++)
    {
        toutEnEntree();
        delay(2);

        activerLettreLow(i);

        uint16_t etat0 = ioex0.read();
        uint16_t etat1 = ioex1.read();

        for (uint8_t j = 0; j < NB_LETTRES; j++)
        {
            if (j == i)
            {
                continue;
            }

            // Évite de détecter deux fois la même liaison
            if (j < i)
            {
                continue;
            }

            if (lettreEstLow(j, etat0, etat1))
            {
                lierLettres(lettres[i].lettre, lettres[j].lettre);
            }
        }
    }

    toutEnEntree();
}

// ---------------------------------------------------------------------------
// Applique la substitution du plugboard à une lettre
// ---------------------------------------------------------------------------
char applyPlugboard(char lettre)
{
    if (lettre < 'A' || lettre > 'Z')
    {
        return lettre;
    }

    return plugboardString[lettre - 'A'];
}