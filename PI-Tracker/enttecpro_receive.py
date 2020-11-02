import serial
import math
from base import BaseInterface
from DMXEnttecPro.utils import get_port_by_serial_number

# ENTTEC FRAME specs:
# https://github.com/SavinaRoja/DMXEnttecPro/blob/master/supplemental/dmx_usb_pro_api_spec.pdf

class DmxInput (BaseInterface):

    def __init__(self, addrin, eventname, rangedivider=10):
        super().__init__("DMX in", "yellow")
        self._addrin = addrin
        self._rangedivider = rangedivider
        self._eventname = eventname
        self._cache = -1


    def listen(self):

        port = get_port_by_serial_number('EN169216')   
        self.log("Starting DMX receiver on port", port)

        ser = serial.Serial(port, timeout=0.1, baudrate=250000, xonxoff=False, rtscts=False, dsrdtr=False) 

        maxDataLength = 600
        buffer = [0]*maxDataLength
        frameStarted = False

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

            if not frameStarted: 
                frameStarted = (readByte() == 126)    # start code

            else:

                frameLabel = readByte()
                if frameLabel < 1 or frameLabel > 12:
                    self.log("Wrong label:", frameLabel)
                    frameStarted = False
                    continue

                frameLength = readByte() 
                frameLength += readByte()*256
                if frameLength > maxDataLength:
                    self.log("Wrong size:", frameLength)
                    frameStarted = False
                    continue
                
                # self.log("frame", frameLabel, frameLength)

                buffer = [0]*frameLength
                for i in range(frameLength):
                    buffer[i] = readByte()
                    # print(buffer[i], end =" ")
                
                
                if readByte() == 231:         # end code
                    
                    # DMX in frame
                    if frameLabel == 5:
                        if frameLength == 0:
                            self.log("Empty frame received")
                        elif buffer[0] > 0:
                            # self.log("Overflow/Overrun occured")
                            pass
                        else:
                            buffer.pop(0)

                            # Scene select
                            val = math.floor(buffer[self._addrin]*100/255/self._rangedivider)
                            # self.log(self._addrin, buffer[self._addrin], val)
                            if val != self._cache:
                                self._cache = val
                                self.emit(self._eventname, self._cache)

                            # Fwd dmx IN
                            self.emit('dmxin', buffer)



                    frameStarted = False
                    continue

                else:
                    self.log("Wrong delimiter")
                    frameStarted = False
                    continue
