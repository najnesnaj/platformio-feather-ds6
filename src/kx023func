/*
 * kx023.c
 *
 *  Created on: Jan 28, 2017
 *      Author: gudozhni
 */

#include "kx023.h"
#include <string.h>

extern SPI_HandleTypeDef hspi3;

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
	return err;
};

int kx023_command(uint8_t addr, uint8_t val)
{
	int err = 0;
	uint8_t temp[2];
	temp[0] = addr;
	temp[1] = val;
	GPIOA->BRR = ACC_CS_PIN;
	asm("NOP");
	asm("NOP");
	HAL_SPI_Transmit(&hspi3, temp, 2, 500);
	GPIOA->BSRR = ACC_CS_PIN;
//	asm("NOP");
//	asm("NOP");

	return err;

};

uint8_t kx023_read_reg(uint8_t addr)
{
	uint8_t TxData = 0x00;
	uint8_t RxData = 0x00;
	TxData = 0x80|addr;

	GPIOA->BRR = ACC_CS_PIN;
	HAL_SPI_Transmit(&hspi3, &TxData, 1, 500);
	TxData = 0x00;
	HAL_SPIEx_FlushRxFifo(&hspi3);
	HAL_SPI_TransmitReceive(&hspi3,&TxData, &RxData, 1, 500);
	GPIOA->BSRR = ACC_CS_PIN;

	return RxData;
};

void kx023_int1_config(uint8_t int_en, uint8_t int_pol, uint8_t int_mask){

};
void kx023_int2_config(uint8_t int_en, uint8_t int_pol, uint8_t int_mask){

};
void kx023_buf_config(uint8_t buf_en, uint8_t watermark_level, uint8_t resolution, uint8_t buf_mode){

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

void kx023_TILT_setup(void){

};
