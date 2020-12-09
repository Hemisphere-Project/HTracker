from operator import itemgetter
from base import EventEmitterX
from zone import Zone

def interpol(x, in_min, in_max, out_min, out_max):
  return int((x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min);


class Sensor (EventEmitterX):

    def __init__(self, data=None):
        super().__init__('Sensor', 'magenta')

        self.hid = 0
        self.zones = []

        if data:
            self.setup(data)


    def setup(self, data):
        if 'hid' in data: 
            self.hid = int(data['hid'])
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
            return []

        # APPLY DMX
        result = []
        for z in self.zones:
            result += z.process(measure)
        return result
        
    

    
