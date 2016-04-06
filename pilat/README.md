pilat -  Raspberry-Pi interrupt latency monitor
#####

This directory contains the programs to measure interrupt latency. It consists of an Arduino UNO monitor and a Raspberri-Pi 2 target.

The RPi waits for an event on the GPIO, and then assert a pin to acknowledge the event. The Arduino measures the time between asserting the trigger pin and the ack signal. This latency is added to an histogram.

The Arduino can be controled with I2C, including fetching the histogram data. The monitor code can be uploaded using the standard arduino IDE. The code for Raspberry-Pi requires python3-smbus package. The wiring is documented in the monitor source code.

Required hardware:
* Arduino UNO
* Raspberry-PI 2
* Breadboard
* Male-female and male-male wires
* 10k pull-down resistor
* 10k/30k voltage divider or 5v to 3.3v level-shifter




