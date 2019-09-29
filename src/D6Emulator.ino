#include <SPI.h>
#include <stdint.h>
//#include <BLEPeripheral.h>
#include <bluefruit.h>
#include <nrf_nvic.h>//interrupt controller stuff
#include <nrf_sdm.h>
#include <nrf_soc.h>
#include <WInterrupts.h>
#include "Adafruit_GFX.h"
#include "SSD1306.h"
#include <TimeLib.h>
#include <nrf.h>
#include "count_steps.h"
#include "count_steps.c"
#include "i2csoft.h"

#define wdt_reset() NRF_WDT->RR[0] = WDT_RR_RR_Reload
#define wdt_enable(timeout) \
	NRF_WDT->CONFIG = NRF_WDT->CONFIG = (WDT_CONFIG_HALT_Pause << WDT_CONFIG_HALT_Pos) | ( WDT_CONFIG_SLEEP_Pause << WDT_CONFIG_SLEEP_Pos); \
NRF_WDT->CRV = (32768*timeout)/1000; \
NRF_WDT->RREN |= WDT_RREN_RR0_Msk;  \
NRF_WDT->TASKS_START = 1

Adafruit_SSD1306 display(128, 32, &SPI, 28, 4, 29);

boolean debug = false;

#define sleepDelay 7000
#define BUTTON_PIN              30
#define refreshRate 100

int menu;
volatile bool buttonPressed = false;
long startbutton;
unsigned long sleepTime, displayRefreshTime;
volatile bool sleeping = false;
int timezone;
int steps;
int steps1;
String serialNr = "235246472";
String versionNr = "110.200.051";
String btversionNr = "100.016.051";
String msgText;
boolean gotoBootloader = false;
boolean vibrationMode;

String bleSymbol = " ";
int contrast;
BLEDis bledis;    // DIS (Device Information Service) helper class instance
BLEBas blebas;    // BAS (Battery Service) helper class instance
BLEUart bleuart;



//BLEPeripheral                   blePeripheral           = BLEPeripheral();
//BLEService                      batteryLevelService     = BLEService("190A");
//BLECharacteristic   TXchar        = BLECharacteristic("0002", BLENotify, 20);
//BLECharacteristic   RXchar        = BLECharacteristic("0001", BLEWriteWithoutResponse, 20);

//BLEService                      batteryLevelService1     = BLEService("190B");
//BLECharacteristic   TXchar1        = BLECharacteristic("0004", BLENotify, 20);
//BLECharacteristic   RXchar1        = BLECharacteristic("0003", BLEWriteWithoutResponse, 20);

#define N_GRAINS     253 // Number of grains of sand
#define WIDTH        127 // Display width in pixels
#define HEIGHT       32 // Display height in pixels
#define MAX_FPS      150 // Maximum redraw rate, frames/second

// The 'sand' grains exist in an integer coordinate space that's 256X
// the scale of the pixel grid, allowing them to move and interact at
// less than whole-pixel increments.
#define MAX_X (WIDTH  * 256 - 1) // Maximum X coordinate in grain space
#define MAX_Y (HEIGHT * 256 - 1) // Maximum Y coordinate
struct Grain {
	int16_t  x,  y; // Position
	int16_t vx, vy; // Velocity
	uint16_t pos;
} grain[N_GRAINS];

uint32_t        prevTime   = 0;      // Used for frames-per-second throttle
uint16_t         backbuffer = 0, img[WIDTH * HEIGHT]; // Internal 'map' of pixels

#ifdef __cplusplus
extern "C" {
#endif

#define LF_FREQUENCY 32768UL
#define SECONDS(x) ((uint32_t)((LF_FREQUENCY * x) + 0.5))
#define wakeUpSeconds 120
	void RTC2_IRQHandler(void)
	{
		volatile uint32_t dummy;
		if (NRF_RTC2->EVENTS_COMPARE[0] == 1)
		{
			NRF_RTC2->EVENTS_COMPARE[0] = 0;
			NRF_RTC2->CC[0] = NRF_RTC2->COUNTER +  SECONDS(wakeUpSeconds);
			dummy = NRF_RTC2->EVENTS_COMPARE[0];
			dummy;
			//powerUp();
		}
	}

	void initRTC2() {

		NVIC_SetPriority(RTC2_IRQn, 15);
		NVIC_ClearPendingIRQ(RTC2_IRQn);
		NVIC_EnableIRQ(RTC2_IRQn);

		NRF_RTC2->PRESCALER = 0;
		NRF_RTC2->CC[0] = SECONDS(wakeUpSeconds);
		NRF_RTC2->INTENSET = RTC_EVTENSET_COMPARE0_Enabled << RTC_EVTENSET_COMPARE0_Pos;
		NRF_RTC2->EVTENSET = RTC_INTENSET_COMPARE0_Enabled << RTC_INTENSET_COMPARE0_Pos;
		NRF_RTC2->TASKS_START = 1;
	}
#ifdef __cplusplus
}
#endif

void powerUp() {
	if (sleeping) {
		sleeping = false;
		display.begin(SSD1306_SWITCHCAPVCC);
		display.clearDisplay();
		display.display();
		if (debug)Serial.begin(38400);

		delay(5);
	}
	sleepTime = millis();
}

void powerDown() {
	if (!sleeping) {
		if (debug)NRF_UART0->ENABLE = UART_ENABLE_ENABLE_Disabled;
		sleeping = true;

		digitalWrite(28, LOW);
		digitalWrite(5, LOW);
		digitalWrite(6, LOW);
		digitalWrite(29, LOW);
		digitalWrite(4, LOW);
		NRF_SAADC ->ENABLE = 0; //disable ADC
		NRF_PWM0  ->ENABLE = 0; //disable all pwm instance
		NRF_PWM1  ->ENABLE = 0;
		NRF_PWM2  ->ENABLE = 0;
	}
}

void charge() {
	if (sleeping)menu = 88;
	powerUp();
}

void buttonHandler() {
	if (!sleeping) buttonPressed = true;
	else menu = 0;
	powerUp();
}

void acclHandler() {
	ReadRegister(0x17);
	if (sleeping) {
		menu = 77;
		powerUp();
	}
}
/*
   void blePeripheralConnectHandler(BLECentral& central) {
   if (debug)Serial.println("BLEconnected");
   menu = 0;
   powerUp();
   bleSymbol = "B";
   }
 */
/*
   void blePeripheralDisconnectHandler(BLECentral& central) {
   if (debug)Serial.println("BLEdisconnected");
   menu = 0;
   powerUp();
   bleSymbol = " ";
   }
 */
String answer = "";
String tempCmd = "";
int tempLen = 0, tempLen1;
boolean syn;
/*
   void characteristicWritten(BLECentral& central, BLECharacteristic& characteristic) {
   char remoteCharArray[21];
   tempLen1 = characteristic.valueLength();
   tempLen = tempLen + tempLen1;
   memset(remoteCharArray, 0, sizeof(remoteCharArray));
   memcpy(remoteCharArray, characteristic.value(), tempLen1);
   tempCmd = tempCmd + remoteCharArray;
   if (tempCmd[tempLen - 2] == '\r' && tempCmd[tempLen - 1] == '\n') {
   answer = tempCmd.substring(0, tempLen - 2);
   tempCmd = "";
   tempLen = 0;
   if (debug)Serial.print("RxBle: ");
   if (debug)Serial.println(answer);
   filterCmd(answer);
   }
   }
 */
void filterCmd(String Command) {
	Serial.println("Command");
	Serial.println(Command);
	if (Command == "AT+BOND") {
		sendBLEcmd("AT+BOND:OK");
	} else if (Command == "AT+ACT") {
		sendBLEcmd("AT+ACT:0");
	} else if (Command.substring(0, 7) == "BT+UPGB") {
		gotoBootloader = true;
	} else if (Command.substring(0, 8) == "BT+RESET") {
		if (gotoBootloader)NRF_POWER->GPREGRET = 0x01;
		sd_nvic_SystemReset();
	} else if (Command.substring(0, 7) == "AT+RUN=") {
		sendBLEcmd("AT+RUN:" + Command.substring(7));
	} else if (Command.substring(0, 8) == "AT+USER=") {
		sendBLEcmd("AT+USER:" + Command.substring(8));
	}  else if (Command.substring(0, 7) == "AT+REC=") {
		sendBLEcmd("AT+REC:" + Command.substring(7));
	} else if (Command.substring(0, 8) == "AT+PUSH=") {
		sendBLEcmd("AT+PUSH:OK");
		menu = 99;
		powerUp();
		handlePush(Command.substring(8));
	} else if (Command.substring(0, 9) == "AT+MOTOR=") {
		sendBLEcmd("AT+MOTOR:" + Command.substring(9));
	} else if (Command.substring(0, 8) == "AT+DEST=") {
		sendBLEcmd("AT+DEST:" + Command.substring(8));
	} else if (Command.substring(0, 9) == "AT+ALARM=") {
		sendBLEcmd("AT+ALARM:" + Command.substring(9));
	} else if (Command.substring(0, 13) == "AT+HRMONITOR=") {
		sendBLEcmd("AT+HRMONITOR:" + Command.substring(13));
	} else if (Command.substring(0, 13) == "AT+FINDPHONE=") {
		sendBLEcmd("AT+FINDPHONE:" + Command.substring(13));
	} else if (Command.substring(0, 13) == "AT+ANTI_LOST=") {
		sendBLEcmd("AT+ANTI_LOST:" + Command.substring(13));
	} else if (Command.substring(0, 9) == "AT+UNITS=") {
		sendBLEcmd("AT+UNITS:" + Command.substring(9));
	} else if (Command.substring(0, 11) == "AT+HANDSUP=") {
		sendBLEcmd("AT+HANDSUP:" + Command.substring(11));
	} else if (Command.substring(0, 7) == "AT+SIT=") {
		sendBLEcmd("AT+SIT:" + Command.substring(7));
	} else if (Command.substring(0, 7) == "AT+LAN=") {
		sendBLEcmd("AT+LAN:ERR");
	} else if (Command.substring(0, 14) == "AT+TIMEFORMAT=") {
		sendBLEcmd("AT+TIMEFORMAT:" + Command.substring(14));
	} else if (Command == "AT+BATT") {
		sendBLEcmd("AT+BATT:" + String(getBatteryLevel()));
	} else if (Command == "BT+VER") {
		sendBLEcmd("BT+VER:" + btversionNr);
	} else if (Command == "AT+VER") {
		Serial.println("filtercmd at+ver");
		sendBLEcmd("AT+VER:" + versionNr);
	} else if (Command == "AT+SN") {
		sendBLEcmd("AT+SN:" + serialNr);
	} else if (Command.substring(0, 10) == "AT+DISMOD=") {
		sendBLEcmd("AT+DISMOD:" + Command.substring(10));
	} else if (Command.substring(0, 7) == "AT+LAN=") {
		sendBLEcmd("AT+LAN:ERR");
	} else if (Command.substring(0, 10) == "AT+MOTOR=1") {
		sendBLEcmd("AT+MOTOR:1" + Command.substring(10));
		digitalWrite(25, HIGH);
		delay(300);
		digitalWrite(25, LOW);
	} else if (Command.substring(0, 12) == "AT+CONTRAST=") {
		contrast = Command.substring(12).toInt();
	} else if (Command.substring(0, 6) == "AT+DT=") {
		SetDateTimeString(Command.substring(6));
		sendBLEcmd("AT+DT:" + GetDateTimeString());
	} else if (Command.substring(0, 5) == "AT+DT") {
		sendBLEcmd("AT+DT:" + GetDateTimeString());
	} else if (Command.substring(0, 12) == "AT+TIMEZONE=") {
		timezone = Command.substring(12).toInt();
		sendBLEcmd("AT+TIMEZONE:" + String(timezone));
	} else if (Command.substring(0, 11) == "AT+TIMEZONE") {
		sendBLEcmd("AT+TIMEZONE:" + String(timezone));
	} else if (Command == "AT+STEPSTORE") {
		sendBLEcmd("AT+STEPSTORE:OK");
	} else if (Command == "AT+TOPACE=1") {
		sendBLEcmd("AT+TOPACE:OK");
		sendBLEcmd("NT+TOPACE:" + String(steps));
	} else if (Command == "AT+TOPACE=0") {
		sendBLEcmd("AT+TOPACE:" + String(steps));
	} else if (Command == "AT+DATA=0") {
		sendBLEcmd("AT+DATA:0,0,0,0");
	} else if (Command.substring(0, 8) == "AT+PACE=") {
		steps1 = Command.substring(8).toInt();
		sendBLEcmd("AT+PACE:" + String(steps1));
	} else if (Command == "AT+PACE") {
		sendBLEcmd("AT+PACE:" + String(steps1));
	} else if (Command == "AT+DATA=1") {
		sendBLEcmd("AT+DATA:0,0,0,0");
	} else if (Command.substring(0, 7) == "AT+SYN=") {
		if (Command.substring(7) == "1") {
			sendBLEcmd("AT+SYN:1");
			syn = true;
		} else {
			sendBLEcmd("AT+SYN:0");
			syn = false;
		}
	}

}

void sendBLEcmd(String Command) {
	uint8_t buffer[20];
	for (int j=0; j++; j<20) buffer[j]=uint8_t(Command[j]);
	//for (int j=0; j++; j<20) buffer[j]=(uint8_t)atoi("J");
	//for (int j=0; j++; j<20) buf[j]=j;
	//Command.toCharArray(buffer,20);
	if (debug)Serial.print("TxBle: ");
	//if (debug)Serial.println(Command);
	Serial.println("Command sendBLEcmd");
	Serial.println(Command);
	Command = Command + "\r\n";
	//	while (Command.length() > 0) {
	uint8_t* TempSendCmd;
	//TempSendCmd=&Command;
	//		String TempCommand = Command.substring(0, 20);
	//TempSendCmd = &TempCommand[0];
	//TempSendCmd = &TempCommand;
	bleuart.write(&buffer[0],20);
	//bleuart.write(buf,20);
	//bleuart.write(&TempCommand,20);
	//    TXchar.setValue(TempSendCmd);
	//TXchar1.setValue(TempSendCmd);
	//		Command = Command.substring(20);
	//	}
}

String GetDateTimeString() {
	String datetime = String(year());
	if (month() < 10) datetime += "0";
	datetime += String(month());
	if (day() < 10) datetime += "0";
	datetime += String(day());
	if (hour() < 10) datetime += "0";
	datetime += String(hour());
	if (minute() < 10) datetime += "0";
	datetime += String(minute());
	return datetime;
}

void SetDateTimeString(String datetime) {
	int year = datetime.substring(0, 4).toInt();
	int month = datetime.substring(4, 6).toInt();
	int day = datetime.substring(6, 8).toInt();
	int hr = datetime.substring(8, 10).toInt();
	int min = datetime.substring(10, 12).toInt();
	int sec = datetime.substring(12, 14).toInt();
	setTime( hr, min, sec, day, month, year);
}

void handlePush(String pushMSG) {
	int commaIndex = pushMSG.indexOf(',');
	int secondCommaIndex = pushMSG.indexOf(',', commaIndex + 1);
	int lastCommaIndex = pushMSG.indexOf(',', secondCommaIndex + 1);
	String MsgText = pushMSG.substring(commaIndex + 1, secondCommaIndex);
	int timeShown = pushMSG.substring(secondCommaIndex + 1, lastCommaIndex).toInt();
	int SymbolNr = pushMSG.substring(lastCommaIndex + 1).toInt();
	msgText = MsgText;
	if (debug)Serial.println("MSGtext: " + msgText);
	if (debug)Serial.println("symbol: " + String(SymbolNr));
}

int getBatteryLevel() {
	return map(analogRead(3), 500, 715, 0, 100);
}

void setup() {
	Serial.begin(38400);
	Serial.println("I'm alive");
	pinMode(BUTTON_PIN, INPUT);
	pinMode(3, INPUT);
	if (digitalRead(BUTTON_PIN) == LOW) {
		NRF_POWER->GPREGRET = 0x01;
		sd_nvic_SystemReset();
	}
	pinMode(2, INPUT);
	pinMode(25, OUTPUT);
	digitalWrite(25, HIGH);
	pinMode(4, OUTPUT);
	digitalWrite(4, LOW);
	pinMode(15, INPUT);
	if (debug)Serial.begin(115200);
	wdt_enable(5000);
	/*  blePeripheral.setLocalName("DS-D6");
	    blePeripheral.setAdvertisingInterval(555);
	    blePeripheral.setAppearance(0x0000);
	    blePeripheral.setConnectable(true);
	    blePeripheral.setDeviceName("ATCDSD6");
	    blePeripheral.setAdvertisedServiceUuid(batteryLevelService.uuid());
	    blePeripheral.addAttribute(batteryLevelService);
	    blePeripheral.addAttribute(TXchar);
	    blePeripheral.addAttribute(RXchar);
	    RXchar.setEventHandler(BLEWritten, characteristicWritten);
	    blePeripheral.setAdvertisedServiceUuid(batteryLevelService1.uuid());
	    blePeripheral.addAttribute(batteryLevelService1);
	 */
	//  blePeripheral.addAttribute(TXchar1);
	//  blePeripheral.addAttribute(RXchar1);
	//  RXchar1.setEventHandler(BLEWritten, characteristicWritten);
	//  blePeripheral.setEventHandler(BLEConnected, blePeripheralConnectHandler);
	//  blePeripheral.setEventHandler(BLEDisconnected, blePeripheralDisconnectHandler);
	//  blePeripheral.begin();
	Bluefruit.begin();
	Serial.println("Bluefruit bluetooth library Start");

	Bluefruit.setName("Open Source Watch");
	// Set the connect/disconnect callback handlers
	Bluefruit.Periph.setConnectCallback(connect_callback);
	Bluefruit.Periph.setDisconnectCallback(disconnect_callback);



	// Configure and Start the Device Information Service
	Serial.println("Configuring the Device Information Service");
	bledis.setManufacturer("the commons");
	bledis.setModel("De-esse six");
	bledis.begin();
	bleuart.begin();
	startAdv();

	attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonHandler, FALLING);
	attachInterrupt(digitalPinToInterrupt(15), acclHandler, RISING);
	NRF_GPIO->PIN_CNF[15] &= ~((uint32_t)GPIO_PIN_CNF_SENSE_Msk);
	NRF_GPIO->PIN_CNF[15] |= ((uint32_t)GPIO_PIN_CNF_SENSE_High << GPIO_PIN_CNF_SENSE_Pos);
	attachInterrupt(digitalPinToInterrupt(2), charge, RISING);
	NRF_GPIO->PIN_CNF[2] &= ~((uint32_t)GPIO_PIN_CNF_SENSE_Msk);
	NRF_GPIO->PIN_CNF[2] |= ((uint32_t)GPIO_PIN_CNF_SENSE_High << GPIO_PIN_CNF_SENSE_Pos);
	display.begin(SSD1306_SWITCHCAPVCC);
	delay(100);
	display.clearDisplay();
	// display.setFont(&FreeSerifItalic9pt7b);
	display.display();
	display.setTextSize(1);
	display.setTextColor(WHITE);
	display.setCursor(10, 0);
	display.println("D6 Emulator");
	display.display();
	digitalWrite(25, LOW);
	sd_power_mode_set(NRF_POWER_MODE_LOWPWR);
	initRTC2();
	initi2c();
	initkx023();

	uint8_t i, j, bytes;
	memset(img, 0, sizeof(img)); // Clear the img[] array
	for (i = 0; i < N_GRAINS; i++) { // For each sand grain...
		do {
			grain[i].x = random(WIDTH  * 256); // Assign random position within
			grain[i].y = random(HEIGHT * 256); // the 'grain' coordinate space
			// Check if corresponding pixel position is already occupied...
			for (j = 0; (j < i) && (((grain[i].x / 256) != (grain[j].x / 256)) ||
						((grain[i].y / 256) != (grain[j].y / 256))); j++);
		} while (j < i); // Keep retrying until a clear spot is found
		img[(grain[i].y / 256) * WIDTH + (grain[i].x / 256)] = 255; // Mark it
		grain[i].pos = (grain[i].y / 256) * WIDTH + (grain[i].x / 256);
		grain[i].vx = grain[i].vy = 0; // Initial velocity is zero
	}
}




void startAdv(void)
{
	// Advertising packet
	Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
	Bluefruit.Advertising.addTxPower();

	// Include HRM Service UUID
	//	Bluefruit.Advertising.addService(hrms);
	//	Bluefruit.Advertising.addService(tijd);
	// Include CTS client UUID
	Bluefruit.Advertising.addService(bleuart);
	//Bluefruit.Advertising.addService(bleCTime);


	// Include Name
	Bluefruit.Advertising.addName();

	/* Start Advertising
	 * - Enable auto advertising if disconnected
	 * - Interval:  fast mode = 20 ms, slow mode = 152.5 ms
	 * - Timeout for fast mode is 30 seconds
	 * - Start(timeout) with timeout = 0 will advertise forever (until connected)
	 * 
	 * For recommended advertising interval
	 * https://developer.apple.com/library/content/qa/qa1931/_index.html   
	 */
	Bluefruit.Advertising.restartOnDisconnect(true);
	Bluefruit.Advertising.setInterval(32, 244);    // in unit of 0.625 ms
	Bluefruit.Advertising.setFastTimeout(30);      // number of seconds in fast mode
	Bluefruit.Advertising.start(0);                // 0 = Don't stop advertising after n seconds  
}




void connect_callback(uint16_t conn_handle)
{
	// Get the reference to current connection
	BLEConnection* connection = Bluefruit.Connection(conn_handle);

	char central_name[32] = { 0 };
	connection->getPeerName(central_name, sizeof(central_name));

	Serial.print("Connected to ");
	Serial.println(central_name);
	/*	if ( bleCTime.discover(conn_handle) )
		{
		bleCTime.enableAdjust();
		Serial.println("receiving time");
		bleCTime.getCurrentTime();
		bleCTime.getLocalTimeInfo();
		Serial.println("weekday");
		Serial.print(bleCTime.Time.weekday);
		}	
		else
		Serial.println("not receiving time");
	 */
}


/**
 * Callback invoked when a connection is dropped
 * @param conn_handle connection where this event happens
 * @param reason is a BLE_HCI_STATUS_CODE which can be found in ble_hci.h
 */
void disconnect_callback(uint16_t conn_handle, uint8_t reason)
{
	(void) conn_handle;
	(void) reason;

	Serial.println("Disconnected");
	Serial.println("Advertising!");
}



void loop() {
	// blePeripheral.poll();
	int teller=0;
	Serial.println("loop starts");
	char remoteCharArray[21];
	if (Serial.available())
	{
		// Delay to get enough input data since we have a
		// limited amount of space in the transmit buffer
		delay(2);

		uint8_t buf[64];
		int count = Serial.readBytes(buf, sizeof(buf));
	Serial.println(count);
		bleuart.write( buf, count );
	}


/*
	while ( bleuart.available() )
	{
		uint8_t ch;
		ch = (uint8_t) bleuart.read();
		bleuart.write(ch);
		if (ch == '\r') Serial.println("slash r");

		if (ch == '\n') Serial.println("slash n");
		//	Serial.println(" blueart");
		Serial.write(ch);
		remoteCharArray[teller]=ch;
		teller++;
		Serial.println(teller);
	}
	*/
	tempLen1 = teller;
	tempLen = tempLen1;
	//	memset(remoteCharArray, 0, sizeof(remoteCharArray));
	//	memcpy(remoteCharArray, characteristic.value(), tempLen1);
	tempCmd = remoteCharArray;
	if (remoteCharArray[tempLen - 2] == '\r' && remoteCharArray[tempLen - 1] == '\n') {
		answer = tempCmd.substring(0, tempLen - 2);
		Serial.println("answer");
		Serial.println(answer);
		tempCmd = "";
		tempLen = 0;
		//if (debug)Serial.print("RxBle: ");
		Serial.print("RxBle: ");
		//if (debug)Serial.println(answer);
		Serial.println(answer);
//		filterCmd(answer);
	}
	if (tempLen > 1)	
	{
		Serial.println("string voldoet niet aan voorwaarden ");
		Serial.println(remoteCharArray);
	}

	wdt_reset();
	if (sleeping) {
		sd_app_evt_wait();
		sd_nvic_ClearPendingIRQ(SD_EVT_IRQn);
	} else {

		switch (menu) {
			case 0:
				if (millis() - displayRefreshTime > refreshRate) {
					displayRefreshTime = millis();
					displayMenu0();
				}
				break;
			case 1:
				if (millis() - displayRefreshTime > refreshRate) {
					displayRefreshTime = millis();
					displayMenu1();
				}
				break;
			case 2:
				if (millis() - displayRefreshTime > refreshRate) {
					displayRefreshTime = millis();
					displayMenu2();
				}
				break;
			case 3:
				if (millis() - displayRefreshTime > refreshRate) {
					displayRefreshTime = millis();
					displayMenu3();
				}
				break;
			case 4:
				if (millis() - displayRefreshTime > refreshRate) {
					displayRefreshTime = millis();
					displayMenu4();
				}
				break;
			case 5:
				displayMenu5();
				break;
			case 77:
				if (millis() - displayRefreshTime > refreshRate) {
					displayRefreshTime = millis();
					displayMenu77();
				}
				break;
			case 88:
				if (millis() - displayRefreshTime > refreshRate) {
					displayRefreshTime = millis();
					displayMenu88();
				}
				break;
			case 99:
				if (millis() - displayRefreshTime > refreshRate) {
					displayRefreshTime = millis();
					displayMenu99();
				}
				break;
		}
		if (buttonPressed) {
			buttonPressed = false;
			switch (menu) {
				case 0:
					menu = 1;
					break;
				case 1:
					menu = 2;
					break;
				case 2:
					menu = 3;
					break;
				case 3:
					menu = 4;
					break;
				case 4:
					startbutton = millis();
					while (!digitalRead(BUTTON_PIN)) {}
					if (millis() - startbutton > 1000) {
						delay(100);
						int err_code = sd_power_gpregret_set(0,0x01);
						sd_nvic_SystemReset();
						while (1) {};
					} else {
						menu = 5;
					}
					break;
				case 5:
					menu = 0;
					break;
				case 77:
					menu = 0;
					break;
				case 88:
					menu = 0;
					break;
				case 99:
					digitalWrite(25, LOW);
					menu = 0;
					break;
			}
		}
		switch (menu) {
			case 0:
				if (millis() - sleepTime > sleepDelay ) powerDown();
				break;
			case 1:
				if (millis() - sleepTime > sleepDelay ) powerDown();
				break;
			case 2:
				if (millis() - sleepTime > sleepDelay ) powerDown();
				break;
			case 3:
				if (millis() - sleepTime > sleepDelay ) powerDown();
				break;
			case 4:
				if (millis() - sleepTime > sleepDelay ) powerDown();
				break;
			case 5:
				if (millis() - sleepTime > 20000 ) powerDown();
				break;
			case 77:
				if (millis() - sleepTime > 3000 ) powerDown();
				break;
			case 88:
				if (millis() - sleepTime > 3000 ) powerDown();
				break;
			case 99:
				if (millis() - sleepTime > 6000 ) {
					digitalWrite(25, LOW);
					powerDown();
				}
				break;
		}
	}
}

void displayMenu0() {
	display.setRotation(0);
	display.clearDisplay();
	display.setCursor(0, 0);
	display.print(bleSymbol);
	display.println(" Time and Batt:");
	display.println(GetDateTimeString() + String(second()));
	display.print(getBatteryLevel());
	display.print("%  ");
	display.println(analogRead(3));
	display.println(contrast);
	display.display();
}

void displayMenu1() {
	display.setRotation(0);
	display.clearDisplay();
	display.setCursor(0, 0);
	display.println("Menue1 Mac:");
	char tmp[16];
	sprintf(tmp, "%04X", NRF_FICR->DEVICEADDR[1] & 0xffff);
	String MyID = tmp;
	sprintf(tmp, "%08X", NRF_FICR->DEVICEADDR[0]);
	MyID += tmp;
	display.println(MyID);
	display.display();
}

void displayMenu2() {
	display.setRotation(0);
	uint8_t res[6];
	softbeginTransmission(0x1F);
	softwrite(0x06);
	softendTransmission();
	softrequestFrom(0x1F , 6);
	res[0] = softread();
	res[1] = softread();
	res[2] = softread();
	res[3] = softread();
	res[4] = softread();
	res[5] = softread();
	byte x = (int16_t)((res[1] << 8) | res[0]) / 128;
	byte y = (int16_t)((res[3] << 8) | res[2]) / 128;
	byte z = (int16_t)((res[5] << 8) | res[4]) / 128;

	display.clearDisplay();
	display.setCursor(0, 0);
	display.println("Menue2 PushMSG:");
	display.println(msgText);
	display.print(x);
	display.print(",");
	display.print(y);
	display.print(",");
	display.println(z);
	display.display();
}

void displayMenu3() {
	display.setRotation(0);
	display.clearDisplay();
	display.setCursor(0, 0);
	display.print("CMD Length: ");
	display.println(answer.length());
	display.println(answer);
	display.display();
}
void displayMenu4() {
	display.setRotation(0);
	display.clearDisplay();
	display.setCursor(0, 0);
	display.println("Hello From Arduino");
	display.println("  :)");
	display.println("Hold for Bootloader");
	display.display();
}
void displayMenu77() {
	display.setRotation(3);
	display.clearDisplay();
	display.setCursor(0, 0);
	display.println(bleSymbol);
	display.setCursor(0, 11);
	display.print(getBatteryLevel());
	display.println("%");

	display.setTextSize(2);
	display.setCursor(4, 30);
	if (hour() < 10) display.print("0");
	display.println(hour());
	display.setCursor(4, 50);
	if (minute() < 10) display.print("0");
	display.println(minute());
	display.setCursor(4, 70);
	if (second() < 10) display.print("0");
	display.println(second());

	display.setCursor(0, 111);
	display.setTextSize(1);
	if (day() < 10) display.print("0");
	display.print(day());
	display.print(".");
	if (month() < 10) display.print("0");
	display.println(month());
	display.print(".");
	display.println(year());

	display.display();
}
void displayMenu88() {
	display.setRotation(3);
	display.clearDisplay();
	display.setCursor(0, 30);
	display.println("Charge");
	display.display();
}
void displayMenu99() {
	display.setRotation(0);
	digitalWrite(25, vibrationMode);
	if (vibrationMode)vibrationMode = false; else vibrationMode = true;
	display.clearDisplay();
	display.setCursor(0, 0);
	display.println(msgText);
	display.display();
}
void displayMenu5() {
	uint32_t t;
	while (((t = micros()) - prevTime) < (1000000L / MAX_FPS));
	prevTime = t;

	uint8_t res[6];
	softbeginTransmission(0x1F);
	softwrite(0x06);
	softendTransmission();
	softrequestFrom(0x1F , 6);
	res[0] = softread();
	res[1] = softread();
	res[2] = softread();
	res[3] = softread();
	res[4] = softread();
	res[5] = softread();
	int x = (int16_t)((res[1] << 8) | res[0]);
	int y = (int16_t)((res[3] << 8) | res[2]);
	int z = (int16_t)((res[5] << 8) | res[4]);

	float accelX = y;
	float accelY = -x;
	float accelZ = z;
	int16_t ax = -accelY / 256,      // Transform accelerometer axes
		ay =  accelX / 256,      // to grain coordinate space
		az = abs(accelZ) / 2048; // Random motion factor
	az = (az >= 3) ? 1 : 4 - az;      // Clip & invert
	ax -= az;                         // Subtract motion factor from X, Y
	ay -= az;
	int16_t az2 = az * 2 + 1;         // Range of random motion to add back in

	int32_t v2; // Velocity squared
	float   v;  // Absolute velocity
	for (int i = 0; i < N_GRAINS; i++) {
		grain[i].vx += ax + random(az2); // A little randomness makes
		grain[i].vy += ay + random(az2); // tall stacks topple better!
		v2 = (int32_t)grain[i].vx * grain[i].vx + (int32_t)grain[i].vy * grain[i].vy;
		if (v2 > 65536) { // If v^2 > 65536, then v > 256
			v = sqrt((float)v2); // Velocity vector magnitude
			grain[i].vx = (int)(256.0 * (float)grain[i].vx / v); // Maintain heading
			grain[i].vy = (int)(256.0 * (float)grain[i].vy / v); // Limit magnitude
		}
	}

	uint16_t        i, bytes, oldidx, newidx, delta;
	int16_t        newx, newy;

	for (i = 0; i < N_GRAINS; i++) {
		newx = grain[i].x + grain[i].vx; // New position in grain space
		newy = grain[i].y + grain[i].vy;
		if (newx > MAX_X) {              // If grain would go out of bounds
			newx         = MAX_X;          // keep it inside, and
			grain[i].vx /= -2;             // give a slight bounce off the wall
		} else if (newx < 0) {
			newx         = 0;
			grain[i].vx /= -2;
		}
		if (newy > MAX_Y) {
			newy         = MAX_Y;
			grain[i].vy /= -2;
		} else if (newy < 0) {
			newy         = 0;
			grain[i].vy /= -2;
		}

		oldidx = (grain[i].y / 256) * WIDTH + (grain[i].x / 256); // Prior pixel #
		newidx = (newy      / 256) * WIDTH + (newx      / 256); // New pixel #
		if ((oldidx != newidx) && // If grain is moving to a new pixel...
				img[newidx]) {       // but if that pixel is already occupied...
			delta = abs(newidx - oldidx); // What direction when blocked?
			if (delta == 1) {           // 1 pixel left or right)
				newx         = grain[i].x;  // Cancel X motion
				grain[i].vx /= -2;          // and bounce X velocity (Y is OK)
				newidx       = oldidx;      // No pixel change
			} else if (delta == WIDTH) { // 1 pixel up or down
				newy         = grain[i].y;  // Cancel Y motion
				grain[i].vy /= -2;          // and bounce Y velocity (X is OK)
				newidx       = oldidx;      // No pixel change
			} else { // Diagonal intersection is more tricky...
				if ((abs(grain[i].vx) - abs(grain[i].vy)) >= 0) { // X axis is faster
					newidx = (grain[i].y / 256) * WIDTH + (newx / 256);
					if (!img[newidx]) { // That pixel's free!  Take it!  But...
						newy         = grain[i].y; // Cancel Y motion
						grain[i].vy /= -2;         // and bounce Y velocity
					} else { // X pixel is taken, so try Y...
						newidx = (newy / 256) * WIDTH + (grain[i].x / 256);
						if (!img[newidx]) { // Pixel is free, take it, but first...
							newx         = grain[i].x; // Cancel X motion
							grain[i].vx /= -2;         // and bounce X velocity
						} else { // Both spots are occupied
							newx         = grain[i].x; // Cancel X & Y motion
							newy         = grain[i].y;
							grain[i].vx /= -2;         // Bounce X & Y velocity
							grain[i].vy /= -2;
							newidx       = oldidx;     // Not moving
						}
					}
				} else { // Y axis is faster, start there
					newidx = (newy / 256) * WIDTH + (grain[i].x / 256);
					if (!img[newidx]) { // Pixel's free!  Take it!  But...
						newx         = grain[i].x; // Cancel X motion
						grain[i].vy /= -2;         // and bounce X velocity
					} else { // Y pixel is taken, so try X...
						newidx = (grain[i].y / 256) * WIDTH + (newx / 256);
						if (!img[newidx]) { // Pixel is free, take it, but first...
							newy         = grain[i].y; // Cancel Y motion
							grain[i].vy /= -2;         // and bounce Y velocity
						} else { // Both spots are occupied
							newx         = grain[i].x; // Cancel X & Y motion
							newy         = grain[i].y;
							grain[i].vx /= -2;         // Bounce X & Y velocity
							grain[i].vy /= -2;
							newidx       = oldidx;     // Not moving
						}
					}
				}
			}
		}
		grain[i].x  = newx; // Update grain position
		grain[i].y  = newy;
		img[oldidx] = 0;    // Clear old spot (might be same as new, that's OK)
		img[newidx] = 255;  // Set new spot
		grain[i].pos = newidx;
	}

	display.clearDisplay();
	for (i = 0; i < N_GRAINS; i++) {
		int yPos = grain[i].pos / WIDTH;
		int xPos = grain[i].pos % WIDTH;
		display.drawPixel(xPos , yPos, WHITE);
	}
	display.display();
}
