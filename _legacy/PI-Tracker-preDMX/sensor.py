from operator import itemgetter
from zone import Zone

def interpol(x, in_min, in_max, out_min, out_max):
  return int((x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min);


class Sensor():

    def __init__(self, dmxout, data=None):
        self.dmxout = dmxout

        self.hid = 0
        self.zones = []

        self.lastMeasure = 0
        
        if data:
            self.setup(data)


    def setup(self, data):
        if 'hid' in data: 
            self.hid = data['hid']
        if 'zones' in data:
            for s in data['zones']:
                self.addZone(s)


    def export(self):
        exp = {'hid': self.hid, 'zones':[]}
        for z in self.zones:
            exp['zones'].append(z.export())
        return exp
        
    def addZone(self, data):
        self.zones.append( Zone(data) )


    def process(self, measure):
        
        # IGNORE - <500 -
        if measure < 500:   
            return

        # APPLY DMX
        dirty = False
        for z in self.zones:
            for data in z.process(measure):
                self.dmxout.dmx.set_channel(data[0], data[1])
                # print('dmx', data[0], data[1])
                dirty = True
    
        if dirty:
            self.dmxout.dmx.submit()

        self.lastMeasure = measure


    def blackout(self, submit=True):
        for z in self.zones:
            for c in z.dmxchannels:
                self.dmxout.dmx.set_channel(c, 0)
        if submit:
            self.dmxout.dmx.submit()

    
