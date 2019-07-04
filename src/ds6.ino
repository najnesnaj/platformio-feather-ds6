
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <ds6_SSD1306.h>
//#include <BLEPeripheral.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

//Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, PIN_SPI_MOSI, PIN_SPI_SCK, OLED_DC, OLED_RST, OLED_CS);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &SPI,  OLED_DC, OLED_RST, OLED_CS);

int i;

void setup()
{
//	pinMode(PIN_VIBRATE, OUTPUT);
	Serial.begin(38400); //to comply with the standard settings DFU
	Serial.println(__FILE__);


if(!display.begin(SSD1306_SWITCHCAPVCC)) {
 Serial.println("SSD1306 allocation failed");
 }
 else
 Serial.println("SSD1306 allocation worked!");
}

void loop()
{
	Serial.println("test");
//	digitalWrite(PIN_VIBRATE, 1);
	delay(20);
//	digitalWrite(PIN_VIBRATE, 0);
	delay(20);

	if (Serial.available() > 0) {
		// read the incoming byte:
		Serial.println("incoming byte");

	}
display.clearDisplay();
for (i=1; i<30; i++)
{
display.drawPixel(i, i, WHITE);
}

display.display();




}









