from base import BaseInterface
from DMXEnttecPro import Controller
from DMXEnttecPro.utils import get_port_by_serial_number
import time


class DmxOutput (BaseInterface):

    def __init__(self):
        super().__init__("DMX out", "green")
        self.test = False

    def listen(self):

        port = get_port_by_serial_number('EN159845')
        self.log("Starting DMX sender on port", port)

        self.dmx = Controller(port)

        i = 0
        while self.isRunning():
            if not self.test:
                time.sleep(0.2)
            else:
                i = (i+1)%2
                if i % 2:
                    self.dmx.set_channel(1, 255)
                else:
                    self.dmx.set_channel(1, 128)
                self.dmx.submit()
                time.sleep(1)
        
                # for i in range(256):
                #     self.set_channel(1, i)
                #     self.submit()
                #     time.sleep(0.02)

                # for i in range(256):
                #     self.set_channel(1, 255-i)
                #     self.submit()
                #     time.sleep(0.02)