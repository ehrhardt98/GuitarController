import serial
import argparse
import time
import logging
import pyvjoy # Windows apenas

class MyControllerMap:
    def __init__(self):
        self.botoes = {'verde': 1, 'vermelho': 2, 'amarelo': 3, 'azul': 4, 'laranja': 5, 'palheta_down': 6, 'palheta_up': 7, 'whammy_up': 8, 'whammy_down': 9}

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

        data = self.ser.read(9)
        # print(data)
        logging.debug("Received DATA: {}".format(data))
        data_str = bytearray(data).decode('ascii')
        logging.debug("data_str:{}".format(data_str)) # rodar com -d para debug
        verde,vermelho,amarelo,azul,laranja,palheta_down,palheta_up,analog,Xis = data_str

        print(verde, vermelho, amarelo, azul, laranja, palheta_down, palheta_up, analog)

        if verde == '1':
            logging.info("Pressionando verde")
            self.j.set_button(self.mapping.botoes['verde'], 1)
        elif verde == '0':
            self.j.set_button(self.mapping.botoes['verde'], 0)
            
        if vermelho == '1':
            logging.info("Pressionando vermelho")
            self.j.set_button(self.mapping.botoes['vermelho'], 1)
        elif vermelho == '0':
            self.j.set_button(self.mapping.botoes['vermelho'], 0)
            
        if amarelo == '1':
            logging.info("Pressionando amarelo")
            self.j.set_button(self.mapping.botoes['amarelo'], 1)
        elif amarelo == '0':
            self.j.set_button(self.mapping.botoes['amarelo'], 0)
            
        if azul == '1':
            logging.info("Pressionando azul")
            self.j.set_button(self.mapping.botoes['azul'], 1)
        elif azul == '0':
            self.j.set_button(self.mapping.botoes['azul'], 0)
            
        if laranja == '1':
            logging.info("Pressionando laranja")
            self.j.set_button(self.mapping.botoes['laranja'], 1)
        elif laranja == '0':
            self.j.set_button(self.mapping.botoes['laranja'], 0)
            
        if palheta_down == '1':
            logging.info("Pressionando palheta_down")
            self.j.set_button(self.mapping.botoes['palheta_down'], 1)
        elif palheta_down == '0':
            self.j.set_button(self.mapping.botoes['palheta_down'], 0)

        if palheta_up == '1':
            logging.info("Pressionando palheta_up")
            self.j.set_button(self.mapping.botoes['palheta_up'], 1)
        elif palheta_up == '0':
            self.j.set_button(self.mapping.botoes['palheta_up'], 0)

        if int(analog) <= 5:
            self.j.set_button(self.mapping.botoes['whammy_up'], 1)
            self.j.set_button(self.mapping.botoes['whammy_down'], 0)

        elif int(analog) > 5:
            self.j.set_button(self.mapping.botoes['whammy_up'], 0)
            self.j.set_button(self.mapping.botoes['whammy_down'], 1)

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
