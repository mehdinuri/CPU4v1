/**
 ******************************************************************************
 * @file    Domain/SignalOutput.c
 * @brief   Pure-computation signal-output predicates — no HAL, no FreeRTOS.
 ******************************************************************************
 */

#include "Domain/SignalOutput.h"

uint8_t SignalOutput_IsActive(uint8_t onCntr, uint8_t offCntr)
{
  return (uint8_t) (onCntr > offCntr);
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
