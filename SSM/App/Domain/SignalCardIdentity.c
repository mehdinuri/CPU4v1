/**
 ******************************************************************************
 * @file    Domain/SignalCardIdentity.c
 *******************************************************************************
 */

#include "Domain/SignalCardIdentity.h"

uint8_t SignalCardIdentity_IsValid(uint8_t bCardId)
{
  return (bCardId < SIGNAL_CARD_ID_VALID_COUNT) ? 1U : 0U;
}

uint8_t SignalCardIdentity_CommandBank(uint8_t bCardId)
{
  if (SignalCardIdentity_IsValid(bCardId) == 0U)
  {
    return 0xFFU;
  }

  return (bCardId < SIGNAL_CARD_MODULES_PER_BANK) ? 0U : 1U;
}

uint8_t SignalCardIdentity_OutputBitBase(uint8_t bCardId)
{
  if (SignalCardIdentity_IsValid(bCardId) == 0U)
  {
    return 0xFFU;
  }

  return (uint8_t) ((bCardId % SIGNAL_CARD_MODULES_PER_BANK)
                    * SIGNAL_CARD_OUTPUT_CHANNELS_PER_MODULE);
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
