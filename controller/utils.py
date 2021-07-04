import sys
import serial


def check_proc_msg_type(port: serial.Serial, msg_type, throw_err=False):
    proc_msg_type = int.from_bytes(port.read(1), 'big')
    if proc_msg_type != msg_type:
        err = f'WRONG MSG PROCESSED: {proc_msg_type} instead of {msg_type}'
        print(err, file=sys.stderr)
        if throw_err:
            raise ValueError(err)
