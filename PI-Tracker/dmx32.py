import serial
import math
from base import BaseInterface
from DMXEnttecPro.utils import get_port_by_serial_number
from time import sleep


class Dmx32 (BaseInterface):

    def __init__(self, portname, addrin, eventname, rangedivider=1.0):
        super().__init__("DMX32", "yellow")
        self.portname=portname
        self._addrin = addrin
        self._cache = -1
        self._rangedivider = rangedivider
        self._eventname = eventname
        self._dmxbuffer = [0]*513
        self.serialok = False
        self._serial = None

    def listen(self):

        # ATTACH SERIAL
        while self.isRunning() and not self.serialok:
            try:
                port = get_port_by_serial_number(self.portname)   
                self.log("Starting DMX32 on port", port)
                self._serial = serial.Serial(port, timeout=0.1, baudrate=115600) 
                self.serialok = True
            except:
                self.log("ERROR: ", self.portname, "not found.. retrying")
                sleep(5)

        # SET CTRL ADDR
        if self._serial:
            self.log("Sending CTRL addr", self._addrin)
            self._serial.write(('\n').encode('utf-8'))
            self._serial.write(('\n').encode('utf-8'))
            msg = ('-'+str(self._addrin)+'\n').encode('utf-8')
            self._serial.write(msg)            

        # RUN
        #
        while self.isRunning():
            line = self._serial.readline().decode().strip()
            if line:
                if line.startswith('#'): 
                    self.log(line)
                    continue
                try:
                    addr, value = line.split(':')
                    addr = int(addr)
                    value = int(value)
                    if addr == self._addrin:
                        if self._cache != round(value/self._rangedivider):
                            self._cache = round(value/self._rangedivider)
                            self.emit(self._eventname, self._cache)
                except:
                    self.log('??', line)
            else:
                # self.log("silence..")
                pass

        if self._serial:
            self.log("kill esp32")
            msg = ('x'+'\n').encode('utf-8')
            self._serial.write(msg) 
            self._serial.flush()
            self._serial.close()
            sleep(0.5)



    def send(self, data):
        msg = (str(data[0])+':'+str(data[1])+'\n').encode('utf-8')
        if self._serial:
            self._serial.write(msg)


    # Send all (but not 0) values
    def update(self, sendNulls=False):
        for addr, value in enumerate(self._dmxbuffer):
            if addr > 0 and (value > 0 or sendNulls):
                self.send( (addr, value) )

    
    def set(self, addr, value, doSend=True):
        self._dmxbuffer[addr] = value
        if doSend:
            self.send( (addr, value) )


    def clear(self, doSend=True):
        self._dmxbuffer = [0]*513
        if doSend:
            self.send( (0,0) )
