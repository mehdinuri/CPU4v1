/**
  ******************************************************************************
  * File Name          : can_util.c
  * Description        : Common utility functions and types used for CAN Communication
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "can_util.h"
#include "gpio.h"
#include "storage.h"
#include "flash.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
OutputGroupt_t xaOutputGroups[SIGNAL_GROUPS_PER_SSX];
uint8_t ucFlashStatus = FALSE;
uint8_t ucFlashSyncStatus = FALSE;

/* Private function prototypes -----------------------------------------------*/
void vSetFlashStatus(uint8_t ucStatus);
void vSetOutput(uint8_t ucGroupIdx, uint8_t ucOutputIdx, uint8_t ucStatus);
void vSetFlashOutput(uint8_t ucGroupIdx, uint8_t ucOutputIdx, uint8_t ucStatus);
void vUpdateOutputs(uint8_t ucCardId, uint8_t * ucaData);
void vUpdateFlashOutputs(uint8_t ucCardId, uint8_t * ucaData);

void vSaveFlashOutputs(void);

/* Public typedef -----------------------------------------------------------*/

/* Public define ------------------------------------------------------------*/

/* Public variables ---------------------------------------------------------*/

/* Public function prototypes -----------------------------------------------*/

/* Private application code --------------------------------------------------*/
void vSetFlashStatus(uint8_t ucStatus)
{
	ucFlashStatus = ucStatus;
}

void vSetFlashSyncStatus(uint8_t ucStatus)
{
	vSetFlashStatus(TRUE);
	ucFlashSyncStatus = ucStatus;
}

void vSetOutput(uint8_t ucGroupIdx, uint8_t ucOutputIdx, uint8_t ucStatus)
{
	if(ucGroupIdx < SIGNAL_GROUPS_PER_SSX)
	{
		if(ucOutputIdx < SIGNAL_OUTPUTS_PER_SIGNAL_GROUP)
		{
			vSetFlashStatus(FALSE);
			xaOutputGroups[ucGroupIdx].xaOutputs[ucOutputIdx].ucIsActive = ucStatus;
		}
	}
}

void vSetFlashOutput(uint8_t ucGroupIdx, uint8_t ucOutputIdx, uint8_t ucStatus)
{
	if(ucGroupIdx < SIGNAL_GROUPS_PER_SSX)
	{
		if(ucOutputIdx < SIGNAL_OUTPUTS_PER_SIGNAL_GROUP)
		{
			xaOutputGroups[ucGroupIdx].xaOutputs[ucOutputIdx].ucIsFlashing = ucStatus;
		}
	}
}

void vUpdateOutputs(uint8_t ucCardId, uint8_t * pucData)
{
	uint8_t ucGroupIdx, ucOutputIdx;
	uint8_t ucOutputBitIdx = (uint8_t)((ucCardId % SSX_MODULES_PER_SSX_GROUP) * (SIGNAL_OUTPUTS_PER_SSX));
	for(ucGroupIdx = 0; ucGroupIdx < SIGNAL_GROUPS_PER_SSX; ucGroupIdx++)
	{
		for(ucOutputIdx = 0; ucOutputIdx < SIGNAL_OUTPUTS_PER_SIGNAL_GROUP; ucOutputIdx++)
		{
			uint8_t ucStatus = GET_BIT_VALUE(pucData[ucOutputBitIdx / NUM_BITS_IN_BYTE], (ucOutputBitIdx % NUM_BITS_IN_BYTE));
			vSetOutput(ucGroupIdx, ucOutputIdx, ucStatus);
			
			ucOutputBitIdx++;
		}
	}
}

void vUpdateFlashOutputs(uint8_t ucCardId, uint8_t * pucData)
{
	uint8_t ucGroupIdx, ucOutputIdx;
	uint8_t ucOutputBitIdx = (uint8_t)((ucCardId % SSX_MODULES_PER_SSX_GROUP) * (SIGNAL_OUTPUTS_PER_SSX));
	for(ucGroupIdx = 0; ucGroupIdx < SIGNAL_GROUPS_PER_SSX; ucGroupIdx++)
	{
		for(ucOutputIdx = 0; ucOutputIdx < SIGNAL_OUTPUTS_PER_SIGNAL_GROUP; ucOutputIdx++)
		{
			uint8_t ucStatus = GET_BIT_VALUE(pucData[ucOutputBitIdx / NUM_BITS_IN_BYTE], (ucOutputBitIdx % NUM_BITS_IN_BYTE));
			vSetFlashOutput(ucGroupIdx, ucOutputIdx, ucStatus);
			
			ucOutputBitIdx++;
		}
	}
	
	vSaveFlashOutputs();
}

void vSaveFlashOutputs(void)
{
	uint8_t ucGroupIdx, ucOutputIdx;
	
	FlashSignalsConfig_t xCurFlashSigConfig;
	FlashSignalsConfig_t xPrevFlashSigConfig;
	
	memset(&xCurFlashSigConfig, 0xFF, sizeof(xCurFlashSigConfig));
	memset(&xPrevFlashSigConfig, 0xFF, sizeof(xPrevFlashSigConfig));
	
	vUtilsRefreshWatchdogs();
	for(ucGroupIdx = 0; ucGroupIdx < SIGNAL_GROUPS_PER_SSX; ucGroupIdx++)
	{
		for(ucOutputIdx = 0; ucOutputIdx < SIGNAL_OUTPUTS_PER_SIGNAL_GROUP; ucOutputIdx++)
		{
			uint8_t ucBitIdx = (ucGroupIdx * SIGNAL_OUTPUTS_PER_SIGNAL_GROUP) + ucOutputIdx;
			if(xaOutputGroups[ucGroupIdx].xaOutputs[ucOutputIdx].ucIsFlashing)
			{
				CLEAR_BIT_VALUE(xCurFlashSigConfig.usFlashBits, ucBitIdx);
			}
			else
			{
				SET_BIT_VALUE(xCurFlashSigConfig.usFlashBits, ucBitIdx);
			}
		}
	}
	
	vUtilsRefreshWatchdogs();
	xCurFlashSigConfig.ulIsWritten = FLASH_SIGNALS_CONFIG_ALREADY_WRITTEN;
	if(StorageRequest(STORAGE_REQ_FLASH_READ, FLASH_ADDR_USER_BASE, (uint32_t *)&xPrevFlashSigConfig, sizeof(xPrevFlashSigConfig)))
	{
		if(memcmp(&xCurFlashSigConfig, &xPrevFlashSigConfig, sizeof(FlashSignalsConfig_t)) == 0)
		{
			return;
		}
	}

	StorageRequest(STORAGE_REQ_FLASH_WRITE, FLASH_ADDR_USER_BASE, (uint32_t *)&xCurFlashSigConfig, sizeof(xCurFlashSigConfig));
}

/* Public application code --------------------------------------------------*/
uint8_t ucCANUtlGetFlashStatus(void)
{
	return ucFlashStatus;
}

uint8_t ucCANUtlGetFlashSyncStatus(void)
{
	return ucFlashSyncStatus;
}

void vCANUtlReadFlashOutputs(void)
{
	uint8_t ucGroupIdx, ucOutputIdx;
	
	FlashSignalsConfig_t xCurFlashSigConfig;
	memset(&xCurFlashSigConfig, 0xFF, sizeof(xCurFlashSigConfig));
	
	if(StorageRequest(STORAGE_REQ_FLASH_READ, FLASH_ADDR_USER_BASE, (uint32_t *)&xCurFlashSigConfig, sizeof(xCurFlashSigConfig)))
	{
		if(xCurFlashSigConfig.ulIsWritten == FLASH_SIGNALS_CONFIG_ALREADY_WRITTEN)
		{
			for(ucGroupIdx = 0; ucGroupIdx < SIGNAL_GROUPS_PER_SSX; ucGroupIdx++)
			{
				for(ucOutputIdx = 0; ucOutputIdx < SIGNAL_OUTPUTS_PER_SIGNAL_GROUP; ucOutputIdx++)
				{
					uint8_t ucBitIdx = (ucGroupIdx * SIGNAL_OUTPUTS_PER_SIGNAL_GROUP) + ucOutputIdx;
					if(GET_BIT_VALUE(xCurFlashSigConfig.usFlashBits, ucBitIdx))
					{
						xaOutputGroups[ucGroupIdx].xaOutputs[ucOutputIdx].ucIsFlashing = FALSE;
					}
					else
					{
						xaOutputGroups[ucGroupIdx].xaOutputs[ucOutputIdx].ucIsFlashing = TRUE;
					}
				}
			}
		}
	}
}

GPIO_PinState xCANUtlGetOutputState(uint8_t ucGroupIdx, uint8_t ucOutputIdx)
{
	return xaOutputGroups[ucGroupIdx].xaOutputs[ucOutputIdx].ucIsActive ? GPIO_PIN_SET : GPIO_PIN_RESET;
}

GPIO_PinState xCANUtlGetOutputFlashState(uint8_t ucGroupIdx, uint8_t ucOutputIdx)
{
	return xaOutputGroups[ucGroupIdx].xaOutputs[ucOutputIdx].ucIsFlashing ? GPIO_PIN_SET : GPIO_PIN_RESET;
}

void vCANUtlProcessStdMsgs(uint32_t ulId, uint8_t * pucData, uint8_t ucLen)
{
	uint8_t ucCardId = GPIOGetCardID();
	switch(ulId)
	{
		case CPX_SIGNAL_OUTPUTS_1_STD_ID:
		{
			if(ucCardId < SSX_MODULES_PER_SSX_GROUP)
			{
				vUpdateOutputs(ucCardId, pucData); 
			}
		}
		break;
		
		case CPX_SIGNAL_OUTPUTS_2_STD_ID:
		{
			if(ucCardId >= SSX_MODULES_PER_SSX_GROUP)
			{
				vUpdateOutputs(ucCardId, pucData); 
			}
		}
		break;
		
		case CPX_FLASH_SIGNALS_1_STD_ID:
		{
			if(ucCardId < SSX_MODULES_PER_SSX_GROUP)
			{
				vUpdateFlashOutputs(ucCardId, pucData);
			}
		}
		break;
		
		case CPX_FLASH_SIGNALS_2_STD_ID:
		{
			if(ucCardId >= SSX_MODULES_PER_SSX_GROUP)
			{
				vUpdateFlashOutputs(ucCardId, pucData); 
			}
		}
		break;
		
		case PSX_FLASH_SYNC_1_STD_ID:
		case PSX_FLASH_SYNC_2_STD_ID:
		{
			vSetFlashSyncStatus(GET_BIT_VALUE(pucData[0], 0));
		}
		break;
	}
}

void vCANUtlProcessExtMsgs(uint32_t ulId, uint8_t * pucData, uint8_t ucLen)
{
}
/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK *****END OF FILE****/
