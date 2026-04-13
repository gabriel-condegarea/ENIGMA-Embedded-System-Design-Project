#define EN_PIN 21
#define DIR_PIN 23
#define STEP_PIN 22//LED_BUILTIN


#define SENSOR 4



#define NUMSTEPS 31  //Microstep 2
#define PULSEDELAY 300


void setup() 
{ 
  Serial.begin(115200);
  while(!Serial);

  pinMode(SENSOR, INPUT);

  pinMode(EN_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  pinMode(STEP_PIN, OUTPUT);

  digitalWrite(STEP_PIN, LOW);

  digitalWrite(EN_PIN, LOW);  //enable torque
  Serial.println("Ready to test");
  
}

void loop() 
{
  for(int i = 0; i<NUMSTEPS; i++)
  {
    digitalWrite(STEP_PIN, 1);
    delayMicroseconds(PULSEDELAY);
    digitalWrite(STEP_PIN, 0);
  }
  Serial.println("Here");

  delay(1000);
}

bool stepOneLetter(int ENPin, int DIRPIN, int STEPPin)  //in the end a rotor structure probably
{
  return(true);
}