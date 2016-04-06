#!/usr/bin/env python3
# script by Alex Eames http://RasPi.tv
# http://RasPi.tv/how-to-use-interrupts-with-python-on-the-raspberry-pi-and-rpi-gpio-part-3
import time
import sys
import RPi.GPIO as GPIO

pin_trg = 23
pin_ack = 24

def triggered(channel):
    print("triggered")
    GPIO.output(pin_ack, GPIO.HIGH)
    # make sure the arduino observed the ack state
    while(GPIO.input(pin_trg) != 0):
        continue
    GPIO.output(pin_ack, GPIO.LOW)

def setup_gpio():
    GPIO.setmode(GPIO.BCM)
    GPIO.setup(pin_trg, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)
    GPIO.setup(pin_ack, GPIO.OUT)
    GPIO.add_event_detect(pin_trg, GPIO.RISING, callback=triggered)

if __name__=="__main__":
    import i2c # must be in the same directory

    setup_gpio()

    i2c.send_cmd(i2c.CMD_RESET)
    i2c.send_cmd(i2c.CMD_START)

    hist = []
    try:
        while(True):
            time.sleep(0.1)
    except KeyboardInterrupt:
        print("quit")
    finally:
        i2c.send_cmd(i2c.CMD_STOP)
        time.sleep(1)
        hist = i2c.fetch_hist()
        GPIO.cleanup()
    print(hist)

