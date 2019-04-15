import serial
import argparse
import time
import logging
import pyvjoy # Windows apenas

class MyControllerMap:
    def __init__(self):
        self.botoes = {'verde': 1,'vermelho': 2, 'amarelo': 3, 'azul': 4, 'laranja': 5, 'palheta': 6, 'pause': 7}

class SerialControllerInterface:

    # Protocolo
    # byte 1 -> Botão 1 (estado - Apertado 1 ou não 0)
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

        data = self.ser.read(8)
        logging.debug("Received DATA: {}".format(data))
        data_str = bytearray(data).decode('ascii')
        logging.debug("data_str:{}".format(data_str)) # rodar com -d para debug
        verde,vermelho,amarelo,azul,laranja,palheta,pause,Xis = data_str

        print(verde, vermelho, amarelo, azul, laranja, palheta, pause)

        if verde == b'1':
            logging.info("Pressionando verde")
            self.j.set_button(self.mapping.botoes['verde'], 1)
        elif verde == b'0':
            self.j.set_button(self.mapping.botoes['verde'], 0)
            
        if vermelho == b'1':
            logging.info("Pressionando vermelho")
            self.j.set_button(self.mapping.botoes['vermelho'], 1)
        elif vermelho == b'0':
            self.j.set_button(self.mapping.botoes['vermelho'], 0)
            
        if amarelo == b'1':
            logging.info("Pressionando amarelo")
            self.j.set_button(self.mapping.botoes['amarelo'], 1)
        elif amarelo == b'0':
            self.j.set_button(self.mapping.botoes['amarelo'], 0)
            
        if azul == b'1':
            logging.info("Pressionando azul")
            self.j.set_button(self.mapping.botoes['azul'], 1)
        elif azul == b'0':
            self.j.set_button(self.mapping.botoes['azul'], 0)
            
        if laranja == b'1':
            logging.info("Pressionando laranja")
            self.j.set_button(self.mapping.botoes['laranja'], 1)
        elif laranja == b'0':
            self.j.set_button(self.mapping.botoes['laranja'], 0)
            
        if palheta == b'1':
            logging.info("Pressionando palheta")
            self.j.set_button(self.mapping.botoes['palheta'], 1)
        elif palheta == b'0':
            self.j.set_button(self.mapping.botoes['palheta'], 0)

        if pause == b'1':
            logging.info("Pressionando pause")
            self.j.set_button(self.mapping.botoes['pause'], 1)
        elif pause == b'0':
            self.j.set_button(self.mapping.botoes['pause'], 0)

        self.incoming = self.ser.read()


if __name__ == '__main__':
    interfaces = ['dummy', 'serial']
    argparse = argparse.ArgumentParser()
    argparse.add_argument('serial_port', type=str)
    argparse.add_argument('-b', '--baudrate', type=int, default=9600)
    argparse.add_argument('-c', '--controller_interface', type=str, default='serial', choices=interfaces)
    argparse.add_argument('-d', '--debug', default=False, action='store_true')
    args = argparse.parse_args()
    if args.debug:
        logging.basicConfig(level=logging.DEBUG)

    print("Connection to {} using {} interface ({})".format(args.serial_port, args.controller_interface, args.baudrate))
    controller = SerialControllerInterface(port=args.serial_port, baudrate=args.baudrate)

    while True:
        controller.update()
