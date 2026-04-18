/**
 ******************************************************************************
 * @file    Domain/SignalOutput.c
 * @brief   Pure-computation signal-output predicates — no HAL, no FreeRTOS.
 ******************************************************************************
 */

#include "Domain/SignalOutput.h"

uint8_t SignalOutput_IsActive(uint8_t bOnCntr, uint8_t bOffCntr)
{
  return (uint8_t) (bOnCntr > bOffCntr);
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
