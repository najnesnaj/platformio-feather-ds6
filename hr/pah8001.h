/* pah8001.h - (c) 2017 Blocks Wearables Ltd.
   ------------------------------------------
   Logic for interacting with PAH8001 pulse oximeter.
*/
#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/**
 * Gets the raw heart rate data.
 * \param buffer Array into which to write data.
 * \return true if successful, false otherwise.
 */
uint8_t Pah8001_ReadRawData(uint8_t buffer[13]);

/**
 * Gets the heart rate value from the given raw data.
 * \param rawdata Array containing raw heart rate data.
 * \param hr_out Reference to value to which to write the heart rate.
 * \return true if successful, false otherwise.
 */
uint8_t Pah8001_HRFromRawData(const uint8_t rawdata[13], float* hr_out);

/**
 * Gets whether the current heart rate value is valid. Call
 * immediately after Pah8001_HRFromRawData.
 * \return true if valid, false otherwise.
 */
bool Pah8001_HRValid(void);

/**
 * Resets the PAH8001.
 * \return true if successful, false otherwise.
 */
bool Pah8001_Reset(void);

/**
 * Enables the PAH8001.
 * \return true if successful, false otherwise.
 */
bool Pah8001_PowerOff(void);

/**
 * Disables the PAH8001.
 * \return true if successful, false otherwise.
 */
bool Pah8001_PowerOn(void);
