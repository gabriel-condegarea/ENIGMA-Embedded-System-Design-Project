#define EN_PIN 6
#define DIR_PIN 5
#define STEP_PIN 4

#define SWI 15

#define MAG_THR 15

const uint8_t sensors[2] = {27,26}; //ADC pin 
// [0] = rotor, [1] = lettre

typedef struct RotorHardware_t
{
  uint8_t sensorOffset; //offset from sensor to displayed lettre
  uint8_t enPin;        //general enable pin
  uint8_t dirPins[3];   //direction pins
  uint8_t directions[3];//turning direction
  uint8_t stepPins[3];  //step pins
  
}RotorHardware_t;


struct Rotor 
{
    char            name[5];        //name in roman numerals
    int             offset;         //position offset (from A)
    int             stellung;       //ring position from wiring
    int             turnnext;       
    const char      *cipher;
    const char      *turnover;
    const char      *notch;
    //probably need to add: real position
};




void setup() 
{ 
  Serial.begin(115200);
  while(!Serial);

  pinMode(sensors[0], INPUT);
  pinMode(sensors[1], INPUT);

  pinMode(EN_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  pinMode(STEP_PIN, OUTPUT);

  pinMode(SWI, INPUT_PULLUP);

  //
  digitalWrite(STEP_PIN, LOW);
  digitalWrite(DIR_PIN, 0);

  digitalWrite(EN_PIN, LOW);  //enable torque
  Serial.println("Ready to test");
  
} 

uint8_t letter = 0;
uint16_t data = 0;

uint8_t swiNow = 1, swiOld = 1;

uint8_t torque = 1;
uint16_t timer = 0;

char alphabet[27] = {"ABCDEFGHIJKLMNOPQRSTUVWXYZ"};

void loop() 
{ 
  swiNow = digitalRead(SWI);
  if(swiNow < swiOld)
  {
    torque ^= 1;
    Serial.print("Torque= ");
    Serial.println(torque);
    if(torque) digitalWrite(EN_PIN, 0);
    else digitalWrite(EN_PIN, 1); //disable torque
  }
  swiOld = swiNow;

  if(torque)
  {
    if( (timer%5000) == 0)
    { 
      rotorID();
    }
  }

  timer+=10;
  if(timer >= 65500) timer = 0;
  delay(10);
}



//rotor identification routine
bool rotorID(void)
{
  //future return vars
  uint8_t textOffset = 0;
  uint8_t startPos = 0;
  
  //local vars
  uint8_t l = 0, i = 0;
  const int numsteps = 40;
  int16_t magnetData[2][26] = {0};
  int16_t lap_avg[2] = {0};
  uint8_t firstMagnetIdx[2] = {0,0};
  uint8_t numMagnets[2] = {0};
  uint8_t rotorNum = 0;

  const int sensorOffset = 9;


  
  //offset half a letter (start of next letter)
  digitalWrite(DIR_PIN, 0);
  for(i = 0; i<(numsteps/2); i++)
  {
    digitalWrite(STEP_PIN, 1);
    delay(5);
    digitalWrite(STEP_PIN, 0);
  }


  for(l = 0; l<26; l++) //move one lap
  {
    for(i = 0; i<numsteps; i++) //step one letter
    {
      digitalWrite(STEP_PIN, 1);
      
      magnetData[0][l] += analogRead(sensors[0]);
      magnetData[1][l] += analogRead(sensors[1]);

      delay(4);
      digitalWrite(STEP_PIN, 0);
    }
    magnetData[0][l] /= numsteps; //compute average of letter
    magnetData[1][l] /= numsteps; //compute average of letter

    lap_avg[0] += magnetData[0][l]; //add to turn average
    lap_avg[1] += magnetData[1][l]; //add to turn average

    delay(50);
  }
  
  //move back half a letter
  digitalWrite(DIR_PIN, 1);
  for(i = 0; i<(numsteps/2); i++)
  { 
    digitalWrite(STEP_PIN, 1);
    delay(5);
    digitalWrite(STEP_PIN, 0);
  }

  lap_avg[0]/= 26;  //calculate averages for the whole turn
  lap_avg[1]/= 26;  

  //parse data
  for(l = 0; l<26; l++) 
  {
    //rotor magnet
    magnetData[0][l] -= lap_avg[0];
    if(magnetData[0][l]<MAG_THR && magnetData[0][l]>(-MAG_THR)) magnetData[0][l] = 0;
    if(magnetData[0][l] >= MAG_THR) magnetData[0][l] = 1;
    if(magnetData[0][l] <= -MAG_THR) magnetData[0][l] = -1;

    //magnet detected
    if(magnetData[0][l] != 0)
    {
      if(numMagnets[0] == 0)
      {
        firstMagnetIdx[0] = l;
      } 
      //write to rotorNum
      if(magnetData[0][l] == 1)
      {
        rotorNum |= (0x01<< numMagnets[0]); //write 1
      }
  
      numMagnets[0]++;

      if(numMagnets[0]>3) return(false); 
    }

    magnetData[1][l] -= lap_avg[1];
    if( magnetData[1][l]<MAG_THR && magnetData[1][l]>(-MAG_THR)) magnetData[1][l] = 0;
    if( magnetData[1][l] >= MAG_THR) magnetData[1][l] = 1;
    if( magnetData[1][l] <= -MAG_THR) magnetData[1][l] = -1;

    //magnet detected
    if(magnetData[1][l] != 0)
    {
      firstMagnetIdx[1] = l;
      numMagnets[1]++;
      if(numMagnets[1]>1) return(false); 
    }

    //Serial.print(magnetData[0][l]);
    //Serial.print(", ");
    // Serial.print(magnetData[1][l]);
    // Serial.print(";");
  }



  //calculate offsets
  textOffset = (firstMagnetIdx[0] - firstMagnetIdx[1]) >= 0 ?  (firstMagnetIdx[0] - firstMagnetIdx[1])%26 : (firstMagnetIdx[0] - firstMagnetIdx[1]) + 26;
  startPos = (sensorOffset - firstMagnetIdx[0] + textOffset) >= 0 ? (sensorOffset - firstMagnetIdx[0] + textOffset)%26 : (sensorOffset - firstMagnetIdx[0] + textOffset)+26;


  if(numMagnets[0] != 3 || numMagnets[1] != 1) 
  {
    Serial.println("Not enough magnets found");
    return(false); //not enough magnets found
  }

  //Print data
  //convert to letter 
  char letter[2] = {0};
  sprintf(letter, "%c", startPos+'A');

  Serial.println("---------------");
  Serial.println("Identification finished!");
  Serial.print("Rotor number: ");
  Serial.println(rotorNum, BIN);
  Serial.print("Start Letter: ");
  // Serial.print(startPos);
  // Serial.print(",");
  Serial.println(letter);
  // Serial.print("Stellung: ");
  // Serial.println(textOffset);
  // Serial.print("First magnet found at: ");
  // Serial.print(firstMagnetIdx[0]);
  // Serial.print(", ");
  // Serial.println(firstMagnetIdx[1]);



  return(true);
}

/*bool stepOneLetter(uint8_t STEPPin)  //in the end a rotor structure probably
{
  const uint8_t numSteps = 40;

  for(int i = 0; i<numSteps; i++)
  {
    digitalWrite(STEP_PIN, 1);
    delay(4);
    digitalWrite(STEP_PIN, 0);
  }

  return(true);
}*/