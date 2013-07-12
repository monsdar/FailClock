 
 /**
  * Controls the 7 segment display and provides a serial interface to perform certain functions
  * http://learn.adafruit.com/adafruit-led-backpack/1-2-inch-7-segment-backpack
  *
  *  --- Protocol ---
  * Note: The protocol is very naive and in no way robust.
  *       Do not try to send malformed data, you'll regret it ;)
  *
  * Input:
  * T012345
  *   - Sets the amount of seconds gone by since midnight.
  *     The code simply uses a second-counter to calc when the daycounter needs
  *     to be increased. You will simply give the amount of seconds here
  *     after you started the device.
  *     Example: T86400 -> Last midnight was 24 hours ago (that makes no sense, yeah!)
  *     Example: T00600 -> Last midnight is 10 minutes ago (that makes sense!)
  *     Returns: Nothing (probably some debugging logs, just don't use the data programatically)
  * R0123
  *   - Resets the counter to the given number
  *     Must be R + 4 digits long
  *     Example: R0000 (resets to 0)
  *     Returns: 0123 -> Previously shown number
  */ 
 
//Arduino libraries
#include <Wire.h> //I2C communication

//Adafruit libraries to control the 7 segment display
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"

//variables
Adafruit_7segment matrix = Adafruit_7segment(); //needed to access the display
unsigned int counter = 0; //this is the current counter, that's what the display will show

//the amount of seconds gone by since last update of the display
//a long is needed because we're dealing with values up to 86400 seconds (1 day)
unsigned long seconds = 0;

void setup() {
  Serial.begin(9600);
  Serial.println("7Seg Counter");
  Serial.println("Usage:"); 
  Serial.println("T012345 - Set the seconds since last midnight");
  Serial.println("R0123 - Reset the display to the given days");
  Serial.println("For a more verbose explanation take a look at the docs in the source");

  //Init the 7seg
  matrix.begin(0x70);
  displayNumber(counter);
}

void loop()
{  
  //check if there is data available
  if (Serial.available() > 0)
  {
    char input = Serial.read();
    if(input == 'T')
    {
      updateTime(); 
    }
    else if(input == 'R')
    {
      unsigned int lastValue = updateDisplay();
      Serial.println(lastValue, DEC); //answer with the previous counter
    }
    else
    {
      //do nothing here... just ignore the input
      while(Serial.available())
      {
        char wasted = Serial.read();
      }
    }
  }
    
  //check if it is midnight. If so, increase the counter
  seconds++;
  if( seconds >= 86400L)
  {
    increaseCounter();
    displayNumber(counter);
    seconds = 0;
  }
  
  //sleep for a second (let's give the arduino a break...)
  delay(1000);
}

void increaseCounter()
{
  counter++;
  if(counter == 9999)
  {
    counter = 0;
  }
}

void displayNumber(unsigned int number)
{
  //print the result to the 7seg
  const unsigned char NUMDIGITS = 4;
  unsigned char singleDigits[NUMDIGITS];
  for(unsigned char index = NUMDIGITS; index-- > 0;) //reverse iterate
  {
    singleDigits[index] = number % 10;
    number = number/10;
  }
  
  matrix.writeDigitNum(0, singleDigits[0]); //first number  (index 0)
  matrix.writeDigitNum(1, singleDigits[1]); //second number (index 1)
  matrix.writeDigitNum(3, singleDigits[2]); //third number  (index 3)
  matrix.writeDigitNum(4, singleDigits[3]); //fourth number (index 4)
  matrix.writeDisplay();
}

void updateTime()
{
  seconds = 0;
  seconds = seconds + ( (unsigned long)charToNumber(Serial.read()) * 10000L);
  seconds = seconds + ( (unsigned long)charToNumber(Serial.read()) * 1000L);
  seconds = seconds + ( (unsigned long)charToNumber(Serial.read()) * 100L);
  seconds = seconds + ( (unsigned long)charToNumber(Serial.read()) * 10L);
  seconds = seconds + ( (unsigned long)charToNumber(Serial.read()) );
  Serial.print("Current Time: ");
  Serial.println( seconds );
}

unsigned int updateDisplay()
{
  //read the new count from serial
  unsigned int newCount = 0;
  if(Serial.available())
  {
    newCount = newCount + ( charToNumber(Serial.read()) * 1000);
    newCount = newCount + ( charToNumber(Serial.read()) * 100);
    newCount = newCount + ( charToNumber(Serial.read()) * 10);
    newCount = newCount + ( charToNumber(Serial.read()) );
  }
  else
  {
    //not enough input, just keep the old value
    Serial.println("Data is invalid, keeping old value");
    newCount = counter;
  }
  
  unsigned int oldCount = counter;
  counter = newCount;
  displayNumber(counter);
  
  return oldCount;
}

unsigned int charToNumber(char givenChar)
{
  if( givenChar >= '0' && givenChar <= '9')
  {
    return givenChar - 48;
  }
  
  //if the given char wasn't a number
  return 0;
}

    
