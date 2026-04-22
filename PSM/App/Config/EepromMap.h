/**
 ******************************************************************************
 * @file    Config/EepromMap.h
 * @brief   HAL-free EEPROM address layout for PSM persistent storage.
 *
 *          This header is the single source of truth for EEPROM addresses.
 *          It must be kept in sync with Core/Inc/i2c.h which mirrors the
 *          same physical offsets for legacy call-sites.
 *
 *          Each persisted record begins with an 8-bit `alreadySet` sentinel
 *          byte that matches CP's SettingsStorage convention — set to 0xF0
 *          when the record is valid; anything else causes PSM to rewrite
 *          the defaults on boot.
 *
 *          Layout (offsets relative to device base 0x0000):
 *          0x0000 - 0x0003 : reserved (vendor device header)
 *          0x0004         : period record sentinel (0xF0 = valid)
 *          0x0005         : raw CP flash-period wire value (5..40)
 *          0x0006         : offset record sentinel (0xF0 = valid)
 *          0x0007 - 0x0008: MeasurementOffset_t (operation + value, 2 bytes)
 *
 *          Runtime unit conversion for the flash period (implemented in
 *          MeasurementService.c):
 *             ms_cycle = raw_wire × CP_PERIOD_SCALE (10)
 *                                 × MEASUREMENT_TASK_PERIOD_MS (20)
 *          Default 5 → 1000 ms total cycle → 500 ms on / 500 ms off.
 ******************************************************************************
 */

#ifndef APP_CONFIG_EEPROMMAP_H
#define APP_CONFIG_EEPROMMAP_H

#include <stdint.h>

/* Address of the flash-period record (sentinel byte + raw wire value). */
#define EEPROM_ADDR_PERIOD  ((uint32_t) 0x0004U)

/* Address of the calibration-offset record (sentinel byte + offset struct). */
#define EEPROM_ADDR_OFFSET  ((uint32_t) 0x0006U)

#endif /* APP_CONFIG_EEPROMMAP_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
