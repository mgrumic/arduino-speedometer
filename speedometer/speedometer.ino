///* I2C LCD with Arduino example code. More info: https://www.makerguides.com */
//// Include the libraries:
//// LiquidCrystal_I2C.h: https://github.com/johnrickman/LiquidCrystal_I2C
//#include <Wire.h> // Library for I2C communication
//#include <LiquidCrystal_I2C.h> // Library for LCD
//// Wiring: SDA pin is connected to A4 and SCL pin to A5.
//// Connect to LCD via I2C, default address 0x3f (A0-A2 not jumpered)
//LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x3f, 20, 4); // Change to (0x27,16,2) for 16x2 LCD.
//
//void setup() {
//  // Initiate the LCD:
//  lcd.init();
//  lcd.backlight();
//}
//
//void loop() {
//  // Print 'Hello World!' on the first line of the LCD:
//  lcd.setCursor(0, 0); // Set the cursor on the first column and first row.
//  lcd.print("Hello World!"); // Print the string "Hello World!"
//  lcd.setCursor(2, 1); //Set the cursor on the third column and the second row (counting starts at 0!).
//  lcd.print("LCD tutorial");
//}

#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x3f, 20, 4);

#define DISTANCE (890.0f)
#define INTERRUPT_TIME (1000)
int magnetPin = 2;  // LED connected to digital pin D2 (2)
int resetPin = 3;  // LED connected to digital pin D2 (2)
int magnetState = HIGH;
int magnetStatePrevious = HIGH;

unsigned long totalMilliseconds = 0;
unsigned long cmillis = 0;
unsigned long lmillis = 0;
float curspeed = 0.0f;
volatile unsigned long cycles = 0;

char topSpeedString[21] = "Top speed: ";
char currentSpeedString[21] = "Speed: ";
char distanceString[21] = "Distance: ";
float distance = 0.0f;
float topspeed = 0.0f;
bool dirtyFlag = false;

unsigned int timerCounter = 0;

void CalculateSpeed()
{
  float timediff = cmillis - lmillis;
  if( timediff == 0)
  {
    curspeed = 0;
    return;
  }
  else if (timediff < 100)
  {
    return;
  }
  else if( timediff > 4000)
  {
    return;
  }

  curspeed = ( DISTANCE / (cmillis - lmillis) ) * 3.6;
  distance += DISTANCE;
}

void UpdateState()
{
  char speedStr[6] = "";
  char topSpeedStr[6] = "";
  char distanceStr[6] = "";
  if( curspeed > topspeed )
  {
    topspeed = curspeed;
  }
  
  dtostrf(topspeed, 4, 2, topSpeedStr);
  dtostrf(curspeed, 4, 2, speedStr);
  dtostrf(distance/1000, 5, 1, distanceStr);
//  memset(distanceString, 0x0, 21);
  memset(topSpeedString, 0x0, 21);
  memset(currentSpeedString, 0x0, 21);
  sprintf(distanceString, "Distance: %9sm", distanceStr);
  sprintf(topSpeedString, "Top speed: %5skm/h", topSpeedStr);
  sprintf(currentSpeedString, "Speed: %9skm/h", speedStr );
//  sprintf(speedStr)
//  Serial.println("%ld", cycles);
  
}

int curspeed_same = 0;
float curspeed_previous = 0.0f;
// Interrupt is called once a millisecond, 
SIGNAL(TIMER0_COMPA_vect) 
{
  timerCounter++;

  if( timerCounter == INTERRUPT_TIME )
  {
    timerCounter = 0;
    dirtyFlag = true;

    if( curspeed_previous == curspeed )
    {
      curspeed_same += 1;
    }
    else
    {
      curspeed_same = 0;
    }
  
    if( curspeed_same == 3 )
    {
      curspeed = 0.0f;
      curspeed_same = 0;
    }
    
    curspeed_previous = curspeed;
  }

}

void UpdateCycleCounter()
{
  lmillis = cmillis;
  cmillis = millis();
  CalculateSpeed();
}


void UpdateLCD()
{
  lcd.setCursor(0,0);
  lcd.print( topSpeedString );
  lcd.setCursor(0, 1);
  lcd.print( currentSpeedString );
  lcd.setCursor(0, 2);
  lcd.print( distanceString );
  lcd.setCursor(0, 3);
  lcd.print("Powered by Grummy");
}

void setup() {
  lcd.init();
  lcd.backlight();
  Serial.begin( 9600 );
//  pinMode(magnetPin, INPUT);
  UpdateLCD();
  OCR0A = 0xAF;
  TIMSK0 |= _BV(OCIE0A);
  pinMode(magnetPin, INPUT_PULLUP);
  pinMode(resetPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(magnetPin), UpdateCycleCounter, RISING);
//  delay(1000);
  curspeed = 0.0f;
  topspeed = 0.0f;
  distance = 0.0f;
}

int resetState = HIGH;

void loop()
{  
  if( dirtyFlag )
  {
    UpdateState();
    UpdateLCD();
    dirtyFlag = false;
  }

  int resetStateCur = digitalRead(resetPin);

  if( resetState == LOW && resetStateCur == HIGH )
  {
    UpdateState();
    UpdateLCD();
    curspeed = 0.0f;
    topspeed = 0.0f;
    distance = 0.0f;    
  }
  resetState = resetStateCur;
}
