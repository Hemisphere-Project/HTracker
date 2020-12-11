import time
import serial
from base import BaseInterface
from datetime import datetime
from DMXEnttecPro.utils import get_port_by_serial_number

class M5Interface (BaseInterface):

    def __init__(self, portname):
        super().__init__("M5", "red")
        self.portname = portname
        self.serialok = False
        self.playing = True
        self._book = {}
        self._serial = None
        self._buffer = []


    def listen(self):

        while self.isRunning() and not self.serialok:
            try:
                port = get_port_by_serial_number(self.portname)   
                self.log("Starting M5 receiver on port", port)
                self.pause()
                self._serial = serial.Serial(port, timeout=0.1, baudrate=115200)
                self.serialok = True
            except:
                self.log("ERROR: ", self.portname, "not found.. retrying")
                time.sleep(5)

        if self.serialok:
            self.log("M5 ready")
            msg = ('w'+'\n').encode('utf-8')    # connect wifi (OTA)

        while self.isRunning():
            try:
                readChar = self._serial.read()
                self.serialok = True
            except:
                self.log('Serial Read error..')
                self.serialok = False
            
            if readChar == b'\n' or readChar == b'\r':
                if len(self._buffer):
                    self.process(self._buffer)
                    self._buffer = []
            elif readChar != '':
                self._buffer.append(readChar)           

        if self._serial:
            self._serial.close()     



    def process(self, buffer):

        # trim
        while len(buffer) > 0 and buffer[0] == b'': buffer.pop(0)
        while len(buffer) > 0 and buffer[-1] == b'': buffer.pop()

        # decode
        data = (b''.join(buffer)).decode()

        # measure
        if len(buffer) > 2 and buffer[1] == b':':
            try: 
                data = data.split(':')
                sensor = int(data[0])
                measure = int(data[1]) 
            except: 
                return

            # ignore duplicates
            if sensor in self._book and self._book[sensor]['value'] == measure:
                pass

            # save and emit new measure
            elif self.playing:
                self._book[sensor] = {'sensor':sensor, 'value': measure}
                self.emit('measure', self._book[sensor])

        # other events
        else:
            self.emit('event', data, buffer)


    def pause(self):
        self.log("Paused")
        self.playing = False
        self._book = {}


    def play(self):
        self.log("Play")
        self.playing = True

