from operator import itemgetter


def interpol(x, in_min, in_max, out_min, out_max):
  return int((x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min);


class Sensor():

    def __init__(self, dmxout, data=None):
        self.dmxout = dmxout
        self.hid = 0
        self.thresholds = []
        self.interpolation = 0      # 0: no / 1: linear
        self.dmxchannels = []
        self.lastMeasure = 0

        if data:
            self.setup(data)


    def setup(self, data):
        if 'hid' in data: 
            self.hid = data['hid']
        if 'thresholds' in data:
            self.thresholds = data['thresholds']
            self.thresholds = sorted(self.thresholds, key=itemgetter(0), reverse=False)
        if 'interpolation' in data:
            self.interpolation = data['interpolation']
        if 'dmxchannels' in data:
            self.dmxchannels = data['dmxchannels']


    def export(self):
        exp = {}
        exp['hid'] = self.hid
        exp['dmxchannels'] = self.dmxchannels
        exp['interpolation'] = self.interpolation
        exp['thresholds'] = self.thresholds
        return exp


    def process(self, measure):
        selIndex = -1
        for k, t in enumerate(self.thresholds):
            if measure > t[0]:
                selIndex = k
        
        dmxValue = 0
        if selIndex >= 0:
            dmxValue = self.thresholds[selIndex][1]
                
            if self.interpolation == 1:    
                if len(self.thresholds) > selIndex+1:
                    dmxValue = interpol(measure, self.thresholds[selIndex][0], self.thresholds[selIndex+1][0], self.thresholds[selIndex][1], self.thresholds[selIndex+1][1])
        
        for c in self.dmxchannels:
            self.dmxout.dmx.set_channel(c, dmxValue)
            # print(c, dmxValue)
        if len(self.dmxchannels) > 0:
            self.dmxout.dmx.submit()

        self.lastMeasure = measure