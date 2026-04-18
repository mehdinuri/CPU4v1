/**
 ******************************************************************************
 * @file    Config/EepromMap.h
 * @brief   HAL-free EEPROM address layout for PSM persistent storage.
 *
 *          This header is the single source of truth for EEPROM addresses.
 *          It must be kept in sync with Core/Inc/i2c.h which uses the same
 *          physical offsets for the low-level I2C driver.
 *
 *          Layout (all addresses relative to device base address 0x0000):
 *          0x0000 - 0x0003 : reserved (vendor device header)
 *          0x0004 - 0x0007 : flash-sync period (uint32_t, raw 1-second units)
 *          0x0008 - 0x0009 : calibration offset (tSMeasurementOffset, 2 bytes)
 ******************************************************************************
 */

#ifndef APP_CONFIG_EEPROMMAP_H
#define APP_CONFIG_EEPROMMAP_H

#include <stdint.h>

/* Flash-sync period — stored in raw units (1–40 s); runtime multiplies by 10
 * to get tick counts at the 100 ms MeasurementTask period. */
#define EEPROM_ADDR_PERIOD  ((uint32_t) 0x0004U)

/* AC voltage calibration offset (tSMeasurementOffset: operation + value). */
#define EEPROM_ADDR_OFFSET  ((uint32_t) 0x0008U)

#endif /* APP_CONFIG_EEPROMMAP_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
