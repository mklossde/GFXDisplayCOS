# SmallTVCOS Flash

ESP32-D0WD-V3 chip (ESP32-WROOM-32)
LED: Onboard blue LED on GPIO2 (inverted)
Disaplay:  Tft Display st7789 1,54“ 240x240 Spi 
Button: Touch button on gpio32

## Flash via Serial 

PINS:

1 GND [] 
2 TX (35) 
3 RX (34)
4 3V3
5 GPIO0 (25) - low an reset for flash
6 RST (3) 

## Display

|1| GND
|2| GP0 → D/C
|3| GND → CS ? tied to GND
|4| GP14 → SCK
|5| GP13 → MOSI
|6| GP2 - > reset ?
|7| VCC
|8| GND
|9| GP5 → Back-light Anode through P MOSFet
|10| GND → Back-light Cathode


## Other Firmware

Original Firmware:
https://github.com/GeekMagicClock/smalltv-pro

HomeESP Firmware 
https://github.com/clowrey/esphome-geekmagic-small-TV-pro


![LOGO](images/CmdOS_logo.gif) a OpenOn.org project - develop by mk@almi.de 