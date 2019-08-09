
#if ARDUINO>=100
#include <Arduino.h> // Arduino 1.0
#else
#include <Wprogram.h> // Arduino 0022
#endif

#include <stdint.h>
#include <avr/pgmspace.h>

const int sendorDeviceAddress = 0x1F;//I2C7bitAddressMode
const int regAddressXOUTL = 0x06;
const int regAddressYOUTL = 0x08;
const int regAddressZOUTL = 0x0A;
const int regAddressODCNTL = 0x1B;
const int regAddressCNTL1 = 0x18;
const int regAddressTDTRC = 0x24;
const int regAddressTDTC = 0x25;
const int regAddressTTH = 0x26;
const int regAddressTTL = 0x27;
const int regAddressFTD = 0x28;
const int regAddressSTD = 0x29;
const int regAddressTLT = 0x2A;
const int regAddressTWS = 0x2B;
const int regAddressINC1 = 0x1C;
const int regAddressINC4 = 0x1F;




class Port {
protected:
    uint8_t portNum;

    inline uint8_t digiPin() const
        { return 14; } //SDA
    inline uint8_t digiPin2() const
        { return 13; } //SCL
    static uint8_t digiPin3()
        { return 24; } // ?? 
    inline uint8_t anaPin() const
    { return portNum - 1; }

    
public:
    inline Port (uint8_t num) : portNum (num) {}

    // DIO pin
    inline void mode(uint8_t value) const
        { pinMode(digiPin(), value); }
    inline uint8_t digiRead() const
        { return digitalRead(digiPin()); }
    inline void digiWrite(uint8_t value) const
        { return digitalWrite(digiPin(), value); }
    inline void anaWrite(uint8_t val) const
        { analogWrite(digiPin(), val); }
    inline uint32_t pulse(uint8_t state, uint32_t timeout =1000000L) const
        { return pulseIn(digiPin(), state, timeout); }
    
    // AIO pin
    inline void mode2(uint8_t value) const
        { pinMode(digiPin2(), value); }
    inline uint16_t anaRead() const
        { return analogRead(anaPin()); }        
    inline uint8_t digiRead2() const
        { return digitalRead(digiPin2()); }
    inline void digiWrite2(uint8_t value) const
        { return digitalWrite(digiPin2(), value); }
    inline uint32_t pulse2(uint8_t state, uint32_t timeout =1000000L) const
        { return pulseIn(digiPin2(), state, timeout); }
        
    // IRQ pin (INT1, shared across all ports)
    static void mode3(uint8_t value)
        { pinMode(digiPin3(), value); }
    static uint8_t digiRead3()
        { return digitalRead(digiPin3()); }
    static void digiWrite3(uint8_t value)
        { return digitalWrite(digiPin3(), value); }
    static void anaWrite3(uint8_t val)
        { analogWrite(digiPin3(), val); }
        
    // both pins: data on DIO, clock on AIO
    inline void shift(uint8_t bitOrder, uint8_t value) const
        { shiftOut(digiPin(), digiPin2(), bitOrder, value); }
    uint16_t shiftRead(uint8_t bitOrder, uint8_t count =8) const;
    void shiftWrite(uint8_t bitOrder, uint16_t value, uint8_t count =8) const;
};




class PortI2C : public Port {
    uint8_t uswait;
#if 0
// speed test with fast hard-coded version for Port 1:
    inline void hold() const
        { _delay_us(1); }
    inline void sdaOut(uint8_t value) const
        { bitWrite(DDRD, 4, !value); bitWrite(PORTD, 4, value); }
    inline uint8_t sdaIn() const
        { return bitRead(PORTD, 4); }
    inline void sclHi() const
        { hold(); bitWrite(PORTC, 0, 1); }
    inline void sclLo() const
        { hold(); bitWrite(PORTC, 0, 0); }
public:
    enum { KHZMAX, KHZ400, KHZ100, KHZ_SLOW };
#else
    inline void hold() const
        { delayMicroseconds(uswait); }
    inline void sdaOut(uint8_t value) const
        { mode(!value); digiWrite(value); }
    inline uint8_t sdaIn() const
        { return digiRead(); }
    inline void sclHi() const
        { hold(); digiWrite2(1); }
    inline void sclLo() const
        { hold(); digiWrite2(0); }
public:
    enum { KHZMAX = 1, KHZ400 = 2, KHZ100 = 9 };
//#endif
    
    PortI2C (uint8_t num, uint8_t rate =KHZMAX);
    
    uint8_t start(uint8_t addr) const;
    void stop() const;
    uint8_t write(uint8_t data) const;
    uint8_t read(uint8_t last) const;
};

class DeviceI2C {
    const PortI2C& port;
    uint8_t addr;
    
public:
    DeviceI2C(const PortI2C& p, uint8_t me) : port (p), addr (me << 1) {}
    
    bool isPresent() const;
    
    uint8_t send() const
        { return port.start(addr); }
    uint8_t receive() const
        { return port.start(addr | 1); }
    void stop() const
        { port.stop(); }
    uint8_t write(uint8_t data) const
        { return port.write(data); }
    uint8_t read(uint8_t last) const
        { return port.read(last); }
        
    uint8_t setAddress(uint8_t me)
        { addr = me << 1; }
};

// The millisecond timer can be used for timeouts up to 60000 milliseconds.
// Setting the timeout to zero disables the timer.
//
// for periodic timeouts, poll the timer object with "if (timer.poll(123)) ..."
// for one-shot timeouts, call "timer.set(123)" and poll as "if (timer.poll())"

class MilliTimer {
    word next;
    byte armed;
public:
    MilliTimer () : armed (0) {}
    
    byte poll(word ms =0);
    word remaining() const;
    byte idle() const { return !armed; }
    void set(word ms);
};

// Low-power utility code using the Watchdog Timer (WDT). Requires a WDT interrupt handler, e.g.
// EMPTY_INTERRUPT(WDT_vect);

// simple task scheduler for times up to 6000 seconds
class Scheduler {
    word* tasks;
    word remaining;
    byte maxTasks;
    MilliTimer ms100;
public:
    // initialize for a specified maximum number of tasks
    Scheduler (byte max);
    Scheduler (word* buf, byte max);

    // return next task to run, -1 if there are none ready to run, but there are tasks waiting, or -2 if there are no tasks waiting (i.e. all are idle)
    char poll();
    // same as poll, but wait for event in power-down mode.
    // Uses Sleepy::loseSomeTime() - see comments there re requiring the watchdog timer. 
    char pollWaiting();
    
    // set a task timer, in tenths of seconds
    void timer(byte task, word tenths);
    // cancel a task timer
    void cancel(byte task);
    
    // return true if a task timer is not running
    byte idle(byte task) { return tasks[task] == ~0; }
};


class PulsePlug : public DeviceI2C {
public:
    PulsePlug (PortI2C& port) : 
    DeviceI2C (port, 0x1F) { //jj
    }

    byte getReg (byte reg);
    int16_t getCoor (byte reg);
    void initPulsePlug();
    void initPulsePlugTap();
    void setReg (byte reg, byte val);
    byte readParam (byte addr);
    void writeParam (byte addr, byte val);

   // variables for output
  // unsigned int ukkie,resp, als_vis, als_ir, ps1, ps2, ps3, aux, more; // keep in this order!
};


#endif
