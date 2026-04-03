// NeoPixel Ring simple sketch (c) 2013 Shae Erisson
// Released under the GPLv3 license to match the rest of the
// Adafruit NeoPixel library

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif


#define PIN        19 
#define NUMPIXELS 26 

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

#define DELAYVAL 100 // Time (in milliseconds) to pause between pixels

void setup() {

  Serial.begin(115200);


  pixels.begin();
}

uint16_t fade_var = 0;

void loop() 
{
  pixels.clear();

  for(int i=0; i<NUMPIXELS; i++) 
  { // For each pixel...
    uint32_t color = fade(fade_var+i, 128);
    pixels.setPixelColor(i, color);
    fade_var++;
    if(fade_var >= 768) fade_var = 0;
  }
  pixels.show();   // Send the updated pixel colors to the hardware.
  delay(DELAYVAL); // Pause before next pass through loop
}


uint32_t fade(uint16_t fadestep, uint8_t brightness)
{
  uint8_t red, green, blue;
  fadestep = fadestep % 768;  //clamp

  if(fadestep < 256)
  {
    red = fadestep;
    blue = 255-fadestep;
    green = 0;
  }
  else if (fadestep < 512)  //from 256 to 511
  { 
    green = fadestep - 256;
    red = 511-fadestep;
    blue = 0;
  }
  else
  {
    green = 767 - fadestep;
    blue = fadestep - 513;
    red = 0;
  }

  if(red == 0 && blue == 0 && green == 0)
  {
    Serial.print("error at step: ");
    Serial.println(fadestep);
    Serial.print("(");
    Serial.print(red);
    Serial.print(",");
    Serial.print(green);
    Serial.print(",");
    Serial.print(blue);
    Serial.println(")");
  }


  green = green*brightness/255;
  red = red*brightness/255;
  blue = blue*brightness/255;


  uint32_t color = pixels.Color(red, green, blue);
  return(color);
}



