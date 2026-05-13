// ============================================================================
// ENIGMA Project - Plugboard Scanner with PCA9555
// ============================================================================
// Principe :
// 1) Toutes les lettres sont en entrée.
// 2) Une lettre est temporairement mise en sortie LOW.
// 3) Toutes les autres lettres sont lues.
// 4) Si une autre lettre vaut LOW, elle est connectée à la lettre testée.
// ============================================================================

#include <Wire.h>
#include <PCA95x5.h>

PCA9555 ioex0; // K7 - Adresse I2C 0x20
PCA9555 ioex1; // K8 - Adresse I2C 0x21

struct LettrePin
{
    char lettre;
    uint8_t chip; // 0 = ioex0 / 0x20, 1 = ioex1 / 0x21
    uint8_t bit;  // bit du PCA9555
};

// Mapping selon ton schéma KiCad
LettrePin lettres[] = {
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

const uint8_t NB_LETTRES = sizeof(lettres) / sizeof(lettres[0]);

// ---------------------------------------------------------------------------
// Met toutes les pins des deux PCA9555 en entrée
// ---------------------------------------------------------------------------
void toutEnEntree()
{
    ioex0.direction(0xFFFF); // 1 = entrée
    ioex1.direction(0xFFFF); // 1 = entrée
}

// ---------------------------------------------------------------------------
// Prépare les sorties à 0.
// Important : tant que les pins sont en entrée, elles ne forcent rien.
// La pin sélectionnée deviendra ensuite une sortie LOW.
// ---------------------------------------------------------------------------
void preparerSortiesLow()
{
    ioex0.write(0x0000);
    ioex1.write(0x0000);
}

// ---------------------------------------------------------------------------
// Met une seule lettre en sortie LOW
// Toutes les autres lettres restent en entrée
// ---------------------------------------------------------------------------
void activerLettreLow(uint8_t index)
{
    uint16_t dir0 = 0xFFFF; // tout en entrée
    uint16_t dir1 = 0xFFFF; // tout en entrée

    uint8_t chip = lettres[index].chip;
    uint8_t bit = lettres[index].bit;

    if (chip == 0)
    {
        dir0 &= ~(1 << bit); // 0 = sortie sur ce bit
    }
    else
    {
        dir1 &= ~(1 << bit); // 0 = sortie sur ce bit
    }

    preparerSortiesLow();

    ioex0.direction(dir0);
    ioex1.direction(dir1);

    delay(5); // temps de stabilisation
}

// ---------------------------------------------------------------------------
// Retourne true si la lettre donnée est lue à LOW
// ---------------------------------------------------------------------------
bool lettreEstLow(uint8_t index, uint16_t etat0, uint16_t etat1)
{
    uint8_t chip = lettres[index].chip;
    uint8_t bit = lettres[index].bit;

    if (chip == 0)
    {
        return ((etat0 >> bit) & 1) == 0;
    }
    else
    {
        return ((etat1 >> bit) & 1) == 0;
    }
}

// ---------------------------------------------------------------------------
// Affiche les valeurs brutes en binaire sur 16 bits
// ---------------------------------------------------------------------------
void afficherBinaire16(uint16_t valeur)
{
    for (int i = 15; i >= 0; i--)
    {
        Serial.print((valeur >> i) & 1);
    }
}

// ---------------------------------------------------------------------------
// Scan complet du plugboard
// ---------------------------------------------------------------------------
void scannerPlugboard()
{
    bool connexionTrouvee = false;

    Serial.println();
    Serial.println("===== Scan plugboard =====");

    for (uint8_t i = 0; i < NB_LETTRES; i++)
    {
        // Sécurité : on remet tout en entrée avant chaque test
        toutEnEntree();
        delay(2);

        // La lettre i devient une sortie LOW
        activerLettreLow(i);

        // Lecture des deux expanders
        uint16_t etat0 = ioex0.read();
        uint16_t etat1 = ioex1.read();

        // Debug brut, utile au début
        /*
        Serial.print("Test ");
        Serial.print(lettres[i].lettre);
        Serial.print(" | 0x20 = ");
        afficherBinaire16(etat0);
        Serial.print(" | 0x21 = ");
        afficherBinaire16(etat1);
        Serial.println();
        */

        for (uint8_t j = 0; j < NB_LETTRES; j++)
        {
            // On ignore la lettre qu'on force nous-même à LOW
            if (j == i)
            {
                continue;
            }

            // Évite d'afficher deux fois A <-> B puis B <-> A
            if (j < i)
            {
                continue;
            }

            if (lettreEstLow(j, etat0, etat1))
            {
                Serial.print(lettres[i].lettre);
                Serial.print(" <-> ");
                Serial.println(lettres[j].lettre);

                connexionTrouvee = true;
            }
        }
    }

    toutEnEntree();

    if (!connexionTrouvee)
    {
        Serial.println("Aucune connexion detectee.");
    }

    Serial.println("==========================");
}

// ---------------------------------------------------------------------------
// Setup
// ---------------------------------------------------------------------------
void setup()
{
    Serial.begin(115200);
    delay(2000);

    Serial.println("=== ENIGMA - Test plugboard PCA9555 ===");

    // I2C sur Wire1 avec les pins du Pico
    Wire1.setSCL(15);
    Wire1.setSDA(14);
    Wire1.begin();

    // PCA9555 K7
    ioex0.attach(Wire1, 0x20);
    ioex0.polarity(PCA95x5::Polarity::ORIGINAL_ALL);
    ioex0.direction(PCA95x5::Direction::IN_ALL);

    // PCA9555 K8
    ioex1.attach(Wire1, 0x21);
    ioex1.polarity(PCA95x5::Polarity::ORIGINAL_ALL);
    ioex1.direction(PCA95x5::Direction::IN_ALL);

    toutEnEntree();

    Serial.println("Initialisation terminee.");
}

// ---------------------------------------------------------------------------
// Loop
// ---------------------------------------------------------------------------
void loop()
{
    scannerPlugboard();

    delay(1000);
}