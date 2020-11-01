import sys
import glob
import serial
import datetime
from time import sleep

def serial_ports():
    if sys.platform.startswith('linux') or sys.platform.startswith('cygwin'):
        # this excludes your current terminal "/dev/tty"
        ports = glob.glob('/dev/tty[A-Za-z]*')

    result = []
    for port in ports:
        try:
            s = serial.Serial(port)
            s.close()
            result.append(port)
        except (OSError, serial.SerialException):
            pass
    return result
    

if __name__ == '__main__':
    print(serial_ports())
   
    ser = serial.Serial("/dev/ttyUSB0", timeout=None, baudrate=250000, xonxoff=False, rtscts=False, dsrdtr=False) 

    maxIndex = 1024
    buffer = [0]*maxIndex
    index = -1

    while True:
        dataTime = datetime.datetime.now()
        value = ord(ser.read(1))

        diff = (datetime.datetime.now() - dataTime).total_seconds() * 1000
        if diff > 10:
            index = 0
            print(buffer)
        
        if index >= maxIndex:
            print('no BRK received.. reset')
            index = -1
        elif index >= 0:
            buffer[index] = value
            index+=1
