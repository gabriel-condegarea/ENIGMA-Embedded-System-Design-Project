/*
But di fichier: Détecter les ports attendus pour les expanders (0x20 et 0x21), lire et écrire des pins
*/
#include <Wire.h>

void setup()
{
    Serial.begin(115200);
    delay(2000);

    Wire.begin(); // SDA/SCL par défaut du Pico selon ton core Arduino

    Serial.println("Scan I2C...");
}

void loop()
{
    byte count = 0;

    for (byte address = 1; address < 127; address++)
    {
        Wire.beginTransmission(address);
        byte error = Wire.endTransmission();

        if (error == 0)
        {
            Serial.print("Device found at 0x");
            if (address < 16)
                Serial.print("0");
            Serial.println(address, HEX);
            count++;
        }
    }

    if (count == 0)
    {
        Serial.println("Aucun périphérique I2C trouvé");
    }
    else
    {
        Serial.print("Nombre de périphériques trouvés : ");
        Serial.println(count);
    }

    Serial.println("------------------");
    delay(2000);
}