/* pah8001.c - (c) 2017 Blocks Wearables Ltd. */
#include <stdint.h>
#include <stddef.h>
#include "pxialg.h"
#include <kxtj2/kxtj2.h>

#define PAH8001_LED_STEP_DELTA 2
#define PAH8001_LED_EXPOSURE_MAX 496
#define PAH8001_LED_EXPOSURE_MIN 32
#define PAH8001_LED_EXPOSURE_BIG 420
#define PAH8001_LED_EXPOSURE_SML 64
#define PAH8001_LED_STEP_MAX 31
#define PAH8001_LED_STEP_MIN 1


// Define to keep pixart happy
static int errno = 0;
int* __errno(void) { return &errno; }

static bool ppg_current_change = false;

static const struct { uint8_t reg; uint8_t value; } config[] =
{
    { 0x27u, 0xFFu },
    { 0x28u, 0xFAu },
    { 0x29u, 0x0Au },
    { 0x2Au, 0xC8u },
    { 0x2Bu, 0xA0u },
    { 0x2Cu, 0x8Cu },
    { 0x2Du, 0x64u },
    { 0x42u, 0x20u },
    { 0x48u, 0x00u },
    { 0x4Du, 0x1Au },
    { 0x7Au, 0xB5u },
    { 0x7Fu, 0x01u },
    { 0x07u, 0x48u },
    { 0x23u, 0x40u },
    { 0x26u, 0x0Fu },
    { 0x2Eu, 0x48u },
    { 0x38u, 0xEAu },
    { 0x42u, 0xA4u },
    { 0x43u, 0x41u },
    { 0x44u, 0x41u },
    { 0x45u, 0x24u },
    { 0x46u, 0xC0u },
    { 0x52u, 0x32u },
    { 0x53u, 0x28u },
    { 0x56u, 0x60u },
    { 0x57u, 0x28u },
    { 0x6Du, 0x02u },
    { 0x0Fu, 0xC8u },
    { 0x7Fu, 0x00u },
    { 0x5Du, 0x81u }
};

extern void Pah8001_Delay(uint8_t ms);
extern bool Pah8001_ReadRegister(uint8_t reg, uint8_t* buffer, uint8_t length);
extern bool Pah8001_WriteRegister(uint8_t reg, uint8_t value);


static bool Pah8001_Configure()
{
    uint8_t value;

    if (!Pah8001_WriteRegister(0x06u, 0x82u)) return false;
    Pah8001_Delay(10);
    if (!Pah8001_WriteRegister(0x09u, 0x5Au        )) return false;
    if (!Pah8001_WriteRegister(0x05u, 0x99u        )) return false;
    if (!Pah8001_ReadRegister (0x17u, &value, 1    )) return false;
    if (!Pah8001_WriteRegister(0x17u, value | 0x80 )) return false;

    for (size_t i = 0; i < sizeof(config)/sizeof(config[0]); i++)
    {
        if (!Pah8001_WriteRegister(config[i].reg, config[i].value)) return false;
    }

    if (!Pah8001_ReadRegister(0x00, &value, 1)) return false;

    return true;
}

static bool Pah8001_UpdateLed(bool touch)
{
    static bool ppg_sleep = true;
    static uint8_t ppg_states = 0;
    static uint8_t ppg_led_mode = 0;
    static uint8_t ppg_led_step = 10;

    if (!touch)
    {
        if (!Pah8001_WriteRegister(0x7Fu, 0x00u)) return false;
        if (!Pah8001_WriteRegister(0x05u, 0xB8u)) return false;
        if (!Pah8001_WriteRegister(0x7Fu, 0x01u)) return false;
        if (!Pah8001_WriteRegister(0x42u, 0xA0u)) return false;
        if (!Pah8001_WriteRegister(0x38u, 0xE5u)) return false;

        ppg_led_step = 10;
        ppg_sleep = true;
        ppg_current_change = false;
    }
    else
    {
        uint8_t value;
        uint16_t exposureTime;
        if (!Pah8001_WriteRegister(0x7Fu, 0x00u)) return false;
        if (!Pah8001_WriteRegister(0x05u, 0x98u)) return false;
        if (!Pah8001_WriteRegister(0x7Fu, 0x01u)) return false;
        if (!Pah8001_WriteRegister(0x42u, 0xA4u)) return false;
        if (!Pah8001_WriteRegister(0x7Fu, 0x00u)) return false;

        // Read exposure time
        if (!Pah8001_ReadRegister(0x33, &value, 1)) return false;
        exposureTime = (value & 0x3u) << 8;
        if (!Pah8001_ReadRegister(0x32, &value, 1)) return false;
        exposureTime |= value;

        if (!Pah8001_WriteRegister(0x7Fu, 0x01u)) return false;
        if (ppg_sleep)
        {
            if (!Pah8001_WriteRegister(0x38u, (0xE0u | 10))) return false;
            ppg_sleep = false;
        }

        if (ppg_states++ > 3)
        {
            ppg_states = 0;
            if (ppg_led_mode == 0)
            {
                if (exposureTime >= PAH8001_LED_EXPOSURE_MAX
                    || exposureTime <= PAH8001_LED_EXPOSURE_MIN)
                {
                    if (!Pah8001_ReadRegister(0x38u, &ppg_led_step, 1)) return false;
                    ppg_led_step &= 0x1Fu;

                    if (exposureTime >= PAH8001_LED_EXPOSURE_MAX &&
                        ppg_led_step < PAH8001_LED_STEP_MAX)
                    {
                        ppg_led_mode = 1;
                        if (ppg_led_step += PAH8001_LED_STEP_DELTA > PAH8001_LED_STEP_MAX) {
                            ppg_led_step = PAH8001_LED_STEP_MAX;
                        }
                        if (!Pah8001_WriteRegister(0x38u, ppg_led_step | 0xE0u)) return false;
                        ppg_current_change = true;
                    }
                    else if (exposureTime <= PAH8001_LED_EXPOSURE_MIN &&
                        ppg_led_step > PAH8001_LED_STEP_MIN)
                    {
                        ppg_led_mode = 2;
                        if (ppg_led_step <= PAH8001_LED_STEP_MIN + PAH8001_LED_STEP_DELTA) {
                            ppg_led_step = PAH8001_LED_STEP_MIN;
                        }
                        else ppg_led_step -= PAH8001_LED_STEP_DELTA;

                        if (!Pah8001_WriteRegister(0x38u, ppg_led_step | 0xE0u)) return false;
                        ppg_current_change = true;
                    }
                    else
                    {
                        ppg_led_mode = 0;
                        ppg_current_change = false;
                    }
                }
                else ppg_current_change = false;
            }
            else if (ppg_led_mode == 1)
            {
                if (exposureTime > PAH8001_LED_EXPOSURE_BIG)
                {
                    if (ppg_led_step += PAH8001_LED_STEP_DELTA > PAH8001_LED_STEP_MAX)
                    {
                        ppg_led_mode = 0;
                        ppg_led_step = PAH8001_LED_STEP_MAX;
                    }
                    if (!Pah8001_WriteRegister(0x38u, ppg_led_step | 0xE0u)) return false;
                    ppg_current_change = true;
                }
                else
                {
                    ppg_led_mode = 0;
                    ppg_current_change = false;
                }
            }
            else
            {
                if (exposureTime < PAH8001_LED_EXPOSURE_SML)
                {
                    ppg_led_mode = 2;
                    if (ppg_led_step <= PAH8001_LED_STEP_MIN + PAH8001_LED_STEP_DELTA)
                    {
                        ppg_led_mode = 0;
                        ppg_led_step = PAH8001_LED_STEP_MIN;
                    }
                    else ppg_led_step -= PAH8001_LED_STEP_DELTA;

                    if (!Pah8001_WriteRegister(0x38u, ppg_led_step | 0xE0u)) return false;
                    ppg_current_change = true;
                }
                else
                {
                    ppg_led_mode = 0;
                    ppg_current_change = false;
                }
            }
        }
        else {
            ppg_current_change = false;
        }
    }
    return true;
}

uint8_t Pah8001_ReadRawData(uint8_t buffer[13])
{
    static uint8_t ppg_frame_count = 0,touch_cnt = 0;
    uint8_t value;
    if (!Pah8001_WriteRegister(0x7Fu, 0x00u)) return 0x11;
    if (!Pah8001_ReadRegister(0x59u, &value, 1)) return 0x12;
    if (value == 0x80)
        touch_cnt = 0;
    if (touch_cnt++<250)
        if (!Pah8001_UpdateLed(1)) return 0x13;
    else
    {
        if (!Pah8001_UpdateLed(0)) return 0x13;
        touch_cnt = 252;
    }
    if (!Pah8001_WriteRegister(0x7Fu, 0x01u)) return 0x14;

    if (!Pah8001_ReadRegister(0x68u, &value, 1)) return 0x15;
    buffer[0] = value & 0xFu;


    if (buffer[0] != 0) //0 means no data, 1~15 mean have data
    {
        uint8_t tmp[4];
        /* 0x7f is change bank register,
        * 0x64~0x67 is HR_DATA
        * 0x1a~0x1C is HR_DATA_Algo
        */
        if (!Pah8001_ReadRegister(0x64u, &tmp, 4)) return 0x16;
        for (size_t i = 0; i < 4; i++) {
            buffer[1+i] = tmp[i] & 0xFFu;
        }
        if (!Pah8001_ReadRegister(0x1au, &tmp, 3)) return 0x17;
        for (size_t i = 0; i < 3; i++) {
            buffer[5+i] = tmp[i] & 0xFFu;
        }

        buffer[8] = ppg_frame_count++;
        buffer[9] = 47;
        buffer[10] = ppg_current_change ? 1 : 0;
        if (!Pah8001_WriteRegister(0x7Fu, 0x00u)) return 0x18;
        if (!Pah8001_ReadRegister(0x59u, &value, 1)) return 0x19;
        buffer[11] = value & 0x80u;
        buffer[12] = buffer[6] & 0xFFu;
    }
    else
    {
        if (!Pah8001_WriteRegister(0x7Fu, 0x00u)) return 0x20;
        return 0x22;
    }
    return 0;
}

uint8_t Pah8001_HRFromRawData(const uint8_t rawdata[13], float* hr_out)
{
    float axis3[3]; Kxtj2_GetXYZ(axis3);
    if (PxiAlg_Process(rawdata, axis3) != FLAG_DATA_READY) return 0x23;
    PxiAlg_HrGet(hr_out);
    return 1;
}

bool Pah8001_HRValid(void)
{
    uint8_t value;
    if (!Pah8001_ReadRegister(0x59u, &value, 1)) return false;
    return value & 0x80u == 0x80u;
}

bool Pah8001_Reset(void)
{
    return Kxtj2_Init() && Pah8001_Configure();
}

bool Pah8001_PowerOff(void)
{
    if (!Kxtj2_PowerOff()) return false;
    if (!Pah8001_WriteRegister(0x7Fu, 0x00u)) return false;
    if (!Pah8001_WriteRegister(0x06u, 0x0Au)) return false;
    return true;
}

bool Pah8001_PowerOn(void)
{
    if (!Kxtj2_PowerOn()) return false;
    if (!Pah8001_WriteRegister(0x7Fu, 0x00u)) return false;
    if (!Pah8001_WriteRegister(0x06u, 0x02u)) return false;
    if (!Pah8001_WriteRegister(0x05u, 0x99u)) return false;
}
