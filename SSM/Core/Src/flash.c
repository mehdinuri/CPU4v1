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
static uint32_t FlashBankGet(uint32_t address)
{
	if (address < FLASH_BANK2_START_ADDR)
	{
		return FLASH_BANK_1;
	}
	else
	{
		return FLASH_BANK_2;
	}
}

static uint32_t FlashPageGet(uint32_t address)
{
  if(address < FLASH_BANK2_START_ADDR)
  {
    return (uint32_t)((address - FLASH_BASE) / FLASH_PAGE_SIZE);
  }
  else
  {
		return (uint32_t)((address - (FLASH_BASE + FLASH_BANK_SIZE)) / FLASH_PAGE_SIZE);
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
uint32_t banks = 0, numberOfPages = 0, pageError = 0;
uint32_t startPage = 0, endPage = 0;

static uint8_t FlashErase(uint32_t address, uint32_t dataSize)
{
	FLASH_EraseInitTypeDef eraseInitStruct = { 0 };
	
	banks = 0, numberOfPages = 0, pageError = 0;
	startPage = 0, endPage = 0,
	
	IWDGRefresh();
	
	banks = FlashBankGet(address);
	startPage = FlashPageGet(address);
	endPage = FlashPageGet((address + dataSize) - 1U);
	numberOfPages = endPage - startPage + 1;
	
	eraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
	eraseInitStruct.Banks = banks;
	eraseInitStruct.Page = startPage;
	eraseInitStruct.NbPages = numberOfPages;
	if(HAL_FLASHEx_Erase(&eraseInitStruct, &pageError) != HAL_OK)
	{
		return FALSE;
	}
	
	return TRUE;
}

uint8_t FlashWrite(uint32_t address, const void * data, uint32_t dataSize)
{
	uint32_t writeAddress = address, bytesWritten = 0;
	
	if ((data == NULL)
	    || (dataSize == 0U)
	    || (writeAddress % 8 != 0)
	    || (writeAddress < FLASH_ADDR_USER_BASE)
	    || (writeAddress > FLASH_ADDR_USER_END)
	    || ((dataSize - 1U) > (FLASH_ADDR_USER_END - writeAddress)))
	{
		return FALSE;
	}
	
	FlashInit();
	
	if (!FlashErase(address, dataSize))
	{
		FlashDeInit();
		return FALSE;
	}
	
	const uint8_t *bytes = (const uint8_t *)data;
	uint64_t dataBuf = 0;

	while(bytesWritten < dataSize)
	{
		IWDGRefresh();

		dataBuf = 0;
		uint8_t numBytes = (dataSize - bytesWritten) >= 8 ? 8 : (dataSize - bytesWritten);
		memcpy(&dataBuf, &bytes[bytesWritten], numBytes);
		
		if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, writeAddress, dataBuf) == HAL_OK)
		{
			bytesWritten += 8;
			writeAddress += 8;
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

uint8_t FlashRead(uint32_t address, void * data, uint32_t dataSize)
{
	uint32_t readAddress = address, bytesRead = 0;
	uint8_t *bytes	= (uint8_t *) data;

	if ((data == NULL)
	    || (dataSize == 0U)
	    || (readAddress < FLASH_ADDR_USER_BASE)
	    || (readAddress > FLASH_ADDR_USER_END)
	    || ((dataSize - 1U) > (FLASH_ADDR_USER_END - readAddress)))
	{
		return FALSE;
	}

	if(readAddress % 4 != 0) return FALSE;

	while(bytesRead < dataSize)
	{
		IWDGRefresh();

		*bytes = *(__I uint8_t *)readAddress;

		readAddress++;
		bytesRead++;
		bytes++;
	}
	
	return TRUE;
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK *****END OF FILE****/
