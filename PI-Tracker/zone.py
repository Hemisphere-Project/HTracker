
from base import EventEmitterX

class Zone(EventEmitterX):
    def __init__(self, data=None):
        super().__init__('Zone', 'magenta')

        self.dmxchannels = []
        self.dmxvalue = 255
        self.min = 0
        self.max = 0

        if data:
            self.setup(data)


    def setup(self, data):
        if 'dmxchannels' in data:
            self.dmxchannels = [int(c) for c in data['dmxchannels']]
        if 'dmxvalue' in data: 
            self.dmxvalue = int(data['dmxvalue'])
        if 'min' in data:
            self.min = int(data['min'])
        if 'max' in data:
            self.max = int(data['max'])


    def export(self):
        exp = {}
        exp['dmxchannels'] = self.dmxchannels
        exp['dmxvalue'] = self.dmxvalue
        exp['min'] = self.min
        exp['max'] = self.max
        return exp


    def process(self, measure):
        dmxData = []
        if measure >= self.min and measure < self.max:
            for c in self.dmxchannels:
                dmxData.append( (c, self.dmxvalue) )
        else:
            for c in self.dmxchannels:
                dmxData.append( (c, 0) )
        return dmxData