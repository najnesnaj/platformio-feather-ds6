# platformio-feather-ds6
nrf52832 smartwatch ds-d6 adafruit-feather 

I used the platformio configuration for the nrf52832 feather board from Adafruit, and adapted it quick&dirty for the DS-D6 smartwatch.
This way you get the latest core and a way to use bluetooth services on the nrf52

-----------
Have a look at :

https://docs.platformio.org/en/latest/quickstart.html




steps I used : 

platformio platform install nordicnrf52

platformio init --board adafruit_feather_nrf52832

platformio lib search Adafruit_GFX

platformio lib install 13

replace ~/.platformio/packages/framework-arduinoadafruitnrf52/variants/feather_nrf52832  by the one under this repo


~

~

