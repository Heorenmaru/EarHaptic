from pythonosc.udp_client import SimpleUDPClient
from pythonosc.dispatcher import Dispatcher
from pythonosc.osc_server import BlockingOSCUDPServer
from threading import Thread
import time
import USBloggerLib
from numpy import uint32, uint8,int8
import numpy as np

import colorama
from colorama import Fore, Back, Style
colorama.init()
#    Fore.BLACK  
#    Fore.RED    
#    Fore.GREEN  
#    Fore.YELLOW 
#    Fore.BLUE   
#    Fore.MAGENTA
#    Fore.CYAN   
#    Fore.WHITE  
#    Fore.RESET  
#    Fore.LIGHTBLACK_EX  
#    Fore.LIGHTRED_EX    
#    Fore.LIGHTGREEN_EX  
#    Fore.LIGHTYELLOW_EX 
#    Fore.LIGHTBLUE_EX   
#    Fore.LIGHTMAGENTA_EX
#    Fore.LIGHTCYAN_EX   
#    Fore.LIGHTWHITE_EX  
red =      Fore.RED
green =    Fore.GREEN
lgreen =   Fore.LIGHTGREEN_EX
blue =     Fore.BLUE
yellow =   Fore.YELLOW
lyellow =  Fore.LIGHTYELLOW_EX
lcyan =    Fore.LIGHTCYAN_EX
lmag =     Fore.LIGHTMAGENTA_EX
lblue =    Fore.LIGHTBLUE_EX
reset =    Fore.RESET
##################################################################
t_pulseL = 5 # Delay (ms)
t_pulseR = 5 # Delay (ms)





###################################################################


tasks = {}
def usb_rx(data):
    global receivedTrg
    receivedTrg = 1
    try:
        tasks[str(data[0])](data[1:-1])
    except:
        pass
        print(green, data)   


def checkDevice():
    USBloggerLib.send([0x00,0b00000001])
    

def StartPort():
    ports = USBloggerLib.find_port()
    for port in ports:
        USBloggerLib.check_port(port)

    listener = Thread(target=USBloggerLib.StartListener, args=(usb_rx, port, False, ), daemon=True) 
    listener.start()
    

    #while not USBloggerLib.port_is_open():
    #    tasks['0']=checkDevice()
    
def ear_l_d(delay):
    cmd = 0x01
    USBloggerLib.send([cmd, 0x1])
    time.sleep(delay/1000)
    USBloggerLib.send([cmd, 0x0])
    time.sleep(delay/1000)
        
def ear_r_d(delay):
    cmd = 0x02
    USBloggerLib.send([cmd, 0x1])
    time.sleep(delay/1000)
    USBloggerLib.send([cmd, 0x0])
    time.sleep(delay/1000)


def Leds(ld):
    cmd = 0x05

    start_time = time.time()
    while (start_time + 10) > time.time():
        if USBloggerLib.send([cmd,  uint8(ld)]):
            break
        else:
            time.sleep(0.5)




#==================================================

def handler(address, *args):
    print("Received OSC message from {}: {}".format(address, args))

    if args[1] == 'L':
        ear_l_d(t_pulseL)
        ear_l_d(t_pulseL)


    if args[1] == 'R':
        ear_r_d(t_pulseR)
        ear_r_d(t_pulseR)






##################### MAIN #####################
if __name__ == '__main__':
    StartPort()
    ### test ###
    for i in range(3):
        Leds(0b00000001)
        time.sleep(0.2)
        Leds(0b00000010)
        time.sleep(0.2)
        Leds(0b00000100)
        time.sleep(0.2)
        Leds(0b00001000)
        time.sleep(0.2)

    ear_l_d(t_pulseL)
    ear_l_d(t_pulseL)
    ear_l_d(t_pulseL)
    ear_l_d(t_pulseL)
    ear_l_d(t_pulseL)
    ear_l_d(t_pulseL)
    ear_l_d(t_pulseL)
    ear_l_d(t_pulseL)
    ear_l_d(t_pulseL)

    ear_r_d(t_pulseR)
    ear_r_d(t_pulseR)
    ear_r_d(t_pulseR)
    ear_r_d(t_pulseR)
    ear_r_d(t_pulseR)
    ear_r_d(t_pulseR)
    ear_r_d(t_pulseR)
    ear_r_d(t_pulseR)
    ear_r_d(t_pulseR)

    ###    ###

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