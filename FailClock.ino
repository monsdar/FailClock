 
 /**
  * Controls the 7 segment display and provides a serial interface to perform certain functions
  * http://learn.adafruit.com/adafruit-led-backpack/1-2-inch-7-segment-backpack
  *
  *  --- Requirements ---
  *  This sketch communicates via Serial interface, see below for a detailed description of the protocol.
  *  Besides the Adafruit libraries to control the Backpack nothing is needed.
  *
  *  --- Protocol ---
  * Note: The protocol is very naive and in no way robust.
  *       Do not try to send malformed data, you'll regret it ;)
  *       This is due to the attempt to make the code as small as possible (though readability is another important factor).
  * Note: After establishing a serial connection the Arduino will autoreset itself.
  *       The actual counter will be stored via EEPROM, so you do not have to worry about that.
  *       However the 'seconds since midnight' need to be reset, else they'll fall back to 0.
  *
  *       It is possible to avoid the AutoReset:
  *         http://playground.arduino.cc/Main/DisablingAutoResetOnSerialConnection
  *       but I want to modify the Arduino as less as possible (making it easier to rebuild the project)
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
#include <Wire.h>   //I2C communication
#include <EEPROM.h> //Write current count permanently (if the Arduino resets, we'll still show the right value)

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
  //NOTE: Commented out because I do not want to send more data than needed
  //Serial.println("7Seg Counter");
  //Serial.println("Usage:"); 
  //Serial.println("T012345 - Set the seconds since last midnight");
  //Serial.println("R0123 - Reset the display to the given days");
  //Serial.println("For a more verbose explanation take a look at the docs in the source");

  //Check if there is data in the EEPROM available
  counter = readEeprom();

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
    if(input == 'T')      //'T' means that the timer should be updated
    {
      updateTime(); 
    }
    else if(input == 'R') //'R' means that the display should be updated
    {
      unsigned int lastValue = updateDisplay();
      Serial.println(lastValue, DEC); //answer with the previous counter (to let the caller keep a Highscore or whatever)
    }
    else
    {
      //do nothing here... just put the input into /dev/null aka Nirvana
      while(Serial.available())
      {
        Serial.read();
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
  //if the counter reaches its maximum we'll simply jump back to 0000
  //for day counts this should be enough (more than 27 years, you've earned your jump to 0000 then...)
  counter++;
  if(counter == 9999)
  {
    counter = 0;
  }
}

void displayNumber(unsigned int number)
{
  //this splits the 4-digit number into single digits (else we wouldn't be able to print something like leading zeroes etc)
  const unsigned char NUMDIGITS = 4;
  unsigned char singleDigits[NUMDIGITS];
  for(unsigned char index = NUMDIGITS; index-- > 0;) //reverse iterate
  {
    singleDigits[index] = number % 10;
    number = number/10;
  }
  
  //Write the digits to the display
  //NOTE: The digits on the display do not address with 0,1,2,3, so we cannot iterate with a for-loop here
  matrix.writeDigitNum(0, singleDigits[0]); //first number  (index 0)
  matrix.writeDigitNum(1, singleDigits[1]); //second number (index 1)
  matrix.writeDigitNum(3, singleDigits[2]); //third number  (index 3)
  matrix.writeDigitNum(4, singleDigits[3]); //fourth number (index 4)
  matrix.writeDisplay();
  
  //store the value on the EEPROM
  //this is needed to have the actual counter available after the Arduino is resetted (Power on/off, Serial connection, ...)
  //Note: EEPROM only allows ~100,000 writes, so one can think that this is not the best solution.
  //      Considering that we're writing 2 bytes, which means the EEPROM will last for 50000 updates.
  //      An update occurs in the following cases:
  //        - Arduino is turned on (shouldn't happen to often if the power source is reliable)
  //        - A serial connection is established (This should just happen if the counter needs to be reset)
  //        - Another day is reached (Which'll only happen once in a day, yeah that's logic!)
  //      Assuming that you have a really crappy environment where you need to reset the counter every day (2 writes per day),
  //      the EEPROM should be fine for more than 60 years.
  
  //Store 2 digits in one byte to avoid writing more data than needed
  unsigned char firstData = (singleDigits[0] * 10) + singleDigits[1];
  unsigned char secondData = (singleDigits[2] * 10) + singleDigits[3];
  EEPROM.write(0, firstData);
  EEPROM.write(1, secondData);
}

unsigned int readEeprom()
{
  unsigned char firstData = EEPROM.read(0);
  unsigned char secondData = EEPROM.read(1);
  
  //if a value is above 99 it hasn't been written by our program (or it hasn't been written at all)
  if(firstData > 99 ||
     secondData > 99)
  {
    //simply return 0 if the data is invalid
    return 0;
  }
  
  //form a 4 digit number from it again
  unsigned int readValue = (firstData * 100) + secondData;
  return readValue;
}

void updateTime()
{
  //This reads the 5 bytes from the Serial line and forms an unsigned long out of them
  seconds = 0;
  seconds = seconds + ( (unsigned long)charToNumber(Serial.read()) * 10000L);
  seconds = seconds + ( (unsigned long)charToNumber(Serial.read()) * 1000L);
  seconds = seconds + ( (unsigned long)charToNumber(Serial.read()) * 100L);
  seconds = seconds + ( (unsigned long)charToNumber(Serial.read()) * 10L);
  seconds = seconds + ( (unsigned long)charToNumber(Serial.read()) );
  
  //NOTE: Commented out because I do not want to send more data than needed
  //Serial.print("Current Time: ");
  //Serial.println( seconds );
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
    newCount = counter;
    
    //NOTE: Commented out because I do not want to send more data than needed
    //Serial.println("Data is invalid, keeping old value");
  }
  
  unsigned int oldCount = counter;
  counter = newCount;
  displayNumber(counter);
  
  return oldCount;
}

unsigned int charToNumber(char givenChar)
{
  //ASCII for '0' is 48, increasing constantly to 57 for '9', so it's easy to
  //get the numeric value out of the char
  if( givenChar >= '0' && givenChar <= '9')
  {
    return givenChar - 48;
  }
  
  //if the given char wasn't a number use 0 as a fallback
  return 0;
}

    
