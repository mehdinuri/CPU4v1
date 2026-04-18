/**
 ******************************************************************************
 * @file    Domain/SignalCardIdentity.h
 * @brief   Pure helpers for validating and mapping the 4-bit SSM card ID.
 *******************************************************************************
 */

#ifndef DOMAIN_SIGNAL_CARD_IDENTITY_H
#define DOMAIN_SIGNAL_CARD_IDENTITY_H

#include <stdint.h>

#define SIGNAL_CARD_ID_VALID_COUNT 8U
#define SIGNAL_CARD_OUTPUT_CHANNELS_PER_MODULE 12U
#define SIGNAL_CARD_MODULES_PER_BANK 4U

uint8_t SignalCardIdentity_IsValid(uint8_t bCardId);
uint8_t SignalCardIdentity_CommandBank(uint8_t bCardId);
uint8_t SignalCardIdentity_OutputBitBase(uint8_t bCardId);

#endif /* DOMAIN_SIGNAL_CARD_IDENTITY_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
