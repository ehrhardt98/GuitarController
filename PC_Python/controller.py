import serial
import argparse
import time
import logging
import subprocess
import signal
import pyvjoy # Windows apenas

def handler(signum, frame):
    print("Exiting!")
    exit(0)

class MyControllerMap:
    def __init__(self):
        self.botoes = {'verde': 1, 'vermelho': 2, 'amarelo': 3, 'azul': 4, 'laranja': 5, 'palheta_down': 6, 'palheta_up': 7, 'whammy': 8, 'especial': 9}

class SerialControllerInterface:
    # Protocolo
    # byte 1 -> ID << 4 | sinal << 0
    # byte 2 -> EOP - End of Packet -> valor reservado 'X'

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
        
        sinal, Xis = data_str
        cores = ["verde","vermelho","amarelo","azul", "laranja", "palheta_up", "palheta_down", "whammy", "especial"]
        
        num = sinal >> 4 # id do botão apertado
        valor = sinal & 15
        if(cores[num-1] == "whammy"):
            if valor > 4:
                self.j.set_button(self.mapping.botoes["whammy"], 0)
            else:
                self.j.set_button(self.mapping.botoes["whammy"], 1)
        else:
            self.j.set_button(self.mapping.botoes[cores[num-1]], valor)

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
        print("Use Ctrl+C to exit")
        print("Controller connected, launch JoyToKey to start playing!")
        signal.signal(signal.SIGINT, handler)
        run = 1
        
    except:
        print("Failed to connect to controller")

    if run:
        while True:
            controller.update()
