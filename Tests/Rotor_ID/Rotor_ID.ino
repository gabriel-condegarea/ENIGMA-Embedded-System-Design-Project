#define EN_PIN 16
#define DIR_PIN 18
#define STEP_PIN 17
#define SWI_PIN 15



#define NUMSTEPS 31  //Microstep 2
#define PULSEDELAY 300


void setup() 
{ 
  Serial.begin(115200);
  while(!Serial);

  pinMode(SWI_PIN, INPUT_PULLUP);

  pinMode(EN_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  pinMode(STEP_PIN, OUTPUT);

  digitalWrite(STEP_PIN, LOW);

  digitalWrite(EN_PIN, LOW);  //enable torque
  Serial.println("Ready to test");
  
}

void loop() 
{
  swiNew = digitalRead(SWI_PIN);
  if(swiNew < swiOld)
  {
    Serial.println("Swi pressed");
  }
  swiOld = swiNew;


}

bool stepOneLetter(int ENPin, int DIRPIN, int STEPPin)  //in the end a rotor structure probably
{
  return(true);
}