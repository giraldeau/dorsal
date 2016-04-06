#!/usr/bin/python3

import smbus
import time

SLAVE_ADDR = 0x4

wire = smbus.SMBus(1)

def safe_op(lbd, tries = 10):
    retry = 0
    while(retry < tries):
        try:
            return lbd()
        except IOError:
            print("IOError, retrying")
            retry += 1
            time.sleep(0.1)
    raise IOError("max retry")

CMD_START = 0x42;
CMD_STOP = 0x43;
CMD_RESET = 0x44;
CMD_DUMP = 0x45;

def send_cmd(cid):
    wire.write_byte(SLAVE_ADDR, cid)

def toggle():
    send_cmd(CMD_START)
    time.sleep(1)
    send_cmd(CMD_STOP)

def fetch_hist():
    hlen = 400 # must match on the Arduino
    hist = []
    wire_read = lambda: wire.read_word_data(SLAVE_ADDR, 0)
    for i in range(hlen):
        val = safe_op(wire_read)
        hist.append(val)
    return hist

if __name__=="__main__":
    send_cmd(CMD_RESET)

    toggle()
    fetch_hist()

    send_cmd(CMD_DUMP)

