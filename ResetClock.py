
import serial
import time

def main():
    #Note: This assumes that the Arduino is connected to COM3. Usually
    #      this is the case when there aren't any other Serial-devices.
    serialHandle = serial.Serial("COM3", 9600)

    #reset the display
    #Note: this returns the previous value, we're ignoring it for now
    #Note: We need to sleep a bit before writing to the device
    time.sleep(2.0)
    data = "R0000"
    serialHandle.write(data)
    print data

if __name__ == "__main__":
    main()