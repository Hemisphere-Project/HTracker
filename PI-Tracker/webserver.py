# set async_mode to 'threading', 'eventlet', 'gevent' or 'gevent_uwsgi' to
# force a mode else, the best mode is selected automatically from what's
# installed
ASYNC_MODE = 'eventlet'

import time
from base import BaseInterface
from flask import Flask, render_template, send_from_directory
import socketio

class Webserver (BaseInterface):
    def __init__(self, scenebook):
        super().__init__("WEB", "blue")
        self.book = scenebook
        self.sio = socketio.Server(logger=True, async_mode=ASYNC_MODE)
        self.app = Flask(__name__)
        self.app.wsgi_app = socketio.WSGIApp(self.sio, self.app.wsgi_app)
        self.app.config['SECRET_KEY'] = '*secret!*'
        self.thread = None


        def background_thread():
            """Example of how to send server generated events to clients."""
            count = 0
            while True:
                self.sio.sleep(10)
                count += 1
                self.sio.emit('my_response', {'data': 'Server generated event'})


        @self.app.route('/')
        def index():
            if self.thread is None:
                self.thread = self.sio.start_background_task(background_thread)
            return send_from_directory('static', 'index.html')

        @self.app.route('/static/<path:path>')
        def send_static(path):
            self.log('static', path)
            return send_from_directory('static', path)


        @self.sio.event
        def connect(sid, environ):
            self.push_book(sid)


        @self.sio.event
        def disconnect(sid):
            self.log('Client disconnected')


        @self.sio.event
        def my_event(sid, message):
            self.sio.emit('my_response', {'data': message['data']}, room=sid)


        @self.sio.event
        def my_broadcast_event(sid, message):
            self.sio.emit('my_response', {'data': message['data']})


        @self.sio.event
        def join(sid, message):
            self.sio.enter_room(sid, message['room'])
            self.sio.emit('my_response', {'data': 'Entered room: ' + message['room']},
                    room=sid)


        @self.sio.event
        def leave(sid, message):
            self.sio.leave_room(sid, message['room'])
            self.sio.emit('my_response', {'data': 'Left room: ' + message['room']},
                    room=sid)


        @self.sio.event
        def close_room(sid, message):
            self.sio.emit('my_response',
                    {'data': 'Room ' + message['room'] + ' is closing.'},
                    room=message['room'])
            self.sio.close_room(message['room'])


        @self.sio.event
        def my_room_event(sid, message):
            self.sio.emit('my_response', {'data': message['data']}, room=message['room'])


        @self.sio.event
        def disconnect_request(sid):
            self.sio.disconnect(sid)



    def listen(self):
    
        if self.sio.async_mode == 'threading':
            # deploy with Werkzeug
            self.app.run(threaded=True)
        elif self.sio.async_mode == 'eventlet':
            # deploy with eventlet
            import eventlet
            import eventlet.wsgi
            eventlet.wsgi.server(eventlet.listen(('', 5000)), self.app)
        elif self.sio.async_mode == 'gevent':
            # deploy with gevent
            from gevent import pywsgi
            try:
                from geventwebsocket.handler import WebSocketHandler
                websocket = True
            except ImportError:
                websocket = False
            if websocket:
                pywsgi.WSGIServer(('', 5000), self.app,
                                handler_class=WebSocketHandler).serve_forever()
            else:
                pywsgi.WSGIServer(('', 5000), self.app).serve_forever()
        elif self.sio.async_mode == 'gevent_uwsgi':
            self.log('Start the application through the uwsgi server. Example:')
            self.log('uwsgi --http :5000 --gevent 1000 --http-websockets --master '
                '--wsgi-file app.py --callable app')
        else:
            self.log('Unknown async_mode: ' + self.sio.async_mode)

        self.stopped.wait()
        


    def push_book(self, sid):
        self.sio.emit('scenario', self.book.export(), room=sid)