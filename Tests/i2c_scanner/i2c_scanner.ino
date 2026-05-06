/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/raspberry-pi-pico-i2c-scanner-arduino/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*********/

#include <Wire.h>
 
void setup() 
{
  Wire1.setSCL(15);
  Wire1.setSDA(14);
  Wire1.begin();
  Serial.begin(115200);
  Serial.println("\nI2C Scanner");
}
 
void loop() {
  byte error, address;
  int nDevices;
  Serial.println("Scanning...");
  nDevices = 0;

  for(address = 0x20; address < 0x22; address++) 
  {
    //address = 0x21;
    Wire1.beginTransmission(address);
    error = Wire1.endTransmission();
    Serial.println(error);
    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address<16) {

        
        Serial.print("0");
      }
      Serial.println(address,HEX);
      nDevices++;
    }
    else if (error==4) {
      Serial.print("Unknow error at address 0x");
      if (address<16) {
        Serial.print("0");
      }
      Serial.println(address,HEX);
    }    
  }
  if (nDevices == 0) {
    Serial.println("No I2C devices found\n");
  }
  else {
    Serial.println("done\n");
  }
  delay(1000);          
}