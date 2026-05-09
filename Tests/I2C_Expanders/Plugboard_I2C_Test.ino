// Code de test du Plugboard (PCA9555)
// Teste la communication I2C avec les deux expanders GPIO
// Affiche l'état de toutes les entrées (26 touches du plugboard)

#include <PCA95x5.h>

PCA9555 ioex0; // Adresse I2C 0x20 (K7 - Port 0: Q A Y W E D C S X)
PCA9555 ioex1; // Adresse I2C 0x21 (K8 - Port 1: Z U J I K O L P M N H G F B T R V)

void setup()
{
    Serial.begin(115200);
    delay(2000);

    // Configure l'I2C sur Wire1 avec les broches du Pico
    Wire1.setSCL(15); // Broche SCL
    Wire1.setSDA(14); // Broche SDA
    Wire1.begin();

    // Initialise le premier PCA9555 (0x20)
    ioex0.attach(Wire1, 0x20);
    ioex0.polarity(PCA95x5::Polarity::ORIGINAL_ALL); // Pas d'inversion de polarité
    ioex0.direction(PCA95x5::Direction::IN_ALL);     // Toutes les broches en entrée

    // Initialise le deuxième PCA9555 (0x21)
    ioex1.attach(Wire1, 0x21);
    ioex1.polarity(PCA95x5::Polarity::ORIGINAL_ALL);
    ioex1.direction(PCA95x5::Direction::IN_ALL);
}

void loop()
{
    // Lit l'état des entrées du premier PCA9555
    Serial.print("0x20: ");
    Serial.println(ioex0.read(), BIN);

    // Lit l'état des entrées du deuxième PCA9555
    Serial.print("0x21: ");
    Serial.println(ioex1.read(), BIN);

    delay(1000); // Attendre 1 seconde avant la prochaine lecture
}