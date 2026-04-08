#define SIG A0

void setup() 
{
  Serial.begin(115200);
  while(!Serial);


  pinMode(SIG, INPUT);

}

uint8_t val;
void loop() 
{
  val = digitalRead(SIG);
  Serial.println(val);

  delay(200);
}
