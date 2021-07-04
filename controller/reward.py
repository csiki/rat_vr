import serial
from utils import *


def prefeed(port: serial.Serial, msg_type=2):
    packet = bytearray()
    packet.append(msg_type)
    packet.append(0x0)  # dummy

    port.write(packet)
    check_proc_msg_type(port, msg_type)


def dispense(port: serial.Serial, ul, msg_type=3):
    packet = bytearray()
    packet.append(msg_type)
    packet.append(ul & 0xff)

    port.write(packet)
    check_proc_msg_type(port, msg_type)


def puff(port: serial.Serial, which, t, msg_types=(4, 5, 6)):
    dir_map = {'l': msg_types[0], 'r': msg_types[1], 'b': msg_types[2]}
    msg_type = dir_map[which]

    packet = bytearray()
    packet.append(msg_type)
    packet.append(t & 0xff)

    port.write(packet)
    check_proc_msg_type(port, msg_type)
