#include "kx023.h"


const int portForKX023 = 1;
PortI2C myBus(portForKX023);
PulsePlug pulse(myBus);




void setup() {

	Serial.begin(38400);
	uint8_t value;
	Serial.print("setting up ");

	delay(300);
	pulse.initPulsePlug();
/*


int kx023_init(void)
{
	int err = 0;
	uint8_t temp;
	//soft reset
	//kx023_command(CNTL2,0x00);
	kx023_command(CNTL2,0x80);
	HAL_Delay(5);
	temp = kx023_read_reg(WHOAMI);
	if (temp != 0x15)
		err += 1;
	temp = kx023_read_reg(COTR);
	if (temp != 0x55)
		err += 2;
	return err; //todo return error code jj
};


void kx023_standby(void)
{
	uint8_t temp = 0;

	temp = kx023_read_reg(CNTL1);
	temp &= 0x7F;
	kx023_command(CNTL1,temp);
};

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
