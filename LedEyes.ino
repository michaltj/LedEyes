//We always have to include the library
#include "LedControl.h"

/*
 Now we need a LedControl to work with.
 ***** These pin numbers will probably not work with your hardware *****
 pin 12 is connected to the DataIn 
 pin 11 is connected to the CLK 
 pin 10 is connected to LOAD 
 We have 2 MAX72XX.
 */
LedControl lc=LedControl(12,11,10,2);

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

// delays
#define DELAY_BLINK_MIDDLE 25
#define DELAY_BLINK_DOWN 15

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

}

void loop() { 



  displayEyes(0,0);
  delay(2000);
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



/*
  This method displays eyeball with pupul offset by X, Y values.
  Valid X and Y range is [-2,2]
  Both LED modules will show identical eyes
*/
void displayEyes(int offsetX, int offsetY) 
{
  
  // ensure offsetX is in valid range
  if (offsetX > 2) 
    offsetX = 2;
  else if (offsetX < -2) 
    offsetX = -2;
    
  // ensure offsetY is in valid range
  if (offsetY > 2) 
    offsetY = 2;
  else if (offsetY < -2)
    offsetY = -2;
   
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
  
  // display on LCD matrix, save to eyeCurrent
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
    delay(DELAY_BLINK_MIDDLE);
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
    delay(DELAY_BLINK_MIDDLE);
  }
}
