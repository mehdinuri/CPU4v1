/**
 ******************************************************************************
 * @file    MLM.h
 * @author  Okan KILIC - Teknotel Electronics
 * @version V1.0.0
 * @date    08/11/2011
 * @brief  Maestro Log Management Header File
 ******************************************************************************
 */

#ifndef __MLM_H__
#define __MLM_H__

#include "data.h"

#define LOG_RECORDS_MAX 1024
#define LOG_SIZE_UPPER_BOUND LOG_RECORDS_MAX - 1
#define LOG_SIZE_LOWER_BOUND 0

/*
 *       following define values are chosen LOG_RECORDS_MAX which cannot be
 *  assigned to log index value any time
 */
#define LOG_GET_WRITE_INDEX_VALUE LOG_RECORDS_MAX
#define LOG_NO_NEW_LOG LOG_RECORDS_MAX

/*
 *       task message queue
 */
#define LOG_REQ_NONE 0
#define LOG_REQ_FIRST 1
#define LOG_REQ_APPEND_ASYNCH 1
#define LOG_REQ_APPEND 2
#define LOG_REQ_READ_NEXT 3
#define LOG_REQ_READ_FROM 4
#define LOG_REQ_LAST 4

typedef struct _tSLogReq
{
  tSLogRecord SLogRecord;
  osThreadId_t SThreadId;
  tpSLogRecord pSLogReadBuf;
  uint16_t sTaskReadIndex;
  uint8_t bReqId;
} tSLogReq, *tpSLogReq;

/*
 *       Public Methods
 */
extern void MLMInit(void);
extern uint16_t LogEventNew(uint16_t);
extern uint8_t LogRequest(uint8_t bReqId, tpSLogRecord pSLogRecordBuffer,
                          uint8_t bEvent, uint8_t bParam, uint16_t sParam,
                          uint32_t lParam, uint16_t sTaskReadIndex);
extern void DeleteLogs(void);
extern uint8_t LogExists(void);
extern uint8_t LogIndexIsValid(uint16_t);

#endif /* ifndef __MLM_H__ */
