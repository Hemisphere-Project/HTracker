
class Zone():
    def __init__(self, data=None):
        self.dmxchannels = []
        self.dmxvalue = 255
        self.min = 0
        self.max = 0

        if data:
            self.setup(data)


    def setup(self, data):
        if 'dmxchannels' in data:
            self.dmxchannels = data['dmxchannels']
        if 'dmxvalue' in data: 
            self.dmxvalue = data['dmxvalue']
        if 'min' in data:
            self.min = data['min']
        if 'max' in data:
            self.max = data['max']


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