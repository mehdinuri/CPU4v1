/**
 ******************************************************************************
 * File Name          : flash.c
 * Description        : Code for CP Asynch applications
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "flash.h"

#include <string.h>

#include "iwdg.h"
#include "projdefs.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Public typedef -----------------------------------------------------------*/

/* Public define ------------------------------------------------------------*/

/* Public variables ---------------------------------------------------------*/

/* Public function prototypes -----------------------------------------------*/

/* Private application code --------------------------------------------------*/
int8_t FlashSectorGet(uint32_t lAddress)
{
  if (((lAddress < ADDR_FLASH_SECTOR_1_BANK1)
       && (lAddress >= ADDR_FLASH_SECTOR_0_BANK1)) || ((lAddress
                                                        <
                                                        ADDR_FLASH_SECTOR_1_BANK2)
                                                       && (lAddress
                                                           >=
                                                           ADDR_FLASH_SECTOR_0_BANK2)))
  {
    return FLASH_SECTOR_0;
  }
  else if (((lAddress < ADDR_FLASH_SECTOR_2_BANK1)
            && (lAddress >= ADDR_FLASH_SECTOR_1_BANK1))
           || ((lAddress < ADDR_FLASH_SECTOR_2_BANK2)
               && (lAddress >= ADDR_FLASH_SECTOR_1_BANK2)))
  {
    return FLASH_SECTOR_1;
  }
  else if (((lAddress < ADDR_FLASH_SECTOR_3_BANK1)
            && (lAddress >= ADDR_FLASH_SECTOR_2_BANK1))
           || ((lAddress < ADDR_FLASH_SECTOR_3_BANK2)
               && (lAddress >= ADDR_FLASH_SECTOR_2_BANK2)))
  {
    return FLASH_SECTOR_2;
  }
  else if (((lAddress < ADDR_FLASH_SECTOR_4_BANK1)
            && (lAddress >= ADDR_FLASH_SECTOR_3_BANK1))
           || ((lAddress < ADDR_FLASH_SECTOR_4_BANK2)
               && (lAddress >= ADDR_FLASH_SECTOR_3_BANK2)))
  {
    return FLASH_SECTOR_3;
  }
  else if (((lAddress < ADDR_FLASH_SECTOR_5_BANK1)
            && (lAddress >= ADDR_FLASH_SECTOR_4_BANK1))
           || ((lAddress < ADDR_FLASH_SECTOR_5_BANK2)
               && (lAddress >= ADDR_FLASH_SECTOR_4_BANK2)))
  {
    return FLASH_SECTOR_4;
  }
  else if (((lAddress < ADDR_FLASH_SECTOR_6_BANK1)
            && (lAddress >= ADDR_FLASH_SECTOR_5_BANK1))
           || ((lAddress < ADDR_FLASH_SECTOR_6_BANK2)
               && (lAddress >= ADDR_FLASH_SECTOR_5_BANK2)))
  {
    return FLASH_SECTOR_5;
  }
  else if (((lAddress < ADDR_FLASH_SECTOR_7_BANK1)
            && (lAddress >= ADDR_FLASH_SECTOR_6_BANK1))
           || ((lAddress < ADDR_FLASH_SECTOR_7_BANK2)
               && (lAddress >= ADDR_FLASH_SECTOR_6_BANK2)))
  {
    return FLASH_SECTOR_6;
  }
  else if (((lAddress < ADDR_FLASH_SECTOR_0_BANK2)
            && (lAddress >= ADDR_FLASH_SECTOR_7_BANK1))
           || ((lAddress < FLASH_END_ADDR)
               && (lAddress >= ADDR_FLASH_SECTOR_7_BANK2)))
  {
    return FLASH_SECTOR_7;
  }

  return -1;
} /* FlashSectorGet */

int8_t FlashBanksGet(uint32_t ulStartAddr, uint32_t ulEndAddr)
{
  if ((ulStartAddr >= ADDR_FLASH_SECTOR_0_BANK1)
      && (ulEndAddr < ADDR_FLASH_SECTOR_0_BANK2))
  {
    return FLASH_BANK_1;
  }
  else if ((ulStartAddr >= ADDR_FLASH_SECTOR_0_BANK2)
           && (ulEndAddr < FLASH_END_ADDR))
  {
    return FLASH_BANK_2;
  }
  else if ((ulStartAddr >= ADDR_FLASH_SECTOR_0_BANK1)
           && (ulEndAddr < FLASH_END_ADDR))
  {
    return FLASH_BANK_BOTH;
  }

  return -1;
}

void FlashClearFlags(void)
{
  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_BANK1 | FLASH_FLAG_ALL_BANK2);
}

void FlashUnlock(void)
{
  HAL_FLASH_Unlock();
}

void FlashLock(void)
{
  HAL_FLASH_Lock();
}

static uint8_t FlashSectorErase(FLASH_EraseInitTypeDef *pSEraseInit)
{
  uint32_t ulSectorError = 0;

  if (HAL_FLASHEx_Erase(pSEraseInit, &ulSectorError) != HAL_OK)
  {
    uint32_t lErr = HAL_FLASH_GetError();

    if (lErr != HAL_FLASH_ERROR_NONE)
    {
      FlashLock();
      SCB_EnableICache();

      Error_Handler();
    }
  }

  return 1;
}

/* Public application code --------------------------------------------------*/
uint8_t FlashErase(uint32_t lAddress, uint32_t lDataSize)
{
  if (lDataSize == 0)
  {
    return 0;
  }

  FLASH_EraseInitTypeDef SEraseInit;
  int8_t ulBanks = 0, ulStartSector = 0, ulEndSector = 0;
  uint32_t ulEndAddress = 0;

  IWDGSetMaxTimeout();

  SCB_DisableICache();

  FlashUnlock();

  FlashClearFlags();

  ulEndAddress = lAddress + lDataSize - 1;
  ulBanks = FlashBanksGet(lAddress, ulEndAddress);
  if (ulBanks == -1)
  {
    FlashLock();
    SCB_EnableICache();

    return 0;
  }

  ulStartSector = FlashSectorGet(lAddress);
  if (ulStartSector == -1)
  {
    FlashLock();
    SCB_EnableICache();

    return 0;
  }

  ulEndSector = FlashSectorGet(ulEndAddress);
  if (ulEndSector == -1)
  {
    FlashLock();
    SCB_EnableICache();

    return 0;
  }

  SEraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;
  SEraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_3;
  if (ulBanks != FLASH_BANK_BOTH)
  {
    SEraseInit.Banks = ulBanks;
    SEraseInit.Sector = ulStartSector;
    SEraseInit.NbSectors = ulEndSector - ulStartSector + 1;
    FlashSectorErase(&SEraseInit);
  }
  else
  {
    SEraseInit.Banks = FLASH_BANK_1;
    SEraseInit.Sector = ulStartSector;
    SEraseInit.NbSectors = FLASH_SECTOR_7 - ulStartSector + 1;
    FlashSectorErase(&SEraseInit);

    SEraseInit.Banks = FLASH_BANK_2;
    SEraseInit.Sector = FLASH_SECTOR_0;
    SEraseInit.NbSectors = ulEndSector + 1;
    FlashSectorErase(&SEraseInit);
  }

  FlashLock();

  SCB_EnableICache();

  IWDGSetNormalTimeout();

  return 1;
} /* FlashErase */

uint8_t FlashWrite(uint32_t lAddress, void *pvSrc, uint32_t lDataSize)
{
  if (lAddress % 32 != 0)
  {
    return 0;
  }

  if (pvSrc == NULL)
  {
    return 0;
  }

  if (lDataSize == 0)
  {
    return 0;
  }

  SCB_DisableICache();

  FlashUnlock();

  FlashClearFlags();

  uint32_t lBytesWritten = 0;
  const uint8_t *cpbSrc = (const uint8_t *) pvSrc;

  while (lBytesWritten < lDataSize)
  {
    IWDGRefresh();

    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD,
                          (lAddress + lBytesWritten),
                          (uint32_t) (cpbSrc + lBytesWritten)) != HAL_OK)
    {
      FlashLock();
      SCB_EnableICache();

      return 0;
    }

    lBytesWritten += 32;
  }

  FlashLock();

  SCB_EnableICache();

  return 1;
} /* FlashWrite */

uint8_t FlashRead(uint32_t lAddress, void *pvDst, uint32_t lDataSize)
{
  if (pvDst == NULL)
  {
    return 0;
  }

  if (lDataSize == 0)
  {
    return 0;
  }

  if ((lAddress % 4 != 0) || ((uint32_t) pvDst % 4 != 0))
  {
    return 0;
  }

  volatile const uint32_t *plSrc = (volatile const uint32_t *) lAddress;
  uint32_t *plDst = (uint32_t *) pvDst;

  uint32_t lIdx = 0;
  uint32_t lNumWords = lDataSize / 4;

  for (lIdx = 0; lIdx < lNumWords; lIdx++)
  {
    plDst[lIdx] = plSrc[lIdx];
  }

  uint8_t bRemaining = lDataSize % 4;

  if (bRemaining)
  {
    uint32_t lAddIdx = lNumWords * 4;
    const uint8_t *pbSrc = (const uint8_t *) (lAddress + lAddIdx);
    uint8_t *pbDst = (uint8_t *) (plDst + lAddIdx);

    uint8_t bIdx = 0;

    for (bIdx = 0; bIdx < bRemaining; bIdx++)
    {
      pbDst[bIdx] = pbSrc[bIdx];
    }
  }

  return 1;
} /* FlashRead */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK *****END OF FILE****/
