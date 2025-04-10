# SmallTVCOS Flash

SmallTV PRO/ GeekMagicClock PRO 
- <a href="https://de.aliexpress.com/item/1005005699410709.html">from aliexpress</a>

(This Project based on PRO VERSION !!)

## THANK

Thanks go to all people how did the basics:
<a href="https://community.home-assistant.io/t/installing-esphome-on-geekmagic-smart-weather-clock-smalltv-pro/618029"> Installing ESPhome on GEEKMAGIC Smart Weather Clock (smalltv/pro)</a>

## Hardware

ESP32-D0WD-V3 chip (ESP32-WROOM-32)
LED: Onboard blue LED on GPIO2 (inverted)
Disaplay:  Tft Display st7789 1,54“ 240x240 Spi 
Button: Touch button on gpio32

## Serial 

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




![LOGO](images/CmdOS_logo.gif) a OpenOn.org project - develop by mk@almi.de 