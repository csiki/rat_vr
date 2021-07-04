import serial
from utils import *

# as defined in roller.ino


def get_btn_state(port: serial.Serial, msg_type=2):
    packet = bytearray()
    packet.append(msg_type)
    packet.extend([0x0, 0x0, 0x0])  # dummy values
    port.write(packet)

    state = int.from_bytes(port.read(1), 'big')
    check_proc_msg_type(port, msg_type)

    return state


def pull_to_base(port: serial.Serial, msg_type=3):
    packet = bytearray()
    packet.append(msg_type)
    packet.extend([0x0, 0x0, 0x0])  # dummy values
    port.write(packet)

    check_proc_msg_type(port, msg_type)


def lin_act(port: serial.Serial, which, direction, t, msg_types=(4, 5, 6, 7)):
    dir_map = {'fb': {'ext': msg_types[0], 'contr': msg_types[1]},
               'lr': {'ext': msg_types[2], 'contr': msg_types[3]}}
    msg_type = dir_map[which][direction]

    packet = bytearray()
    packet.append(msg_type)
    packet.append(0x0)  # dummy
    packet.extend([(t & 0xff00) >> 8, t & 0x00ff])

    port.write(packet)
    check_proc_msg_type(port, msg_type)


def roll(port: serial.Serial, direction, pwm, t, msg_types=(8, 9, 10, 11)):
    dir_map = {'f': msg_types[0], 'b': msg_types[1], 'l': msg_types[2], 'r': msg_types[3]}
    msg_type = dir_map[direction]

    packet = bytearray()
    packet.append(msg_type)
    packet.append(pwm & 0xff)
    packet.extend([(t & 0xff00) >> 8, t & 0x00ff])

    port.write(packet)
    check_proc_msg_type(port, msg_type)


def pull_sol(port: serial.Serial, t, msg_type=12):
    packet = bytearray()
    packet.append(msg_type)
    packet.append(0x0)  # dummy
    packet.extend([(t & 0xff00) >> 8, t & 0x00ff])

    port.write(packet)
    check_proc_msg_type(port, msg_type)
