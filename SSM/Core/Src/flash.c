/**
  ******************************************************************************
  * File Name          : flash.c
  * Description        : Code for CP Asynch applications
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "flash.h"
#include "utilities.h"
#include "iwdg.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/ 

/* Public typedef -----------------------------------------------------------*/

/* Public define ------------------------------------------------------------*/

/* Public variables ---------------------------------------------------------*/

/* Public function prototypes -----------------------------------------------*/

/* Private application code --------------------------------------------------*/
static uint32_t FlashBankGet(uint32_t lAddress)
{
	if (lAddress < FLASH_BANK2_START_ADDR)
	{
		return FLASH_BANK_1;
	}
	else
	{
		return FLASH_BANK_2;
	}
}

static uint32_t FlashPageGet(uint32_t lAddress)
{
  if(lAddress < FLASH_BANK2_START_ADDR)
  {
    return (uint32_t)((lAddress - FLASH_BASE) / FLASH_PAGE_SIZE);
  }
  else
  {
		return (uint32_t)((lAddress - (FLASH_BASE + FLASH_BANK_SIZE)) / FLASH_PAGE_SIZE);
  }
}

static void FlashFlagsClear(void)
{
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPTVERR | FLASH_FLAG_EOP | FLASH_FLAG_OPERR | 
												 FLASH_FLAG_WRPERR | FLASH_FLAG_PGSERR);
}

static void FlashInit(void)
{
	HAL_FLASH_Unlock();
	FlashFlagsClear();
}

static void FlashDeInit(void)
{
	HAL_FLASH_Lock();
}

/* Public application code --------------------------------------------------*/
uint32_t lBanks = 0, lNumberOfPages = 0, lPageError = 0;
uint32_t lStartPage = 0, lEndPage = 0;

static uint8_t FlashErase(uint32_t lAddress, uint32_t lDataSize)
{
	FLASH_EraseInitTypeDef SEraseInitStruct = { 0 };
	
	lBanks = 0, lNumberOfPages = 0, lPageError = 0;
	lStartPage = 0, lEndPage = 0,
	
	IWDGRefresh();
	
	lBanks = FlashBankGet(lAddress);
	lStartPage = FlashPageGet(lAddress);
	lEndPage = FlashPageGet((lAddress + lDataSize) - 1U);
	lNumberOfPages = lEndPage - lStartPage + 1;
	
	SEraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
	SEraseInitStruct.Banks = lBanks;
	SEraseInitStruct.Page = lStartPage;
	SEraseInitStruct.NbPages = lNumberOfPages;
	if(HAL_FLASHEx_Erase(&SEraseInitStruct, &lPageError) != HAL_OK)
	{
		return FALSE;
	}
	
	return TRUE;
}

uint8_t FlashWrite(uint32_t lAddress, const void * pvData, uint32_t lDataSize)
{
	uint32_t lWriteAddress = lAddress, lBytesWritten = 0;
	
	if ((pvData == NULL)
	    || (lDataSize == 0U)
	    || (lWriteAddress % 8 != 0)
	    || (lWriteAddress < FLASH_ADDR_USER_BASE)
	    || (lWriteAddress > FLASH_ADDR_USER_END)
	    || ((lDataSize - 1U) > (FLASH_ADDR_USER_END - lWriteAddress)))
	{
		return FALSE;
	}
	
	FlashInit();
	
	if (!FlashErase(lAddress, lDataSize))
	{
		FlashDeInit();
		return FALSE;
	}
	
	const uint8_t *pbData = (const uint8_t *)pvData;
	uint64_t llDataBuf = 0;
		
	while(lBytesWritten < lDataSize)
	{
		IWDGRefresh();
		
		llDataBuf = 0;
		uint8_t bNumBytes = (lDataSize - lBytesWritten) >= 8 ? 8 : (lDataSize - lBytesWritten);
		memcpy(&llDataBuf, &pbData[lBytesWritten], bNumBytes);
		
		if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, lWriteAddress, llDataBuf) == HAL_OK)
		{
			lBytesWritten += 8;
			lWriteAddress += 8;
		}
		else
		{
			FlashDeInit();
			return FALSE;
		}
	}
	
	FlashDeInit();
	
	return TRUE;
}

uint8_t FlashRead(uint32_t lAddress, void * pvData, uint32_t lDataSize)
{
	uint32_t lReadAddress = lAddress, lBytesRead = 0;
	uint8_t * pbData	= (uint8_t *) pvData;
	
	if ((pvData == NULL)
	    || (lDataSize == 0U)
	    || (lReadAddress < FLASH_ADDR_USER_BASE)
	    || (lReadAddress > FLASH_ADDR_USER_END)
	    || ((lDataSize - 1U) > (FLASH_ADDR_USER_END - lReadAddress)))
	{
		return FALSE;
	}
	
	if(lReadAddress % 4 != 0) return FALSE;
	
	while(lBytesRead < lDataSize)
	{
		IWDGRefresh();
		
		*pbData = *(__I uint8_t *)lReadAddress;
		
		lReadAddress++;
		lBytesRead++;
		pbData++;
	}
	
	return TRUE;
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK *****END OF FILE****/
