# SmallTVCOS Flash

A build version is at <a href="../bin">bin</A>

## Flash firmware update

- Connect SmallTV to power, access via AP and setup the local wlan.
- Access via IP from  browser and update firmware to SmallTVCOS.
- (Update will take some time)

Use "SmallTVCOS_Vxxx.bin" for this update.


## Flash via Serial 

PINS:
1 GND [] 
2 TX (35) 
3 RX (34)
4 3V3
5 GPIO0 (25) - low an reset for flash
6 RST (3) 

Flash via USB2Serial with file "SmallTVCOS_Vxxx_paritions.bin"


## Other Firmware

Original Firmware:
https://github.com/GeekMagicClock/smalltv-pro

HomeESP Firmware 
https://github.com/clowrey/esphome-geekmagic-small-TV-pro


![LOGO](images/CmdOS_logo.gif) a OpenOn.org project - develop by michael@openon.org 