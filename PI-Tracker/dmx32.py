import serial
import math
from base import BaseInterface
from DMXEnttecPro.utils import get_port_by_serial_number
from time import sleep


class Dmx32 (BaseInterface):

    def __init__(self, portname, addrin, eventname, rangedivider=1):
        super().__init__("DMX32", "yellow")
        self.portname=portname
        self._addrin = addrin
        self._rangedivider = rangedivider
        self._eventname = eventname
        self._cache = -1
        self.serialok = False

    def listen(self):

        while self.isRunning() and not self.serialok:
            try:
                port = get_port_by_serial_number(self.portname)   
                self.log("Starting DMX32 on port", port)
                ser = serial.Serial(port, timeout=0.1, baudrate=115600) 
                self.serialok = True
            except:
                self.log("ERROR: ", self.portname, "not found.. retrying")
                sleep(5)

        # TODO: set _addrin to ARDUINO

        # wait for a valid byte
        def readByte():
            val = 0
            while self.isRunning():
                try:
                    val = ord(ser.read(1))
                    break
                except: pass
            return val

        # RUN
        #
        while self.isRunning():
            
            line = ser.readline().decode().strip()
            if line:
                addr, value = line.split(':')
                addr = int(addr)
                value = int(value)
                if addr == self._addrin:
                    self.emit(self._eventname, value)


    def push(self):
        # TODO: send all not null data
        pass

    def set(self, addr, value, push=True):
        # TODO: set buffer and send data (if push)
        pass

