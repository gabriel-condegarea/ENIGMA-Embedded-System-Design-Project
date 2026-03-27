#define EN_PIN 16
#define DIR_PIN 18
#define STEP_PIN 17

void setup() 
{ 
  Serial.begin(115200);

  pinMode(EN_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  pinMode(STEP_PIN, OUTPUT);

  digitalWrite(EN_PIN, LOW);  //enable torque
  digitalWrite(STEP_PIN, LOW);  //enable torque

}

uint8_t direction = 0;
void loop() 
{
  digitalWrite(DIR_PIN, direction);
  Serial.println("Walking");
  for(int i = 0; i<10; i++)
  {
    digitalWrite(STEP_PIN, 1);
    delay(10);
    digitalWrite(STEP_PIN, 0);
  }
  direction ^= 1; //toggle dir
  delay(1000);

}
