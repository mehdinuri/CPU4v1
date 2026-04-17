/**
  ******************************************************************************
  * File Name          : can_msg_parser.c
  * Description        : Code for freertos applications
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "utilities.h"
#include "can_msg_parser.h"
#include "measurement.h"
/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
void vCanMsgParserInit(void);

/* Public typedef -----------------------------------------------------------*/

/* Public define ------------------------------------------------------------*/

/* Public variables ---------------------------------------------------------*/

/* Public function prototypes -----------------------------------------------*/

/* Private application code --------------------------------------------------*/
void CANMsgParserInit(void)
{
}

void CANMsgParse(tpSFDCANRxMsg pSRxMsg)
{
	switch(pSRxMsg->SRxHeader.IdType)
	{
		case FDCAN_STANDARD_ID:
		{
			switch(pSRxMsg->SRxHeader.Identifier)
			{
				case FDCAN_CP_DATE_TIME_STD_ID:
				{
					MeasurementFlashStateSet(FALSE);
					MeasurementCommCntrReset();
				}
				break;
				
				case FDCAN_CP_FLASH_SIGNALS_1_STD_ID:
				{
					MeasurementPeriodSet(pSRxMsg->baData[6]);
				}
				break;
				
				case FDCAN_CP_OFFSET_1_STD_ID:
				case FDCAN_CP_OFFSET_2_STD_ID:
				{
					MeasurementOffsetSet(pSRxMsg->baData[0], pSRxMsg->baData[1]);
				}
				break;
			}
		}
		break;
	}
}

/* USER CODE BEGIN Header_vCANMsgParserTask */
/**
* @brief Function implementing the xCANMsgParserTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_vCANMsgParserTask */

/* Public application code --------------------------------------------------*/
void CANRxRequest(tpSFDCANRxMsg pSRxMsg)
{
	tpSFDCANRxMsg pSReq = (tpSFDCANRxMsg)osMemoryPoolAlloc(CANRxReqsMemPoolHandle, 0);
	if(pSRxMsg != NULL)
	{
		memcpy(pSReq, pSRxMsg, sizeof(tSFDCANRxMsg));
		if(osMessageQueuePut(CANRxReqsQueueHandle, &pSReq, 0, 0) != osOK)
		{
			osMemoryPoolFree(CANRxReqsMemPoolHandle, pSReq);
		}
	}
}

void CANMsgParserTaskFunc(void * argument)
{
  /* USER CODE BEGIN vCANMsgParserTask */
	UNUSED(argument);

	tpSFDCANRxMsg pSRxMsg = NULL;
	
	CANMsgParserInit();
	
	CANStart(&hfdcan1);
	
  /* Infinite loop */
  while(pdTRUE)
  {
		if(osMessageQueueGet(CANRxReqsQueueHandle, &pSRxMsg, NULL, osWaitForever) == osOK)
		{
			CANMsgParse(pSRxMsg);
			osMemoryPoolFree(CANRxReqsMemPoolHandle, pSRxMsg);
		}
  }
  /* USER CODE END vCANMsgParserTask */
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK *****END OF FILE****/
