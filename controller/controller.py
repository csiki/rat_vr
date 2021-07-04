import sys, os
import serial
import glob
import time
from pprint import pprint

from utils import *
import reward
import roller


def serial_ports():
    """ Lists serial port names

        :raises EnvironmentError:
            On unsupported or unknown platforms
        :returns:
            A list of the serial ports available on the system
    """
    if sys.platform.startswith('win'):
        ports = ['COM%s' % (i + 1) for i in range(256)]
    elif sys.platform.startswith('linux') or sys.platform.startswith('cygwin'):
        # this excludes your current terminal "/dev/tty"
        ports = glob.glob('/dev/tty[A-Za-z]*')
    elif sys.platform.startswith('darwin'):
        ports = glob.glob('/dev/tty.*')
    else:
        raise EnvironmentError('Unsupported platform')

    result = []
    for port in ports:
        try:
            s = serial.Serial(port)
            s.close()
            result.append(port)
        except (OSError, serial.SerialException):
            pass
    return result


def get_dev_id(port: serial.Serial, msg_type=0x1):
    packet = bytearray()
    packet.append(msg_type)
    port.write(packet)
    dev_id = int.from_bytes(port.read(1), 'big')

    check_proc_msg_type(port, msg_type)

    return dev_id


# TODO add functions to run roller & reward, with the port as param


if __name__ == '__main__':
    roller_dev_id = 1  # defined in arduino files
    reward_dev_id = 2

    port_settings = {'baudrate': 115200}

    port_ids = serial_ports()
    print(f'ports found: {port_ids}')

    ports = [serial.Serial(pid, **port_settings) for pid in port_ids]
    time.sleep(3)  # wait for setup

    devs = {get_dev_id(port): port for port in ports}
    roller_dev = devs[roller_dev_id]
    reward_dev = devs[reward_dev_id]
    pprint(devs)

    # roller.roll(roller_dev, 'l', 100, 1000)
    # roller.roll(roller_dev, 'r', 150, 1000)
    # roller.roll(roller_dev, 'l', 200, 1000)
    # roller.roll(roller_dev, 'r', 255, 1000)

    reward.puff(reward_dev, 'l', 255)
    reward.puff(reward_dev, 'r', 255)
    reward.puff(reward_dev, 'b', 255)
