

import datetime
import serial
import time

def getSecondsSinceMidnight():
    now = datetime.datetime.now()
    midnight = now.replace(hour=0, minute=0, second=0, microsecond=0)
    seconds = (now - midnight).seconds
    return seconds
    
def intTo5DigitString(number):
    return "%05d" % number 

def main():
    #Note: This assumes that the Arduino is connected to COM3. Usually
    #      this is the case when there aren't any other Serial-devices.
    serialHandle = serial.Serial("COM3", 9600)

    #sync the timer
    #Note: Needs to be done because the serial connection resets the Arduino
    #      It also comes in practical to sync the timer from time to time...
    time.sleep(2.0)
    seconds = getSecondsSinceMidnight()
    secondStr = intTo5DigitString(seconds)
    data = "T" + secondStr
    serialHandle.write(data)
    print data

if __name__ == "__main__":
    main()
    