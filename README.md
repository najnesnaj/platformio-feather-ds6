# platformio-feather-ds6
I used the platformio configuration for the nrf52832 feather board from Adafruit, and adapted it quick&dirty for the DS-D6 smartwatch.
This way you get the latest core and a way to use bluetooth services on the nrf52

this is work in progress, but it generates a usable firmware which you can flash to the smartwatch

I use the startdebuggerrpi script, which starts the openocd debugger on the raspberry pi GPIO pins.

You can then connect to the debugger with telnet 127.0.0.1 4444

upload firmware with : program firmware.hex



-----------
Have a look at :

https://docs.platformio.org/en/latest/quickstart.html




steps I used : 

platformio platform install nordicnrf52

platformio init --board adafruit_feather_nrf52832

platformio lib search Adafruit_GFX

platformio lib install 13

replace ~/.platformio/packages/framework-arduinoadafruitnrf52/variants/feather_nrf52832  by the one under this repo


to compile :  platformio run
under directory it puts the firmware  .pioenvs/adafruit_feather_nrf52832/firmware.hex 

~

~

