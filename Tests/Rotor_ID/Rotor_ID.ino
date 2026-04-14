#define EN_PIN 21
#define DIR_PIN 23
#define STEP_PIN 22//LED_BUILTIN

#define SENSOR 15



#define NUMSTEPS 31  //Microstep 2
#define PULSEDELAY 500


void setup() 
{ 
  Serial.begin(115200);
  while(!Serial);

  pinMode(SENSOR, INPUT_PULLUP);

  pinMode(EN_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  pinMode(STEP_PIN, OUTPUT);

  digitalWrite(STEP_PIN, LOW);

  digitalWrite(EN_PIN, LOW);  //enable torque
  Serial.println("Ready to test");
  
} 

uint8_t letter = 0;
uint16_t data = 0;

void loop() 
{ 
  data = analogRead(SENSOR);
  Serial.println(data);

  for(int i = 0; i<NUMSTEPS; i++)
  {
    digitalWrite(STEP_PIN, 1);
    delay(2);
    digitalWrite(STEP_PIN, 0);
  }


  delay(1000);

}

bool stepOneLetter(int ENPin, int DIRPIN, int STEPPin)  //in the end a rotor structure probably
{
  return(true);
}