from base import BaseInterface
from DMXEnttecPro import Controller
from DMXEnttecPro.utils import get_port_by_serial_number
import time


class DmxOutput (BaseInterface):

    def __init__(self, portname):
        super().__init__("DMX out", "green")
        self.portname=portname
        self.serialok = False
        self.test = False

    def listen(self):

        while self.isRunning() and not self.serialok:
            try:
                port = get_port_by_serial_number(self.portname)
                self.log("Starting DMX sender on port", port)
                self.dmx = Controller(port)
                self.serialok = True
            except:
                self.log("ERROR: ", self.portname, "not found.. retrying")
                time.sleep(5)

        self.stopped.wait()

        # i = 0
        # while self.isRunning():
        #     if not self.test:
        #         time.sleep(0.2)
        #     else:
        #         i = (i+1)%2
        #         if i % 2:
        #             self.dmx.set_channel(1, 255)
        #         else:
        #             self.dmx.set_channel(1, 128)
        #         self.dmx.submit()
        #         time.sleep(1)
        
                # for i in range(256):
                #     self.set_channel(1, i)
                #     self.submit()
                #     time.sleep(0.02)

                # for i in range(256):
                #     self.set_channel(1, 255-i)
                #     self.submit()
                #     time.sleep(0.02)

    def setBuffer(self, buffer):
        for i in range( min(512,len(buffer)) ):
            if i > 0:
                self.dmx.set_channel(i, buffer[i])
        self.dmx.submit()
        # self.log("DMX fwd", buffer)
        # time.sleep(1)