
import json
import os
import socket
import subprocess
import time

PYTHON = "c:/Python26/python.exe"
RESETSCRIPT = os.getcwd() + "/ResetClock.py"
INCSCRIPT = os.getcwd() + "/IncreaseClock.py"

UDP_IP = "0.0.0.0"
UDP_PORT = 31337

def main():
    sock = socket.socket(socket.AF_INET,    # Internet
                         socket.SOCK_DGRAM) # UDP
    sock.bind((UDP_IP, UDP_PORT))

    while True:
        data, addr = sock.recvfrom(1024 * 100) # buffer size is 100 kB
        
        #assume that the given data is usable Jenkins code
        try:
            print "Received: "
            print data
            jsonData = json.loads(data)
            if( jsonData["build"]["status"] == "FAILED" or
                jsonData["build"]["status"] == "FAILURE" ):
                time.sleep(2.0) #wait until COM can be used without problem
                subprocess.Popen(PYTHON + ' "' + RESETSCRIPT + '"').wait() #reset the clock if the build failed
            elif(   jsonData["build"]["status"] == "SUCCESS" and
                    jsonData["build"]["phase"] == "FINISHED"):
                time.sleep(2.0) #wait until COM can be used without problem
                subprocess.Popen(PYTHON + ' "' + INCSCRIPT + '"').wait() #increase the clock on success
                
        except:
            print "Received unusable data"
            pass #cannot work with the data, let's just ignore it then...
    
if __name__ == "__main__":
    main()
