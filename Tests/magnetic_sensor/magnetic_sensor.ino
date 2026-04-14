#define SIG 12




int16_t val;
uint32_t zero_offset = 0;

void setup() 
{
  Serial.begin(115200);
  while(!Serial);


  pinMode(SIG, INPUT);

  //calibration
  for(int i = 0; i<100; i++)
  { 
    val = analogRead(SIG);
    zero_offset += val;
    //Serial.println(val);
    delay(10);
  }
  zero_offset /= 100;
  Serial.print("Calibration average: ");
  Serial.println(zero_offset);
}


void loop() 
{
  val = analogRead(SIG) - zero_offset;
  Serial.println(val);


  if(val > 200)
  {
    Serial.println("N detected");
  }
  else if(val < (-200))
  {
    Serial.println("S detected");
  }



  delay(200);
}
