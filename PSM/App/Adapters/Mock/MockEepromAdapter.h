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
  uint8_t  buf[MOCK_EEPROM_SIZE]; /* backing store                         */
  uint32_t readCount;             /* total Read calls                      */
  uint32_t writeCount;            /* total Write calls                     */
  uint8_t  readResult;            /* return value for Read  (default: 1)   */
  uint8_t  writeResult;           /* return value for Write (default: 1)   */
} MockEepromAdapterCtx_t;

void          MockEepromAdapterInit(MockEepromAdapterCtx_t *ctx);
IEepromPort_t MockEepromAdapterCreatePort(MockEepromAdapterCtx_t *ctx);

#endif /* ADAPTERS_MOCK_MOCKEEPROMADAPTER_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
