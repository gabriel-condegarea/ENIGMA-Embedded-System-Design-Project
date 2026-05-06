/***************************************************
Simple example of reading the MCP3008 analog input channels and printing
them all out.

Author: Carter Nelson
License: Public Domain
****************************************************/

#include <Adafruit_MCP3008.h>

Adafruit_MCP3008 adc;

int count = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println("MCP3008 simple test.");

  while(!adc.begin(10,11,12,16))    //using soft SPI because I can't be arsed to make the hardware one work
  {                                 //and also it's not like we need to be fast anyways
    Serial.println("Failed to initialise");
    delay(1000);
  }
  // Software SPI (specify all, use any available digital)
  // (sck, mosi, miso, cs);
  //adc.begin(13, 11, 12, 10);
}

void loop() {
  for (int chan=0; chan<8; chan++) {
    Serial.print(adc.readADC(chan)); Serial.print("\t");
  }

  Serial.print("["); Serial.print(count); Serial.println("]");
  count++;
  
  delay(1000);
}
