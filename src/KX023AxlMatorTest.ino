#include "kx023.h"


const int portForKX023 = 1; // JJ
PortI2C myBus(portForKX023);
PulsePlug pulse(myBus);





const int sendorDeviceAddress = 0x1F;//I2C7bitAddressMode
const int regAddressXOUTL = 0x06;
const int regAddressYOUTL = 0x08;
const int regAddressZOUTL = 0x0A;
const int regAddressODCNTL = 0x1B;
const int regAddressCNTL1 = 0x18;


void setup() {

	Serial.begin(38400);
	uint8_t value;
	Serial.println("data available: 0x00");
	Serial.print(value);

	Serial.print("setting up ");

	delay(300);
	pulse.setReg(0x18, 0x40);
	pulse.setReg(0x1B, 0x02);
	pulse.setReg(0x18, 0xC0);
	Serial.print("setting up LPF");

	//setup LPF--------------------------------------------------

	//set device standbyMode
	//readCNTL1reg
	byte CNTL1 = 0;
	CNTL1=pulse.getReg(regAddressCNTL1);
	Serial.print("CNTL1 ");
	Serial.println(CNTL1);
	//setCNTL1reg

	CNTL1 = CNTL1 & 0b01111111;
	pulse.setReg(regAddressCNTL1,  CNTL1);
	Serial.print("setting up LPF parameters");
	//set LPF parameters
	//readODCNTLreg
	byte ODCNTL = 0;
	ODCNTL=pulse.getReg(regAddressODCNTL);
	Serial.print("ODCNTL ");
	Serial.println(ODCNTL );
	//setODCNTLreg
	ODCNTL = ODCNTL | 0b01000000;//set filter corner frequency set to ODR/2
	ODCNTL = ODCNTL & 0b11110000;//set OutputDataRate 12.5Hz
	pulse.setReg(regAddressODCNTL, ODCNTL);

	Serial.print("setting up device operating mode");
	//set device operating mode
	//readCNTL1reg
	CNTL1=pulse.getReg(regAddressCNTL1);
	Serial.print("CNTL1 ");
	Serial.println(CNTL1);
	//setCNTL1re""g
	CNTL1 = CNTL1 | 0b10000000;
	pulse.setReg(regAddressCNTL1, CNTL1);
	//--------------------------------------------------setup LPF

}

void loop() {
	int xData,zData,yData;	
	//readXout
	xData=pulse.getCoor(regAddressXOUTL);
	yData=pulse.getCoor(regAddressYOUTL);
	zData=pulse.getCoor(regAddressZOUTL);
	//  //rawDataOutput
	Serial.print("xdata:");
	Serial.print(xData, DEC);
	Serial.print(",");
	Serial.print("ydata:");
	Serial.print(yData, DEC);
	Serial.print(",");
	Serial.print("zdata:");
	Serial.println(zData, DEC);


	delay(100);
}
