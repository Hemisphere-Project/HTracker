import time
import dmx

sender = dmx.DMX_Serial()
sender.start()
# for i in range(200):
#     if i % 2:
#         sender.set_data(bytes((255,)*512))
#     else:
#         sender.set_data(bytes((0,)*512))
#     time.sleep(1)
    
while True:
    for i in range(256):
        sender.set_data(bytes((i,)*512))
        time.sleep(0.02)

    for i in range(256):
        sender.set_data(bytes((255-i,)*512))
        time.sleep(0.02)