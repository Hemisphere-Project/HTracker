from pymitter import EventEmitter
from termcolor import colored
import sys, threading
from abc import ABC, abstractmethod

printlock = threading.Lock()

class EventEmitterX(EventEmitter):
    def __init__(self, name, color):
        super().__init__(wildcard=True, delimiter=".")
        self.name = name.replace(" ", "_")
        self.nameP = colored(('['+self.name+']').ljust(10, ' ')+' ', color)
        self.logEvents = True

    def log(self, *argv):
        with printlock:
            print(self.nameP, *argv)
            sys.stdout.flush()

    # Emit extended
    def emit(self, event, *args):
        if self.logEvents:
            fullEvent = self.name.lower() + '.' + event
            self.log('EVENT', fullEvent, *args )
        a = [event] + list(args)    # prepend event to args
        super().emit(event, *a) 


class BaseInterface(ABC, EventEmitterX):
    def  __init__(self, name, color="blue"):
        super().__init__(name, color)

        # stopping flag
        self.stopped = threading.Event()
        self.stopped.set()

        # Listen thread
        self.recvThread = threading.Thread(target=self.listen)


    # Receiver THREAD (ABSTRACT)
    @abstractmethod
    def listen(self):
        self.stopped.wait()

    # Start
    def start(self):
        self.stopped.clear()
        self.recvThread.start()
        return self

    # Stop
    def quit(self):
        self.log("stopping...")
        self.stopped.set()
        self.recvThread.join()
        self.log("stopped")

	# is Running
    def isRunning(self, state=None):
        if state is not None:
            self.stopped.clear() if state else self.stopped.set()
        return not self.stopped.is_set()
