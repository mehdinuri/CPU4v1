/**
  ******************************************************************************
  * File Name          : utilities.c
  * Description        : Common utility functions and types
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "cmsis_os2.h"
#include <string.h>
#include "main.h"
#include "utilities.h"
#include "iwdg.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Public typedef -----------------------------------------------------------*/

/* Public define ------------------------------------------------------------*/

/* Public variables ---------------------------------------------------------*/

const uint32_t laUtilsBitValues[32] =
{
	0x00000001,
	0x00000002,
	0x00000004,
	0x00000008,
	0x00000010,
	0x00000020,
	0x00000040,
	0x00000080,
	0x00000100,
	0x00000200,
	0x00000400,
	0x00000800,
	0x00001000,
	0x00002000,
	0x00004000,
	0x00008000,
	0x00010000,
	0x00020000,
	0x00040000,
	0x00080000,
	0x00100000,
	0x00200000,
	0x00400000,
	0x00800000,
	0x01000000,
	0x02000000,
	0x04000000,
	0x08000000,
	0x10000000,
	0x20000000,
	0x40000000,
	0x80000000
};

/* Public function prototypes -----------------------------------------------*/

/* Private application code --------------------------------------------------*/

/* Public application code --------------------------------------------------*/
uint8_t ucUtilsCalcByteChkSum(void * pvMem, uint16_t usLength)
{
	uint8_t ucCheckSum = 0;
	uint16_t usBytes = usLength;
	uint8_t * pucByte = pvMem;
	
	while(usBytes)
	{
		ucCheckSum ^= *pucByte;
		pucByte++;
		usBytes--;
	}
	
	return ucCheckSum;
}

uint8_t ucUtilsBufferCompare(uint8_t * pucBuf1, uint8_t *pucBuf2, uint16_t usLen)
{
	while(usLen--)
	{
		if(*pucBuf1 != *pucBuf2)
		{
			return FALSE;
		}
		
		pucBuf1++;
		pucBuf2++;
	}
	
	return TRUE;
}

void vUtilsRefreshWatchdogs(void)
{
	IWDGRefresh();
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK *****END OF FILE****/
