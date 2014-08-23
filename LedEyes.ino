//We always have to include the LedControl library
#include "LedControl.h"

/*
 Create LetControl object, define pin connections
 We have 2 MAX72XX for eyes.
 */
#define PIN_EYES_DIN 12
#define PIN_EYES_CS 10
#define PIN_EYES_CLK 11
LedControl lc = LedControl(PIN_EYES_DIN, PIN_EYES_CLK, PIN_EYES_CS, 2);

// define eye ball without pupil  
byte eyeBall[8]={
  B00111100,
  B01111110,
  B11111111,
  B11111111,
  B11111111,
  B11111111,
  B01111110,
  B00111100
};

// stores current state of LEDs
byte eyeCurrent[8];
int currentX;
int currentY;

// min and max positions
#define MIN -2
#define MAX  2

// delays
#define DELAY_BLINK 25

/*
  Arduino setup
*/
void setup() {

  // MAX72XX is in power-saving mode on startup, we have to do a wakeup call
  lc.shutdown(0,false);
  lc.shutdown(1,false);

  // set the brightness to low
  lc.setIntensity(0,1);
  lc.setIntensity(1,1);

  // clear both modules
  lc.clearDisplay(0);
  lc.clearDisplay(1);
  
  // LED test
  // vertical line
  byte b = B10000000;
  for (int c=0; c<=7; c++)
  {
    for (int r=0; r<=7; r++)
    {
      lc.setRow(0, r, b);
      lc.setRow(1, r, b);
    }
    b = b >> 1;
    delay(50);
  }
  // full module
  b = B11111111;
  for (int r=0; r<=7; r++)
  {
    lc.setRow(0, r, b);
    lc.setRow(1, r, b);
  }
  delay(500);
  
   // clear both modules
  lc.clearDisplay(0);
  lc.clearDisplay(1);
  delay(1000);

  // random seed
  randomSeed(analogRead(0));
 
  // center eyes, crazy blink
  displayEyes(0, 0);
  delay(2000);
  blinkEyes(true, false);
  blinkEyes(false, true);
  delay(1000);
}

/*
  Arduino loop
*/
void loop() 
{ 
  // move to random position, wait random time
  moveEyes(random(MIN, MAX + 1), random(MIN, MAX + 1), 50);
  delay(random(2, 7) * 500);
  
  // blink time?
  if (random(0,5) == 0)
  {
    delay(500);
    blinkEyes();
    delay(500);
  }
}

/*
  This method corrects provided coordinate value
*/
int getValidValue(int value)
{
  if (value > MAX)
    return MAX;
  else if (value < MIN)
    return MIN;
  else
    return value;
}

/*
  This method displays eyeball with pupil offset by X, Y values from center position.
  Valid X and Y range is [MIN,MAX]
  Both LED modules will show identical eyes
*/
void displayEyes(int offsetX, int offsetY) 
{
  // ensure offsets are  in valid ranges
  offsetX = getValidValue(offsetX);
  offsetY = getValidValue(offsetY);
  
  // calculate indexes for pupil rows (perform offset Y)
  int row1 = 3 - offsetY;
  int row2 = 4 - offsetY;

  // pupil row
  byte pupilRow = B11100111;

  // perform offset X
  // bit shift and fill in new bit with 1 
  if (offsetX > 0) {
    for (int i=1; i<=offsetX; i++)
    {
      pupilRow = pupilRow >> 1;
      pupilRow = pupilRow | B10000000;
    }
  }
  else if (offsetX < 0) {
    for (int i=-1; i>=offsetX; i--)
    {
      pupilRow = pupilRow << 1;
      pupilRow = pupilRow | B1;
    }
  }

  // pupil row cannot have 1s where eyeBall has 0s
  byte pupilRow1 = pupilRow & eyeBall[row1];
  byte pupilRow2 = pupilRow & eyeBall[row2];
  
  // display on LCD matrix, update to eyeCurrent
  for(int r=0; r<8; r++)
  {
    if (r == row1)
    {
      lc.setRow(0, r, pupilRow1);
      lc.setRow(1, r, pupilRow1);
      eyeCurrent[r] = pupilRow1;
    }
    else if (r == row2)
    {
      lc.setRow(0, r, pupilRow2);
      lc.setRow(1, r, pupilRow2);
      eyeCurrent[r] = pupilRow2;
    }
    else
    {
      lc.setRow(0, r, eyeBall[r]);
      lc.setRow(1, r, eyeBall[r]);
      eyeCurrent[r] = eyeBall[r];
    }
  }
  
  // update current X and Y
  currentX = offsetX;
  currentY = offsetY;
}


/*
  This method blinks both eyes
*/
void blinkEyes()
{
  blinkEyes(true, true);
}


/*
  This method blinks eyes as per provided params
*/
void blinkEyes(boolean blinkLeft, boolean blinkRight)
{
  // blink?
  if (!blinkLeft && !blinkRight)
    return;
  
  // close eyelids
  for (int i=0; i<=3; i++)
  {
    if (blinkLeft)
    {
      lc.setRow(0, i, 0);
      lc.setRow(0, 7-i, 0);
    }
    if (blinkRight)
    {
      lc.setRow(1, i, 0);
      lc.setRow(1, 7-i, 0);
    }
    delay(DELAY_BLINK);
  }
  
  // open eyelids
  for (int i=3; i>=0; i--) 
  {
    if (blinkLeft)
    {
      lc.setRow(0, i, eyeCurrent[i]);
      lc.setRow(0, 7-i, eyeCurrent[7-i]);
    }
    if (blinkRight)
    {
      lc.setRow(1, i, eyeCurrent[i]);
      lc.setRow(1, 7-i, eyeCurrent[7-i]);
    }
    delay(DELAY_BLINK);
  }
}

/*
  This method moves both eyes from current position to new position
*/
void moveEyes(int newX, int newY, int stepDelay)
{
  // set current position as start position
  int startX = currentX;
  int startY = currentY;
  
  // fix invalid new X Y values
  newX = getValidValue(newX);
  newY = getValidValue(newY);
   
  // eval how many steps needed to get to new position
  int stepsX = abs(currentX - newX);
  int stepsY = abs(currentY - newY);
  
  // need at least 1 X or 1 Y step
  if ((stepsX == 0) && (stepsY ==0))
    return;

  // eval direction of movement, # of steps, change per X Y step, perform move
  int dirX = (newX >= currentX) ? 1 : -1;
  int dirY = (newY >= currentY) ? 1 : -1;
  int steps = (stepsX > stepsY) ? stepsX : stepsY;
  float changeX = stepsX / steps;
  float changeY = stepsY / steps;
  for (int i=1; i<=steps; i++)
  {
    currentX = startX + round(changeX * i * dirX);
    currentY = startY + round(changeY * i * dirY);
    displayEyes(currentX, currentY);
    delay(stepDelay);
  }
}

/*
  Test display and blink 
*/
void testDisplayBlink()
{
  blinkEyes(true, false);
  blinkEyes(false, true);
  delay(1000);
  displayEyes(1,0); delay(100);
  displayEyes(2,0); delay(1000);
  displayEyes(1,0); delay(50);
  displayEyes(0,0); delay(50);
  displayEyes(-1,0); delay(50);
  displayEyes(-2,0); delay(1000);
  blinkEyes(); delay(1000);
  displayEyes(-1,0); delay(100);
  displayEyes(0,0); delay(1000);
  displayEyes(0,1); delay(100);
  displayEyes(0,2); delay(1000);
  blinkEyes(); delay(1000);
  displayEyes(0,1); delay(100);
  displayEyes(0,0); delay(2000);
  blinkEyes(); delay(500); blinkEyes(); delay(1000);
  displayEyes(1,1); delay(3000);
  displayEyes(0,0); delay(50);
  displayEyes(-1,-1); delay(2000);
}
