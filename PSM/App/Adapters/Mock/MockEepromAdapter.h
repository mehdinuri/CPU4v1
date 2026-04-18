/**
 ******************************************************************************
 * @file    Adapters/Mock/MockEepromAdapter.h
 * @brief   Mock adapter for IEepromPort — in-memory 256-byte buffer.
 ******************************************************************************
 */

#ifndef ADAPTERS_MOCK_MOCKEEPROMADAPTER_H
#define ADAPTERS_MOCK_MOCKEEPROMADAPTER_H

#include "Ports/IEepromPort.h"

#define MOCK_EEPROM_SIZE 256U

typedef struct
{
  uint8_t  aBuf[MOCK_EEPROM_SIZE]; /* backing store                         */
  uint32_t lReadCount;             /* total Read calls                      */
  uint32_t lWriteCount;            /* total Write calls                     */
  uint8_t  bReadResult;            /* return value for Read  (default: 1)   */
  uint8_t  bWriteResult;           /* return value for Write (default: 1)   */
} MockEepromAdapterCtx_t;

void          MockEepromAdapterInit(MockEepromAdapterCtx_t *ctx);
IEepromPort_t MockEepromAdapterCreatePort(MockEepromAdapterCtx_t *ctx);

#endif /* ADAPTERS_MOCK_MOCKEEPROMADAPTER_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
