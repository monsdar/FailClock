import json
import os
import socket
import subprocess
import time

PYTHON = "c:/Python27/python.exe"
SYNCSCRIPT = os.getcwd() + "/SyncClock.py"
RESETSCRIPT = os.getcwd() + "/ResetClock.py"

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
            jsonData = json.loads(data)
            if( jsonData["build"]["status"] == "FAILED" ||
                jsonData["build"]["status"] == "FAILURE" ):
                time.sleep(2.0) #wait until COM can be used without problem
                subprocess.Popen(PYTHON + ' "' + RESETSCRIPT + '"').wait() #reset the clock if the build failed
                
            time.sleep(2.0) #wait until COM can be used without problem
            subprocess.Popen(PYTHON + ' "' + SYNCSCRIPT + '"').wait() #always sync, can't be done too often
        except:
            print "ERROR"
            pass #cannot work with the data, let's just ignore it then...
    
if __name__ == "__main__":
    main()
