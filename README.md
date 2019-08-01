# platformio-feather-ds6

Adafruit industries is so kind as to provide a lot of source-code.
Pls be so kind as to buy stuff from them!

the ds6 is a smartwatch which contains the nrf52832 microcontroller.

Have a look at @fanoush, the guy that discovered and unlocked the ds6 potential!

I used the platformio configuration for the nrf52832 feather board from Adafruit.(see below for howto)
https://docs.platformio.org/en/latest/quickstart.html



I attached the watch to a raspberry pi, so I can develop and flash over the network.



It is equally possible to use Arduino IDE to create sketches.
Just follow the instructions for installing the Adafruit feather nrf52 board.
under C:\..\AppData\Local\Arduino15\packages\adafruit\hardware\nrf52\0.11.1\variants\feather_nrf52832
you can find the variant.h, which you can replace by the one found under feather_nrf52


this is work in progress, but it generates a usable firmware which you can flash to the smartwatch


--------usefull scripts------

startdebugger (starts openocd back-end, to which you can connect with telnet 127.0.0.1 4444)
genimage (compiles and creates uploadable image)
uploadblue (upload firmware via bluetooth)
upload-serial (upload firmware via serial port)

-----------

the adafruit boatloader was modified by fanoush
Adafruit_nRF52_Bootloader-dsd6.diff -- use patch -p1
it is adapted to the watch, which does not have the buttons and led the featherboard has

----------------------------

IMPORTANT: 
when performing a reset in openocd, make it a double

----------------------------


steps I used to install platformio on rpi: 

platformio platform install nordicnrf52

platformio init --board adafruit_feather_nrf52832

platformio lib search Adafruit_GFX

platformio lib install 13

replace ~/.platformio/packages/framework-arduinoadafruitnrf52/variants/feather_nrf52832  by the one under this repo
replace ~/.platformio/packages/framework-arduinoadafruitnrf52/..../cores/nRF5/linker/nrf52832_s132_v6.ld

to compile :  pio run
under directory it puts the firmware  .pioenvs/adafruit_feather_nrf52832/firmware.hex 

~

~

