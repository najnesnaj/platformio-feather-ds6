#include "kx023.h"


const int portForKX023 = 1;
PortI2C myBus(portForKX023);
PulsePlug pulse(myBus);




void setup() {

	Serial.begin(38400);
	uint8_t value;
	Serial.print("setting up ");

	delay(300);
	pulse.initPulsePlugTap();
/*
void kx023_wake(void)
{
	uint8_t temp = 0;

	temp = kx023_read_reg(CNTL1);
	temp |= 0x80;
	kx023_command(CNTL1,temp);
};

void kx023_WUM_setup(void)
{
	kx023_command(CNTL1, WUM_EN + MAX_4G);
	//data rate
	kx023_command(CNTL3,0x03); //6.25 Hz
	//directions
	kx023_command(INC2,0x3F);
	//delay
	kx023_command(WUFC, 0x01);
	// threshold
	kx023_command(ATH, 0x20);
	// interrupts
	kx023_command(INC1,INT1_EN + HIGH_POL + INT_LATCH); // INT1 pin behavior
	kx023_command(INC4,WUM_EN); // routing of interrupts on INT1
	// run
	kx023_command(CNTL1, WUM_EN + MAX_4G + RUN);

};

void kx023_TAP_setup(void)
{
	kx023_command(CNTL1, TAP_EN + HRES_EN + MAX_4G);
	//data rate
	kx023_command(CNTL3,0x18); //200 Hz
	//directions
	kx023_command(INC3,0x3F);
	//single tap and/or double tap
	kx023_command(TDTRC, 0x03);
	// 1tap timing
	kx023_command(TDTC, 0x58);
	// low thresh
	kx023_command(TTL, 0x2A);
	// high thresh
	kx023_command(TTH, 250);
	// any tap timing
	kx023_command(FTD, 0xA2);
	// second tap timing
	kx023_command(STD, 0x24);
	// any tap timing
	kx023_command(TLT, 0x28);
	// any tap timing
	kx023_command(TWS, 0xA0);


	// interrupts
	kx023_command(INC1, INT1_EN + HIGH_POL + INT_LATCH); // INT1 pin behavior
	kx023_command(INC4, TAP_EN); // routing of interrupts on INT1
	// run
	kx023_command(CNTL1, TAP_EN + HRES_EN + MAX_2G + RUN);

};
*/
}

void loop() {
	byte tapinter;	
	tapinter=pulse.getReg(0x15); //STAT register:w
	if ((tapinter && 0b00001000) == 1) Serial.println ("tap detected");
	tapinter=pulse.getReg(0x13); //INS2 register bit 3 & 4 distinguis double/single tap
	if ((tapinter && 0b00000010) == 1) Serial.println ("single tap detected");
	if ((tapinter && 0b00000100) == 1) Serial.println (" double tap detected");

	//BUF_STATUS_1 0X3A
	//BUF_STATUS_2 0X3B
//	Serial.print("interrupts:");
//	Serial.println(tapinter);
//	tapinter=pulse.getReg(0x3A); //STAT register:w
//	Serial.print("interrupts:");
//	Serial.println(tapinter);
//	tapinter=pulse.getReg(0x3B); //STAT register:w
//	Serial.print("interrupts:");
//	Serial.println(tapinter);
	tapinter=pulse.getReg(0x17); //has to be read to release register


	delay(100);
}
