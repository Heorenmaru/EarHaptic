from pythonosc.udp_client import SimpleUDPClient
from pythonosc.dispatcher import Dispatcher
from pythonosc.osc_server import BlockingOSCUDPServer
import time



##################################################################
t_pulseL = 5 # Delay (ms)
t_pulseR = 5 # Delay (ms)





###################################################################

def ear_l(delay):
    cmd = 0x01
    # send_to_usb-com([cmd, 0x1])
    time.sleep(delay/1000)
    # send_to_usb-com([cmd, 0x0])
    time.sleep(delay/1000)
        
def ear_r(delay):
    cmd = 0x02
    # send_to_usb-com([cmd, 0x1])
    time.sleep(delay/1000)
    # send_to_usb-com([cmd, 0x0])
    time.sleep(delay/1000)






#==================================================

def handler(address, *args):
    print("Received OSC message from {}: {}".format(address, args))

    if args[1] == 'L':
        ear_l(t_pulseL)
        ear_l(t_pulseL)


    if args[1] == 'R':
        ear_r(t_pulseR)
        ear_r(t_pulseR)






##################### MAIN #####################
if __name__ == '__main__':
   
    # start usb-com (pyserial)
    # ....


    dispatcher = Dispatcher()
    #dispatcher.map("*", handler)
    dispatcher.map("/avatar/parameters/L", handler)
    dispatcher.map("/avatar/parameters/R", handler)

    ip = "127.0.0.1"
    port = 9001

    server = BlockingOSCUDPServer((ip, port), dispatcher)
    print("OSC server started on {}:{}".format(ip, port))

    client = SimpleUDPClient(ip, port)

    while True:
        # отправляем OSC сообщение на сервер
        #client.send_message("/test", [1, 2, 3])

        # обрабатываем входящие OSC сообщения
        server.handle_request()