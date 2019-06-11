import serial
import argparse
import time
import logging
import signal
import pyvjoy # Windows apenas

def handler(signum, frame):
    print("Exiting!")
    exit(0)

class MyControllerMap:
    def __init__(self):
        self.buttons = {'green': 1, 'red': 2, 'amarelo': 3, 'blue': 4, 'orange': 5, 'down_stroke': 6, 'up_stroke': 7, 'whammy': 8, 'special': 9}

class SerialControllerInterface:
    # Protocol
    # byte 1 -> ID << 4 | signal << 0
    # byte 2 -> EOP -> reserved value 'X'

    def __init__(self, port, baudrate):
        self.ser = serial.Serial(port, baudrate=baudrate)
        self.mapping = MyControllerMap()
        self.j = pyvjoy.VJoyDevice(1)
        self.incoming = '0'

    def update(self):
        ## Sync protocol
        while self.incoming != b'X':
            self.incoming = self.ser.read()
            logging.debug("Received INCOMING: {}".format(self.incoming))

        data = self.ser.read(2)
        logging.debug("Received DATA: {}".format(data))
        data_str = bytearray(data)
        
        signal, Xis = data_str
        colors = ["green","red","amarelo","blue", "orange", "up_stroke", "down_stroke", "whammy", "special"]
        
        num = signal >> 4 # id do botão apertado
        value = signal & 15
        if(colors[num-1] == "whammy"):
            if value > 4:
                self.j.set_button(self.mapping.buttons["whammy"], 0)
            else:
                self.j.set_button(self.mapping.buttons["whammy"], 1)
        else:
            self.j.set_button(self.mapping.buttons[colors[num-1]], value)

if __name__ == '__main__':
    run = 0
    try:
        print("Connecting controller")
        interfaces = ['dummy', 'serial']
        argparse = argparse.ArgumentParser()
        argparse.add_argument('serial_port', type=str)
        argparse.add_argument('-b', '--baudrate', type=int, default=9600)
        argparse.add_argument('-c', '--controller_interface', type=str, default='serial', choices=interfaces)
        argparse.add_argument('-d', '--debug', default=False, action='store_true')
        args = argparse.parse_args()
        if args.debug:
            logging.basicConfig(level=logging.DEBUG)

        controller = SerialControllerInterface(port=args.serial_port, baudrate=args.baudrate)
        print("Connection to {} using {} interface ({})".format(args.serial_port, args.controller_interface, args.baudrate))
        print("Press Ctrl+C to exit")
        signal.signal(signal.SIGINT, handler)
        print("Controller connected, launch JoyToKey to start playing!")
        run = 1

    except:
        print("Failed to connect to controller")

    if run: 
        while True:
            controller.update()
