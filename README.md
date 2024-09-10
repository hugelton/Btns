# Btns
Buttons board compatible with monome grid

## Overview
Btns is a clone of [Monome Grid]([url](https://monome.org)).


## Make
### Hardware
Make your board from the gerber.

#### BOM
+ Raspberry Pi Pico (or same footprint clones) * 1
+ TSHD-T-2.8T2 SMD Tact switch with LED * 64
+ 1N4148W (SOD-123) SMD diode * 64
+ TM1640 LED segments driver * 1
+ 10k THT resistor * 2
+ 10ohm THT resistor * 1
+ MPU-6050 (optional for tilt mod)
+ VXO7803-500 (optional)
+ Eurorack 16pin Bus Conn (optional)

### Firmware
upload the firmware to Pico from [Release]([url](https://github.com/hugelton/Btns/releases/tag/0.3)). 

## Acknowledgments
Based on okyeron's [neotrellis-monome](https://github.com/okyeron/neotrellis-monome/) project.
