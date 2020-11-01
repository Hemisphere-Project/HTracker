from sensor import Sensor
import json, os


class Scene():

    def __init__(self, dmxout, data=None):
        self.dmxout = dmxout

        self.sensors = []

        if data:
            self.setup(data)


    def addSensor(self, data):
        self.sensors.append( Sensor(self.dmxout, data) )


    def setup(self, data):
        if 'sensors' in data:
            for s in data['sensors']:
                self.addSensor(s)


    def export(self):
        exp = {'sensors': []}
        for s in self.sensors:
            exp['sensors'].append(s.export())
        return exp


    def process(self, sensorHID, measure):
        for s in self.sensors:
            if s.hid == sensorHID:
                s.process(measure)



class SceneBook():

    def __init__(self, dmxout, file):
        self.dmxout = dmxout
        self.file = file

        self.scenes = [None]*25
        self.activeScene = 0
        
    def selectscene(self, sceneN):
        if sceneN < len(self.scenes) and self.scenes[sceneN]: 
            self.activeScene = sceneN

    def setup(self, data):
        if 'scenes' in data:
            for k, s in enumerate(data['scenes']):
                self.scenes[k] = Scene(self.dmxout, s)

    def export(self):
        exp = {'scenes': []}
        for s in self.scenes:
            if s:
                exp['scenes'].append(s.export())
            else:
                exp['scenes'].append(None)
        return exp

    def save(self):
        jdata = json.dumps( self.export(), indent=4 )
        with open(self.file, "w") as f:
            f.write(jdata)
    
    def load(self):
        try:
            with open(self.file, "r") as f:
                jdata = f.read()
            jdata=json.loads(jdata)
            self.setup( jdata )
            print("reload scenario")
        except:
            print("error while loading scenario")
    
    def process(self, sensorHID, measure):
        if self.activeScene >= 0 and self.activeScene < len(self.scenes):
            if self.scenes[self.activeScene]: 
                self.scenes[self.activeScene].process(sensorHID, measure)

    