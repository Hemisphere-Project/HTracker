from base import BaseInterface
import serial
from base import BaseInterface
from datetime import datetime
from DMXEnttecPro.utils import get_port_by_serial_number

class M5Interface (BaseInterface):

    def __init__(self):
        super().__init__("M5", "red")
        self._book = {}


    def listen(self):

        port = get_port_by_serial_number('01DB750E')   
        self.log("Starting M5 receiver on port", port)

        ser = serial.Serial(port, timeout=0.01, baudrate=115200)

        while self.isRunning():

            data = ser.read_until('\n')
            if len(data) > 2:
                # self.log(data[:-2].decode())
                try: 
                    data = data[:-2].decode().split(':')
                    sensor = int(data[0])
                    measure = int(data[1]) 
                except: 
                    continue
                if sensor in self._book and self._book[sensor]['value'] == measure:
                    self._book[sensor]['timestamp'] = datetime.now()
                else:
                    self._book[sensor] = {'sensor':sensor, 'value': measure, 'timestamp': datetime.now()}
                    self.emit('measure', self._book[sensor])
                # self.log(sensor, measure)


