import time
from base import BaseInterface
import serial
from base import BaseInterface
from datetime import datetime
from DMXEnttecPro.utils import get_port_by_serial_number

class M5Interface (BaseInterface):

    def __init__(self):
        super().__init__("M5", "red")
        self._book = {}
        self.playing = True
        self.wasPaused = True
        self._serial = None
        self._buffer = []


    def listen(self):

        port = get_port_by_serial_number('01DB750E')   
        self.log("Starting M5 receiver on port", port)
        self.pause()

        self._serial = serial.Serial(port, timeout=0.1, baudrate=115200)

        while self.isRunning():

            if self.playing:
                try:
                    readChar = self._serial.read()
                except:
                    self.log('Serial Read error..')
                
                if readChar == b'\n' or readChar == b'\r':
                    if len(self._buffer):
                        self.process(self._buffer)
                        self._buffer = []
                elif readChar != '':
                    self._buffer.append(readChar)                

            else:
                time.sleep(0.1)


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

            if sensor in self._book and self._book[sensor]['value'] == measure:
                pass
            else:
                self._book[sensor] = {'sensor':sensor, 'value': measure}
                self.emit('measure', self._book[sensor])

        # other events
        else:
            self.emit('event', data, buffer)



    def pause(self):
        self.log("Paused")
        self.playing = False
        self.wasPaused = True

    def play(self):
        self.log("Play")
        
        if self.playing:
            self.playing = False
        else:
            self._book = {}
    
        if self._serial:
            self._serial.reset_input_buffer()
            self._buffer = []

        for s in self._book:
            self.emit('measure', self._book[s])
        self.playing = True

