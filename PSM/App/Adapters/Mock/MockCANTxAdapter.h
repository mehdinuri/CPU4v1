/**
 ******************************************************************************
 * @file    Adapters/Mock/MockCANTxAdapter.h
 * @brief   Mock adapter for ICANTxPort — records the last frame sent.
 ******************************************************************************
 */

#ifndef ADAPTERS_MOCK_MOCKCANTXADAPTER_H
#define ADAPTERS_MOCK_MOCKCANTXADAPTER_H

#include "Ports/ICANTxPort.h"

#define MOCK_CAN_BUF_SIZE 8U

typedef struct
{
  uint32_t lastID;                     /* CAN ID of last call        */
  uint8_t  lastData[MOCK_CAN_BUF_SIZE]; /* payload of last call       */
  uint8_t  lastLen;                    /* length of last call        */
  uint32_t sendCount;                  /* total Send calls           */
  uint8_t  mockOverflow;               /* simulated overflow state   */
} MockCANTxAdapterCtx_t;

void          MockCANTxAdapterInit(MockCANTxAdapterCtx_t *ctx);
ICANTxPort_t  MockCANTxAdapterCreatePort(MockCANTxAdapterCtx_t *ctx);

#endif /* ADAPTERS_MOCK_MOCKCANTXADAPTER_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
