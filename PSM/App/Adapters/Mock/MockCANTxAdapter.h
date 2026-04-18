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
  uint32_t lLastID;                     /* CAN ID of last call        */
  uint8_t  aLastData[MOCK_CAN_BUF_SIZE]; /* payload of last call       */
  uint8_t  bLastLen;                    /* length of last call        */
  uint32_t lSendCount;                  /* total Send calls           */
} MockCANTxAdapterCtx_t;

void          MockCANTxAdapterInit(MockCANTxAdapterCtx_t *ctx);
ICANTxPort_t  MockCANTxAdapterCreatePort(MockCANTxAdapterCtx_t *ctx);

#endif /* ADAPTERS_MOCK_MOCKCANTXADAPTER_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
