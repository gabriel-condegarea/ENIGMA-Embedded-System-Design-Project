#define EN_PIN 16
#define DIR_PIN 18
#define STEP_PIN 17
#define SWI_PIN 15

#define NUMSTEPS 400*2
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
  Serial.println("Torque enabled");
  

}

uint8_t direction = 0, torque = 1;
uint8_t swiOld = 1, swiNew = 1;

uint32_t stepCounter;

void loop() 
{
  swiNew = digitalRead(SWI_PIN);
  if(swiNew < swiOld)
  {
    Serial.print("Swi pressed. Torque = ");
    delay(50);
    torque ^= 1;
    digitalWrite(EN_PIN, !torque);
    Serial.println(torque);
    stepCounter = 0;
  }
  swiOld = swiNew;

  if(torque)
  {
    if(stepCounter < NUMSTEPS)
    { 
      digitalWrite(STEP_PIN, 1);
      delayMicroseconds(PULSEDELAY);
      digitalWrite(STEP_PIN, 0);
      delayMicroseconds(PULSEDELAY);
      stepCounter++;
    }
    else
    {
      stepCounter = 0;
      direction ^= 1; //toggle dir
      digitalWrite(DIR_PIN, direction);
      delay(1000);
    }  
  }

  

}
