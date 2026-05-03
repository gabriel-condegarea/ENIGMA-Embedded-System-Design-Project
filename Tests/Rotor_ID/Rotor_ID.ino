#include "pinout.h"

#define NUMROTORS 1
#define ALLROTORS for(r = 0; r < rcfg->numRotors; r++)



#define SWI 15

#define MAG_THR 15

const uint8_t sensors[3][2] = {{27,26}, {27,26}, {27,26}}; //ADC pin   //TODO replace with calls to the MCP3008
// [0] = rotor, [1] = lettre

typedef struct RotorHardware_t
{
  uint8_t numRotors;  //number of rotors installed
  uint8_t sensorOffset; //offset from sensor to displayed lettre
  uint8_t numSteps;   //number of steps for a letter rotation
  uint8_t enPin;        //general enable pin
  uint8_t dirPins[3];   //direction pins
  uint8_t directions[3];//turning direction
  uint8_t stepPins[3];  //step pins
  uint8_t ident[3][3];  //{rotor#, offset, stellung}
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


RotorHardware_t rotorConfig = {.numRotors= NUMROTORS, .sensorOffset = 9, .numSteps = 40,.enPin = MOTOR_EN_PIN, .dirPins={DIR0_PIN, DIR1_PIN, DIR2_PIN}, .directions={0,0,0}, .stepPins={STEP0_PIN,STEP1_PIN,STEP2_PIN}};


void setup() 
{ 
  Serial.begin(115200);
  while(!Serial);

  pinMode(sensors[0][0], INPUT);
  pinMode(sensors[0][1], INPUT);

  configRotorsPins(&rotorConfig);

  pinMode(SWI, INPUT_PULLUP);  


  digitalWrite(rotorConfig.enPin, 0); //enable torque
} 

//uint16_t data = 0;

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
    if(torque) digitalWrite(MOTOR_EN_PIN, 0);
    else digitalWrite(MOTOR_EN_PIN, 1); //disable torque
  }
  swiOld = swiNow;

  if(torque)
  {
    if( (timer%5000) == 0)  //every 5 seconds
    { 
      rotorID(&rotorConfig);
    }
  }

  timer+=10;
  if(timer >= 65000) timer = 0;
  delay(10);
}


bool configRotorsPins(RotorHardware_t* rcfg)
{
  if(rcfg == NULL) return false;

  uint8_t r = 0;

  pinMode(rcfg->enPin, OUTPUT);

  ALLROTORS
  {
    pinMode(rcfg->dirPins[r], OUTPUT);
    pinMode(rcfg->stepPins[r], OUTPUT);

    digitalWrite(rcfg->stepPins[r], 0);  
    digitalWrite(rcfg->dirPins[r], rcfg->directions[r]);  //set normal direction
  }

  return(true);
}

//rotor identification routine
bool rotorID(RotorHardware_t* rcfg)
{
  //
  if(rcfg == NULL) return(false);
  
  //local vars
  uint8_t l = 0, i = 0, r = 0;
  int16_t magnetData[NUMROTORS][2][26] = {0}; //raw sensor data
  int16_t lap_avg[NUMROTORS][2] = {0};        //average reading over a lap
  uint8_t firstMagnetIdx[NUMROTORS][2] = {0}; //index of first magnet
  uint8_t numMagnets[NUMROTORS][2] = {0};     //number of magnets found
  uint8_t rotorNum[NUMROTORS] = {0};          //read number of rotor

  uint8_t corrFirstMagnet[NUMROTORS] = {0};     //corrected index for edge cases
  uint8_t stellungCorrector[NUMROTORS] = {0}; //corrected stellung for edge cases
  uint8_t digit = 0;



  
  //Offset half a letter
  //set direction
  ALLROTORS digitalWrite(rcfg->dirPins[r], rcfg->directions[r]);

  for(i = 0; i<(rcfg->numSteps/2); i++) //advance half a letter
  {
    ALLROTORS digitalWrite(rcfg->stepPins[r], 1);
    delay(5);
    ALLROTORS digitalWrite(rcfg->stepPins[r], 0);
  }


  for(l = 0; l<26; l++) //move one lap
  {
    for(i = 0; i<rcfg->numSteps; i++) //step one letter
    {
      ALLROTORS
      {
        digitalWrite(rcfg->stepPins[r], 1);
        magnetData[r][0][l] += analogRead(sensors[r][0]);
        magnetData[r][1][l] += analogRead(sensors[r][1]);
      } 
      delay(4);
      ALLROTORS digitalWrite(rcfg->stepPins[r], 0);
    }

    ALLROTORS
    {
      magnetData[r][0][l] /= rcfg->numSteps; //compute average of letter
      magnetData[r][1][l] /= rcfg->numSteps; 

      lap_avg[r][0] += magnetData[r][0][l]; //add to turn average
      lap_avg[r][1] += magnetData[r][1][l]; 
    }

    delay(50);
  }
  
  //Offset back half a letter
  //set direction
  ALLROTORS digitalWrite(rcfg->dirPins[r], !(rcfg->directions[r]));

  for(i = 0; i<(rcfg->numSteps/2); i++) //advance half a letter
  {
    ALLROTORS digitalWrite(rcfg->stepPins[r], 1);
    delay(5);
    ALLROTORS digitalWrite(rcfg->stepPins[r], 0);
  }

  //set direction correctly again
  ALLROTORS digitalWrite(rcfg->dirPins[r], (rcfg->directions[r]));

  ALLROTORS
  {
    lap_avg[r][0]/= 26;  //calculate averages for the whole turn
    lap_avg[r][1]/= 26;  
  }


  //parse data
  for(l = 0; l<26; l++) 
  {
    ALLROTORS
    {
      //rotor magnet
      magnetData[r][0][l] -= lap_avg[r][0]; //normalise to average
      if(magnetData[r][0][l]<MAG_THR && magnetData[r][0][l]>(-MAG_THR)) magnetData[r][0][l] = 0;
      if(magnetData[r][0][l] >= MAG_THR) magnetData[r][0][l] = 1;
      if(magnetData[r][0][l] <= -MAG_THR) magnetData[r][0][l] = -1;

      //magnet detected
      if(magnetData[r][0][l] != 0)
      {
        if(numMagnets[r][0] == 0) //if it's the first
        {
          firstMagnetIdx[r][0] = l;
        } 
    
        numMagnets[r][0]++;

        if(numMagnets[r][0]>3) return(false); 
      }

      //text magnet
      magnetData[r][1][l] -= lap_avg[r][1];
      if( magnetData[r][1][l]<MAG_THR && magnetData[r][1][l]>(-MAG_THR)) magnetData[r][1][l] = 0;
      if( magnetData[r][1][l] >= MAG_THR) magnetData[r][1][l] = 1;
      if( magnetData[r][1][l] <= -MAG_THR) magnetData[r][1][l] = -1;

      //magnet detected
      if(magnetData[r][1][l] != 0)
      {
        firstMagnetIdx[r][1] = l;
        numMagnets[r][1]++;
        if(numMagnets[r][1]>1) return(false); 
      }  


      // Serial.print(magnetData[r][1][l]);
      // Serial.print(",");    
    }

  }
  // Serial.println(";");


  Serial.println("---------------");
  //calculate offsets and store to rotorConfig
  ALLROTORS
  {
    //error detection and correction
    corrFirstMagnet[r] = firstMagnetIdx[r][0]; //normal case
    if(firstMagnetIdx[r][0] == 0) //started on a magnet
    {
      if(magnetData[r][0][24] != 0)  //it was the 2nd magnet
      {
        stellungCorrector[r] = 2;
        corrFirstMagnet[r] = 24;
      }
      else if(magnetData[r][0][25] != 0)  //it was the 1st magnet
      {
        stellungCorrector[r] = 1;
        corrFirstMagnet[r] = 25;
      }  
    }

    //parse rotor number
    for(i = corrFirstMagnet[r]; i<(corrFirstMagnet[r]+3); i++)
    {
      digit = magnetData[r][0][i%26] == -1 ? 0 : 1;
      rotorNum[r] |= (digit << (i-corrFirstMagnet[r]));
    }
   

    rcfg->ident[r][0] = rotorNum[r];
    rcfg->ident[r][2] = (26 + firstMagnetIdx[r][0] - firstMagnetIdx[r][1] - stellungCorrector[r]) % 26; 
    rcfg->ident[r][1] = (26 + rcfg->sensorOffset - firstMagnetIdx[r][0] + rcfg->ident[r][2] + stellungCorrector[r]) % 26;

  
    if(numMagnets[r][0] != 3 || numMagnets[r][1] != 1) 
    {
      Serial.print("Rotor ");
      Serial.print(r);
      Serial.print(": ");
      Serial.println("Not enough magnets found");
      Serial.print(numMagnets[r][0]);
      Serial.print(", ");
      Serial.println(numMagnets[r][1]);
      return(false); //not enough magnets found
    }
  }

  //Print data
  //convert to letter 
  char letter[2] = {0};
  sprintf(letter, "%c", rcfg->ident[0][1]+'A');


  Serial.println("Identification finished!");
  Serial.print("Rotor number: ");
  Serial.println(rcfg->ident[0][0], BIN);
  Serial.print("Start Letter: ");
  Serial.println(letter);
  Serial.print("Stellung: ");
  Serial.println(rcfg->ident[0][2]);
  // Serial.print("First magnet found at: ");
  // Serial.print(firstMagnetIdx[0]);
  // Serial.print(", ");
  // Serial.println(firstMagnetIdx[1]);



  return(true);
}
