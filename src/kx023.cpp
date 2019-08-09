#include "kx023.h"

// flag bits sent to the receiver
void PulsePlug::initPulsePlug(){
	PulsePlug::setReg(regAddressCNTL1 , 0x40);
	PulsePlug::setReg(regAddressODCNTL, 0x02);
	PulsePlug::setReg(regAddressCNTL1 , 0xC0);

//	Serial.print("setting up LPF");

	//setup LPF--------------------------------------------------

	//set device standbyMode
	//readCNTL1reg
	byte CNTL1 = 0;
	//CNTL1=pulse.getReg(regAddressCNTL1);
	CNTL1=PulsePlug::getReg(regAddressCNTL1);
//	Serial.print("CNTL1 ");
//	Serial.println(CNTL1);
	//setCNTL1reg

	CNTL1 = CNTL1 & 0b01111111;
	//pulse.setReg(regAddressCNTL1,  CNTL1);
	PulsePlug::setReg(regAddressCNTL1,  CNTL1);
//	Serial.print("setting up LPF parameters");
	//set LPF parameters
	//readODCNTLreg
	byte ODCNTL = 0;
	ODCNTL=PulsePlug::getReg(regAddressODCNTL);
//	Serial.print("ODCNTL ");
//	Serial.println(ODCNTL );
	//setODCNTLreg
	ODCNTL = ODCNTL | 0b01000000;//set filter corner frequency set to ODR/2
	ODCNTL = ODCNTL & 0b11110000;//set OutputDataRate 12.5Hz
	PulsePlug::setReg(regAddressODCNTL, ODCNTL);

//	Serial.print("setting up device operating mode");
	//set device operating mode
	//readCNTL1reg
	CNTL1=PulsePlug::getReg(regAddressCNTL1);
//	Serial.print("CNTL1 ");
//	Serial.println(CNTL1);
	//setCNTL1re""g
	CNTL1 = CNTL1 | 0b10000000;
	PulsePlug::setReg(regAddressCNTL1, CNTL1);
	//--------------------------------------------------setup LPF


}



void PulsePlug::initPulsePlugTap(){
	PulsePlug::setReg(regAddressCNTL1 , 0x44);
	PulsePlug::setReg(regAddressODCNTL, 0x3F);
	PulsePlug::setReg(regAddressCNTL1 , 0x98);
	PulsePlug::setReg(regAddressTDTRC , 0x03);
	PulsePlug::setReg(regAddressTDTC , 0x78); //0.3 sec min between first and second tap 
	PulsePlug::setReg(regAddressTTH , 0xCB); // tap threshold
	PulsePlug::setReg(regAddressTTL , 0x1A); // any tap event 
	PulsePlug::setReg(regAddressFTD , 0xA2); // double tap event 
	PulsePlug::setReg(regAddressSTD , 0x24); //  
	PulsePlug::setReg(regAddressTLT , 0x28); //  
	PulsePlug::setReg(regAddressTWS , 0xA0); //  
	PulsePlug::setReg(regAddressINC1 , 0x30); //  
	PulsePlug::setReg(regAddressINC4 , 0x04); //  
	PulsePlug::setReg(regAddressCNTL1 , 0xC4);
//	Serial.print("setting up LPF");

	//setup LPF--------------------------------------------------

	//set device standbyMode
	//readCNTL1reg
	byte CNTL1 = 0;
	//CNTL1=pulse.getReg(regAddressCNTL1);
	CNTL1=PulsePlug::getReg(regAddressCNTL1);
//	Serial.print("CNTL1 ");
//	Serial.println(CNTL1);
	//setCNTL1reg

	CNTL1 = CNTL1 & 0b01111111;
	//pulse.setReg(regAddressCNTL1,  CNTL1);
	PulsePlug::setReg(regAddressCNTL1,  CNTL1);
//	Serial.print("setting up LPF parameters");
	//set LPF parameters
	//readODCNTLreg
	byte ODCNTL = 0;
	ODCNTL=PulsePlug::getReg(regAddressODCNTL);
//	Serial.print("ODCNTL ");
//	Serial.println(ODCNTL );
	//setODCNTLreg
	ODCNTL = ODCNTL | 0b01000000;//set filter corner frequency set to ODR/2
	ODCNTL = ODCNTL & 0b11110000;//set OutputDataRate 12.5Hz
	PulsePlug::setReg(regAddressODCNTL, ODCNTL);

//	Serial.print("setting up device operating mode");
	//set device operating mode
	//readCNTL1reg
	CNTL1=PulsePlug::getReg(regAddressCNTL1);
//	Serial.print("CNTL1 ");
//	Serial.println(CNTL1);
	//setCNTL1re""g
	CNTL1 = CNTL1 | 0b10000000;
	PulsePlug::setReg(regAddressCNTL1, CNTL1);
	//--------------------------------------------------setup LPF


}

int16_t PulsePlug::getCoor (byte reg) {
	// get a register
	union data {
		int16_t data16;
		byte  byteStr[2];//{*DATAL,*DATAH}
	};
	union data cdata;
	send();    
	write(reg);
	receive();
	//int result = ((read(0)<<8) && read(0));
	cdata.byteStr[0] =  read(0);
	cdata.byteStr[1] =  read(0);
	read(1);
	stop();
	delay(10);
	return cdata.data16;
}

byte PulsePlug::getReg (byte reg) {
	// get a register
	send();    
	write(reg);
	receive();
	byte result = read(1);
	stop();
	delay(10);
	return result;
}


void PulsePlug::setReg (byte reg, byte val) {
	// set a register
	send();    
	write(reg);
	write(val);
	stop();
	delay(10);
}

uint16_t Port::shiftRead(uint8_t bitOrder, uint8_t count) const {
	uint16_t value = 0, mask = bit(LSBFIRST ? 0 : count - 1);
	for (uint8_t i = 0; i < count; ++i) {
		digiWrite2(1);
		delayMicroseconds(5);
		if (digiRead())
			value |= mask;
		if (bitOrder == LSBFIRST)
			mask <<= 1;
		else
			mask >>= 1;
		digiWrite2(0);
		delayMicroseconds(5);
	}
	return value;
}

void Port::shiftWrite(uint8_t bitOrder, uint16_t value, uint8_t count) const {
	uint16_t mask = bit(LSBFIRST ? 0 : count - 1);
	for (uint8_t i = 0; i < count; ++i) {
		digiWrite((value & mask) != 0);
		if (bitOrder == LSBFIRST)
			mask <<= 1;
		else
			mask >>= 1;
		digiWrite2(1);
		digiWrite2(0);
	}
}


	PortI2C::PortI2C (uint8_t num, uint8_t rate)
: Port (num), uswait (rate)
{
	sdaOut(1);
	mode2(OUTPUT);
	sclHi();
}

uint8_t PortI2C::start(uint8_t addr) const {
	sclLo();
	sclHi();
	sdaOut(0);
	return write(addr);
}

void PortI2C::stop() const {
	sdaOut(0);
	sclHi();
	sdaOut(1);
}

uint8_t PortI2C::write(uint8_t data) const {
	sclLo();
	for (uint8_t mask = 0x80; mask != 0; mask >>= 1) {
		sdaOut(data & mask);
		sclHi();
		sclLo();
	}
	sdaOut(1);
	sclHi();
	uint8_t ack = ! sdaIn();
	sclLo();
	return ack;
}

uint8_t PortI2C::read(uint8_t last) const {
	uint8_t data = 0;
	for (uint8_t mask = 0x80; mask != 0; mask >>= 1) {
		sclHi();
		if (sdaIn())
			data |= mask;
		sclLo();
	}
	sdaOut(last);
	sclHi();
	sclLo();
	if (last)
		stop();
	sdaOut(1);
	return data;
}

bool DeviceI2C::isPresent () const {
	byte ok = send();
	stop();
	return ok;
}

byte MilliTimer::poll(word ms) {
	byte ready = 0;
	if (armed) {
		word remain = next - millis();
		// since remain is unsigned, it will overflow to large values when
		// the timeout is reached, so this test works as long as poll() is
		// called no later than 5535 millisecs after the timer has expired
		if (remain <= 60000)
			return 0;
		// return a value between 1 and 255, being msecs+1 past expiration
		// note: the actual return value is only reliable if poll() is
		// called no later than 255 millisecs after the timer has expired
		ready = -remain;
	}
	set(ms);
	return ready;
}

word MilliTimer::remaining() const {
	word remain = armed ? next - millis() : 0;
	return remain <= 60000 ? remain : 0;
}

void MilliTimer::set(word ms) {
	armed = ms != 0;
	if (armed)
		next = millis() + ms - 1;
}

Scheduler::Scheduler (byte size) : maxTasks (size), remaining (~0) {
	byte bytes = size * sizeof *tasks;
	tasks = (word*) malloc(bytes);
	memset(tasks, 0xFF, bytes);
}

Scheduler::Scheduler (word* buf, byte size) : tasks (buf), maxTasks (size), remaining(~0) {
	byte bytes = size * sizeof *tasks;
	memset(tasks, 0xFF, bytes);
}

char Scheduler::poll() {
	// all times in the tasks array are relative to the "remaining" value
	// i.e. only remaining counts down while waiting for the next timeout
	if (remaining == 0) {
		word lowest = ~0;
		for (byte i = 0; i < maxTasks; ++i) {
			if (tasks[i] == 0) {
				tasks[i] = ~0;
				return i;
			}
			if (tasks[i] < lowest)
				lowest = tasks[i];
		}
		if (lowest != ~0) {
			for (byte i = 0; i < maxTasks; ++i) {
				if(tasks[i] != ~0) {
					tasks[i] -= lowest;
				}
			}
		}
		remaining = lowest;
	} else if (remaining == ~0) //remaining == ~0 means nothing running
		return -2;
	else if (ms100.poll(100))
		--remaining;
	return -1;
}


void Scheduler::timer(byte task, word tenths) {
	// if new timer will go off sooner than the rest, then adjust all entries
	if (tenths < remaining) {
		word diff = remaining - tenths;
		for (byte i = 0; i < maxTasks; ++i)
			if (tasks[i] != ~0)
				tasks[i] += diff;
		remaining = tenths;
	}
	tasks[task] = tenths - remaining;
}

void Scheduler::cancel(byte task) {
	tasks[task] = ~0;
}


