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

    def blackout(self, submit=True):
        for s in self.sensors:
            s.blackout(submit)



class SceneBook():

    def __init__(self, dmxout, file):
        self.dmxout = dmxout
        self.file = file

        self.scenes = [None]*256
        self.activeScene = 0
        
        self.load()
        
    def selectscene(self, sceneN):
        if sceneN < len(self.scenes): 

            # blackout old sensors
            if self.activeScene > 0 and self.scenes[self.activeScene]:
                self.scenes[self.activeScene].blackout(False)

            # change scene
            self.activeScene = sceneN

            # blackout new sensors
            if self.activeScene > 0 and self.scenes[self.activeScene]:
                self.scenes[self.activeScene].blackout(False)


    def setup(self, data):
        if 'scenes' in data:
            for k, s in enumerate(data['scenes']):
                if s:
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

        # Scenario not found: creating a new one
        if not os.path.exists(self.file):
            print("Scenario not found. Creating a new one.")
            template = {"scenes": [None]*256}
            template["scenes"][1] = {"sensors": [{
                                    "hid": 1,
                                    "zones": [
                                        {"dmxchannels": [1], "dmxvalue": 255, "min": 500, "max": 2000},
                                        {"dmxchannels": [2], "dmxvalue": 255, "min": 3000, "max": 4000}
                                    ]}]}

            self.setup(template)
            self.save()

        # Loading scenario from file
        try:
            with open(self.file, "r") as f:
                jdata = f.read()
            jdata=json.loads(jdata)
            self.setup( jdata )
            print("- reload scenario")
        except:
            print("error while loading scenario..")


    
    def process(self, sensorHID, measure):
        if self.activeScene >= 0 and self.activeScene < len(self.scenes):
            if self.scenes[self.activeScene]: 
                self.scenes[self.activeScene].process(sensorHID, measure)

    