# set async_mode to 'threading', 'eventlet', 'gevent' or 'gevent_uwsgi' to
# force a mode else, the best mode is selected automatically from what's
# installed

import time
from base import BaseInterface
from flask import Flask, render_template, send_from_directory, request
import socketio

class Webserver (BaseInterface):
    def __init__(self, scenebook, M5, DMX32):
        super().__init__("WEB", "blue")
        self.book = scenebook
        self.M5 = M5
        self.DMX32 = DMX32

        self.sio = socketio.Server(logger=False, async_mode='eventlet')
        self.app = Flask(__name__)
        self.app.wsgi_app = socketio.WSGIApp(self.sio, self.app.wsgi_app)
        self.app.config['SECRET_KEY'] = '*secret!*'
        self.thread = None
        self.last_update = {}


        def background_thread():
            """Example of how to send server generated events to clients."""
            
            while True:
                self.sio.sleep(0.5)
                update = {'scene':self.book.activeScene, 
                            'CAN': self.M5.serialok, 
                            'DMX': self.DMX32.serialok,
                            'CTRL': self.DMX32._addrin,
                            'MEAS': self.M5._book.copy()
                            }
                if update != self.last_update:
                    self.last_update = update
                    self.sio.emit('update', update)



        @self.app.route('/')
        def index():
            return send_from_directory('static', 'index.html')

        @self.app.route('/static/<path:path>')
        def send_static(path):
            self.log('static', path)
            return send_from_directory('static', path)


        @self.sio.event
        def connect(sid, environ):
            self.last_update = {}
            if self.thread is None:
                self.thread = self.sio.start_background_task(background_thread)
            self.push_book(sid)


        @self.sio.event
        def disconnect(sid):
            self.log('Client disconnected')

        @self.sio.event
        def save(sid, data):
            M5.pause()
            self.book.clear()
            self.book.setup(data)
            self.book.save()
            M5.play()
            self.log('Web scenario saved')

        # @self.sio.event
        # def my_event(sid, message):
        #     self.sio.emit('my_response', {'data': message['data']}, room=sid)


        # @self.sio.event
        # def my_broadcast_event(sid, message):
        #     self.sio.emit('my_response', {'data': message['data']})


        # @self.sio.event
        # def join(sid, message):
        #     self.sio.enter_room(sid, message['room'])
        #     self.sio.emit('my_response', {'data': 'Entered room: ' + message['room']},
        #             room=sid)


        # @self.sio.event
        # def leave(sid, message):
        #     self.sio.leave_room(sid, message['room'])
        #     self.sio.emit('my_response', {'data': 'Left room: ' + message['room']},
        #             room=sid)


        # @self.sio.event
        # def close_room(sid, message):
        #     self.sio.emit('my_response',
        #             {'data': 'Room ' + message['room'] + ' is closing.'},
        #             room=message['room'])
        #     self.sio.close_room(message['room'])


        # @self.sio.event
        # def my_room_event(sid, message):
        #     self.sio.emit('my_response', {'data': message['data']}, room=message['room'])


        # @self.sio.event
        # def disconnect_request(sid):
        #     self.sio.disconnect(sid)



    def listen(self):
        
        self.log('starting')

        # deploy with eventlet
        import eventlet
        import eventlet.wsgi
        eventlet.wsgi.server(eventlet.listen(('', 5000)), self.app)

    # Stop
    def quit(self):
        self.log("stopping...")
        self.stopped.set()
        # TODO: proper shutdown
        self.recvThread.join()
        self.log("stopped")


    def push_book(self, sid):
        self.sio.emit('scenario', self.book.export(), room=sid)