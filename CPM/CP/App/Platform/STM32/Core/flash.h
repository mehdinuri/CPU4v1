/**
 ******************************************************************************
 * @file           : flash.h
 * @brief          : Header for cp_asynch.c file.
 *                   This file contains the common defines for CP Asynch
 * operations.
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FLASH_H__
#define __FLASH_H__

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/* Private define ------------------------------------------------------------*/
#define FLASH_BASE_ADDR (uint32_t) (FLASH_BASE)
#define FLASH_END_ADDR (uint32_t) (FLASH_END)

#define ADDR_FLASH_SECTOR_0_BANK1                                              \
        ((uint32_t) 0x08000000) /* Base @ of Sector 0, 128 Kbytes */
#define ADDR_FLASH_SECTOR_1_BANK1                                              \
        ((uint32_t) 0x08020000) /* Base @ of Sector 1, 128 Kbytes */
#define ADDR_FLASH_SECTOR_2_BANK1                                              \
        ((uint32_t) 0x08040000) /* Base @ of Sector 2, 128 Kbytes */
#define ADDR_FLASH_SECTOR_3_BANK1                                              \
        ((uint32_t) 0x08060000) /* Base @ of Sector 3, 128 Kbytes */
#define ADDR_FLASH_SECTOR_4_BANK1                                              \
        ((uint32_t) 0x08080000) /* Base @ of Sector 4, 128 Kbytes */
#define ADDR_FLASH_SECTOR_5_BANK1                                              \
        ((uint32_t) 0x080A0000) /* Base @ of Sector 5, 128 Kbytes */
#define ADDR_FLASH_SECTOR_6_BANK1                                              \
        ((uint32_t) 0x080C0000) /* Base @ of Sector 6, 128 Kbytes */
#define ADDR_FLASH_SECTOR_7_BANK1                                              \
        ((uint32_t) 0x080E0000) /* Base @ of Sector 7, 128 Kbytes */

/* Base address of the Flash sectors Bank 2 */
#define ADDR_FLASH_SECTOR_0_BANK2                                              \
        ((uint32_t) 0x08100000) /* Base @ of Sector 0, 128 Kbytes */
#define ADDR_FLASH_SECTOR_1_BANK2                                              \
        ((uint32_t) 0x08120000) /* Base @ of Sector 1, 128 Kbytes */
#define ADDR_FLASH_SECTOR_2_BANK2                                              \
        ((uint32_t) 0x08140000) /* Base @ of Sector 2, 128 Kbytes */
#define ADDR_FLASH_SECTOR_3_BANK2                                              \
        ((uint32_t) 0x08160000) /* Base @ of Sector 3, 128 Kbytes */
#define ADDR_FLASH_SECTOR_4_BANK2                                              \
        ((uint32_t) 0x08180000) /* Base @ of Sector 4, 128 Kbytes */
#define ADDR_FLASH_SECTOR_5_BANK2                                              \
        ((uint32_t) 0x081A0000) /* Base @ of Sector 5, 128 Kbytes */
#define ADDR_FLASH_SECTOR_6_BANK2                                              \
        ((uint32_t) 0x081C0000) /* Base @ of Sector 6, 128 Kbytes */
#define ADDR_FLASH_SECTOR_7_BANK2                                              \
        ((uint32_t) 0x081E0000) /* Base @ of Sector 7, 128 Kbytes */

/* Private macros ------------------------------------------------------------*/

/* Private types -------------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Public define ------------------------------------------------------------*/
#define INTERNAL_FLASH_ADDR_BASE FLASH_BASE_ADDR
#define INTERNAL_FLASH_ADDR_END FLASH_END_ADDR

/* Public macros ------------------------------------------------------------*/

/* Public types -------------------------------------------------------------*/

/* Public function prototypes -----------------------------------------------*/
extern uint8_t FlashErase(uint32_t lAddress, uint32_t lDataSize);
extern uint8_t FlashWrite(uint32_t lAddress, void *pvData, uint32_t lDataSize);
extern uint8_t FlashRead(uint32_t lAddress, void *pvData, uint32_t lDataSize);

#endif /* __FLASH_H__ */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK *****END OF FILE****/
