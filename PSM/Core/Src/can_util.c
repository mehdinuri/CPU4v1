/**
  ******************************************************************************
  * File Name          : can_util.c
  * Description        : Common utility functions and types used for CAN Communication
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "gpio.h"
#include "storage.h"
#include "flash.h"
#include "measurement.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Public typedef -----------------------------------------------------------*/

/* Public define ------------------------------------------------------------*/

/* Public variables ---------------------------------------------------------*/

/* Public function prototypes -----------------------------------------------*/

/* Private application code --------------------------------------------------*/

/* Public application code --------------------------------------------------*/

void vCANUtlProcessStdMsgs(uint32_t ulId, uint8_t * pucData, uint8_t ucLen)
{
	switch(ulId)
	{
		case CP_DATE_TIME_STD_ID:
		{
			vMeasurementSetFlashState(FALSE);
			vMeasurementSetCommError(0);
		}
		break;
		
		case CP_FLASH_SIGNALS_1_STD_ID:
		{
			vMeasurementSetPeriod(pucData[6], TRUE);
		}
		break;
		
		case CP_OFFSET_1_STD_ID:
		case CP_OFFSET_2_STD_ID:
		{
			Offset_t Off;
			if(ulId == CP_OFFSET_1_STD_ID + ucGPIOGetCardID())
			{
				memset(&Off, 0, sizeof(Off));
				Off.ulOperation = pucData[0];
				Off.ulValue = pucData[1];
				vMeasurementSetOffset(&Off, TRUE);
			}
		}
		break;
	}
}

void vCANUtlProcessExtMsgs(uint32_t ulId, uint8_t * pucData, uint8_t ucLen)
{
}
/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK *****END OF FILE****/
