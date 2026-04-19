/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  include files */
#include "CanMsgParser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "MCS.h"
#include "MLM.h"
#include "cpmpcomm.h"
#include "data.h"
#include "fdcan.h"
#include "main.h"
#include "signalCardDrv.h"
/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  members */
tSCanPSMVoltMeas SCanPSMVoltMeas[SIG_DEV_PSM_MAX]; /* psm measurements states */
/* are copied into this */
/* buffer */
tSCanSSMVoltCurMeas SCanSSMVoltCurMeas[SIG_DEV_SSM_MAX]; /* ssm measurements */

/* states are copied */
/* into this buffer */

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  methods */
void UseSSMVoltCurrentMeasurements(void)
{
  uint8_t bSGNoInSSM = 0, bSSMNo = 0;

  for (bSSMNo = 0; bSSMNo < SIG_DEV_SSM_MAX; bSSMNo++)
  {
    for (bSGNoInSSM = 0;
         bSGNoInSSM < SIGNAL_OUTPUT_CURRENT_GROUPS_PER_SSM;
         bSGNoInSSM++)
    {
      uint8_t bSGNo = (bSSMNo * SIGNAL_OUTPUT_CURRENT_GROUPS_PER_SSM)
                      + bSGNoInSSM;
      uint16_t sCurrent =
        SCanSSMVoltCurMeas[bSSMNo].USOCurrentsL.baCurrentsL[bSGNoInSSM];

      /* calculate the high byte of the current measurement, high bytes are 2 */
      /* bits for all groups and all of them is stores in only one byte so */
      /* extract the related 2 bits for the bSGNoInSSM, then shift left this */
      /* value that shift value changes according to bSGNoInSSM */
      sCurrent +=
        (uint16_t) ((((SCanSSMVoltCurMeas[bSSMNo].USOCurrentsH.bCurrentsH
                       & laValue2Bit[2 * bSGNoInSSM])
                      + (SCanSSMVoltCurMeas[bSSMNo].USOCurrentsH.
                         bCurrentsH & laValue2Bit[2 * bSGNoInSSM + 1]))
                     << (8 - (bSGNoInSSM * 2))));

      SetCurrentMeasurement(bSGNo, sCurrent);
    }
  }
}

void UsePSMVoltMeasurements(void)
{
  uint16_t sValue = 0;
  uint8_t bPSMNo = 0;

  for (bPSMNo = 0;
       bPSMNo < (SIG_DEV_PSM_LAST - SIG_DEV_PSM_FIRST + 1);
       bPSMNo++)
  {
    /* 24V1 */
    sValue = SCanPSMVoltMeas[bPSMNo].b24V1L + (SCanPSMVoltMeas[bPSMNo].b24V1H <<
                                               8);
    Set24V1(bPSMNo, sValue);

    /* 5V1 */
    sValue = SCanPSMVoltMeas[bPSMNo].b5V1L + (SCanPSMVoltMeas[bPSMNo].b5V1H <<
                                              8);
    Set5V1(bPSMNo, sValue);

    /* 24V2 */
    sValue = SCanPSMVoltMeas[bPSMNo].b24V2L + (SCanPSMVoltMeas[bPSMNo].b24V2H <<
                                               8);
    Set24V2(bPSMNo, sValue);

    /* 5V2 */
    sValue = SCanPSMVoltMeas[bPSMNo].b5V2L + (SCanPSMVoltMeas[bPSMNo].b5V2H <<
                                              8);
    Set5V2(bPSMNo, sValue);

    /* Net Voltage */
    sValue = SCanPSMVoltMeas[bPSMNo].bNetVoltageL
             + (SCanPSMVoltMeas[bPSMNo].bNetVoltageH << 8);
    SetNetVoltages(bPSMNo, sValue);

    /* Net Frequency */
    sValue = SCanPSMVoltMeas[bPSMNo].bNetFrequency;
    SetNetFrequency(bPSMNo, sValue);

    SetIsolatedVoltageState(bPSMNo, SCanPSMVoltMeas[bPSMNo].fIsolatedVoltage);
  }
}

void CanConfigLDM(uint8_t bLDMNo)
{
  /*
  ** Change EMCY CAN ID from (0x80 + Node ID) to (0x300 + Node ID)
  ** This configuration is urgent for resolving ID confliction between IOM and
  * default EMCY CANOpen IDs
  ** A better solution would be changing IOM input status IDs in the future, but
  * for now we will keep it for compatibility
  */

  uint8_t baData[] = { 0x23, 0x14, 0x10, 0x00, 0x00, 0x03, 0x00, 0x00 };

  /* 0x23: SDO Command; write 4 byte value
  ** 0x14: Index (low-byte); 0x1014 COB-ID EMCY entry
  ** 0x10: Index (high-byte); 0x1014 COB-ID EMCY entry
  ** 0x00: Sub-index; 0
  ** 0x00000300: New EMCY CAN ID; 0x300 + Node ID
  */
  CANTxRequest(sizeof(baData), CAN_ID_TYPE_STD,
               CAN_MID_LOOP_DETECTOR_STATUS_REQUEST0 + bLDMNo,
               baData);
}

uint8_t CanReqParser(tpSFDCANRxMsg pRxMsg)
{
  switch (pRxMsg->RxHeader.IdType)
  {
      case FDCAN_EXTENDED_ID:
      {
        switch (pRxMsg->RxHeader.Identifier & 0xFF00)
        {
            case CAN_TX_MP_EXT_ID_SIGNAL_DEFS_0:
            case CAN_TX_MP_EXT_ID_SIGNAL_DEFS_1:
            case CAN_TX_MP_EXT_ID_SIGNALS_DEFINED:
            case CAN_TX_MP_EXT_ID_SG_DEFS_0:
            case CAN_TX_MP_EXT_ID_SG_DEFS_1:
            case CAN_TX_MP_EXT_ID_SG_DEFS_2:
            case CAN_TX_MP_EXT_ID_SG_DEFS_3:
            case CAN_TX_MP_EXT_ID_SG_DEFS_4:
            case CAN_TX_MP_EXT_ID_SG_DEFS_5:
            case CAN_TX_MP_EXT_ID_SO_DEFS_0:
            case CAN_TX_MP_EXT_ID_SO_DEFS_1:
            case CAN_TX_MP_EXT_ID_CVS_DEFS_0:
            case CAN_TX_MP_EXT_ID_CVS_DEFS_1:
            case CAN_TX_MP_EXT_ID_CVS_DEFS_2:
            case CAN_TX_MP_EXT_ID_CVS_DEFS_3:
            case CAN_TX_MP_EXT_ID_CVS_DEFS_4:
            case CAN_TX_MP_EXT_ID_CONFLICTS_EM:
            case CAN_TX_MP_EXT_ID_FLASH_CFG:
            {
              if (CPMPTxCheck(pRxMsg->RxHeader.Identifier,
                              (uint8_t *) &(pRxMsg->Data),
                              pRxMsg->RxHeader.DataLength))
              {
                CPMPCommPacketIncSet(TRUE);
              }

              break;
            }

            case CAN_TX_MP_EXT_ID_DEFAULT:
            {
              CPMPResetErrCounter();
              break;
            }

            case CAN_TX_MP_EXT_ID_EVENT:
            {
              EventMPCont((tpSEvent) & pRxMsg->Data);
              break;
            }

            case CAN_TX_MP_EXT_ID_RESET:
            {
              if (LRLFDetectTimeRead())
              {
                uint8_t bTime = LRLFDetectTimeGet();

                CANTxRequest(1, CAN_ID_TYPE_STD, CAN_LRLF_DETECT_TIME, &bTime);
              }

              ResetCPMPComm();
              break;
            }

            case CAN_TX_MP_EXT_ID_SG_CONFLICT:
            {
              break;
            }
        } /* switch */

        break;
      }

      case FDCAN_STANDARD_ID:
      {
        switch (pRxMsg->RxHeader.Identifier)
        {
            case CAN_MID_LD_DATA0:
            case CAN_MID_LD_DATA1:
            case CAN_MID_LD_DATA2:
            case CAN_MID_LD_DATA3:
            case CAN_MID_LD_DATA4:
            case CAN_MID_LD_DATA5:
            case CAN_MID_LD_DATA6:
            case CAN_MID_LD_DATA7:
            {
              /* Example Data: {0x4F, 0x00, 0x60, 0x01, 0x0F, 0x00, 0x00, 0x00}
              ** 0x4F: SDO response;
              ** 0x00: Sub-index;
              ** 0x60: Index (low-byte)
              ** 0x01: Index (high-byte)
              ** 0x0F: Input status/error
              ** Bytes 5-7: padding
              */

              uint8_t bIOMNo;
              uint8_t bLDMNo = (pRxMsg->RxHeader.Identifier - CAN_MID_LD_DATA0);

              baLoopErrorCounters[bLDMNo] = 0;

              /* loop dedectors 1, 2, 3 and 4 are attached to 1st i/o card */
              /* loop dedectors 5, 6, 7 and 8 are attached to 2nd i/o card */
              if (bLDMNo < (LD_NUM_FOR_1_IO_CARD))
              {
                bIOMNo = 0;
              }
              else
              {
                bIOMNo = 1;
              }

              SaCanDetectorIOInputs[bIOMNo].fIsPhysicallyDriven = TRUE;
              SetLDInputs(bLDMNo, bIOMNo, &(pRxMsg->Data[4]));

              if ((GetErrorInfoPtr()->bErrLDM & laValue2Bit[bLDMNo]) >>
                  (bLDMNo) == 1)
              {
                if (BrokenInputSettingsLoopFlagGet())
                {
                  LogRequest(LOG_REQ_APPEND_ASYNCH,
                             NULL,
                             EVENT_MODULE_RESPONDS,
                             (SIG_DEV_LD_FIRST + bLDMNo),
                             0,
                             0,
                             0);
                }

                GetErrorInfoPtr()->bErrLDM &= ~(laValue2Bit[bLDMNo]);
                SRuntimes.sLDMStatus |= (laValue2Bit[bLDMNo]);

                /* Configure LD module to prevent ID conflict between LDM and IOM */
                CanConfigLDM(bLDMNo);
              }

              break;
            }

            case CAN_MID_IO_INPUTS0:
            case CAN_MID_IO_INPUTS1:
            {
              if (pRxMsg->RxHeader.DataLength == CAN_MID_IO_INPUTS_DATA_LEN)
              {
                if (((pRxMsg->Data[0] == 0x10) || (pRxMsg->Data[0] == 0x50) )
                    && (pRxMsg->Data[1] == 0x81) )
                {
                  /* ID conflict still continues, LDM may have missed configuration */
                  /* message, resend it! */
                  uint8_t bLDMNo;

                  for (bLDMNo = 0; bLDMNo < TOTAL_LOOP_DEDECTOR_NUM; bLDMNo++)
                  {
                    if ((GetErrorInfoPtr()->bErrLDM & laValue2Bit[bLDMNo]) >>
                        (bLDMNo) == 0)
                    {
                      CanConfigLDM(bLDMNo);
                    }
                  }
                }

                uint8_t bIOMNo = (pRxMsg->RxHeader.Identifier
                                  - CAN_MID_IO_INPUTS0);
                tSCanDigitalIOInputs SIOInputs;

                memset(&SIOInputs, 0, sizeof(SIOInputs));

                /* Set state as safe automatically as IO doesn't support sending */
                /* safe states for the moment */
                SIOInputs.sInputSafeStates = 0xFFFF;
                SIOInputs.fIsPhysicallyDriven = TRUE;
                memcpy(&SIOInputs.sInputStates, &pRxMsg->Data[0],
                       sizeof(SIOInputs.sInputStates));

                SetIOInputs(bIOMNo,
                            &SIOInputs);

                if (GetErrorInfoPtr()->bErrIOM == 1)
                {
                  if (BrokenInputSettingsDigitalFlagGet())
                  {
                    LogRequest(LOG_REQ_APPEND_ASYNCH,
                               NULL,
                               EVENT_MODULE_RESPONDS,
                               SIG_DEV_IO_FIRST,
                               0,
                               0,
                               0);
                  }

                  GetErrorInfoPtr()->bErrIOM = 0;
                }

                bInputErrorCounter = 0;
              }

              break;
            }

            case CAN_MID_SSM_VOLT_CURRENT_MEASUREMENTS0:
            case CAN_MID_SSM_VOLT_CURRENT_MEASUREMENTS1:
            case CAN_MID_SSM_VOLT_CURRENT_MEASUREMENTS2:
            case CAN_MID_SSM_VOLT_CURRENT_MEASUREMENTS3:
            case CAN_MID_SSM_VOLT_CURRENT_MEASUREMENTS4:
            case CAN_MID_SSM_VOLT_CURRENT_MEASUREMENTS5:
            case CAN_MID_SSM_VOLT_CURRENT_MEASUREMENTS6:
            case CAN_MID_SSM_VOLT_CURRENT_MEASUREMENTS7:
            {
              uint8_t bSSMNo = (pRxMsg->RxHeader.Identifier
                                - CAN_MID_SSM_VOLT_CURRENT_MEASUREMENTS0);

              bSSMNo = (pRxMsg->RxHeader.Identifier
                        - CAN_MID_SSM_VOLT_CURRENT_MEASUREMENTS0);
              memcpy(&SCanSSMVoltCurMeas[bSSMNo],
                     (tpSCanSSMVoltCurMeas) & (pRxMsg->Data),
                     sizeof(tSCanSSMVoltCurMeas));
              UseSSMVoltCurrentMeasurements();
              break;
            }

            case CAN_MID_PSM_VOLT_MEASUREMENTS0:
            case CAN_MID_PSM_VOLT_MEASUREMENTS1:
            {
              uint8_t bPSMNo = (pRxMsg->RxHeader.Identifier
                                - CAN_MID_PSM_VOLT_MEASUREMENTS0);

              memcpy(&SCanPSMVoltMeas[bPSMNo], &(pRxMsg->Data),
                     sizeof(tSCanPSMVoltMeas));
              UsePSMVoltMeasurements();
              break;
            }
        } /* switch */

        break;
      }
  } /* switch */

  return TRUE;
} /* CanReqParser */

uint8_t CANGetRxDataLength(uint32_t lCode)
{
  if (lCode <= FDCAN_DLC_BYTES_8)
  {
    return FDCAN_DLC_BYTES_0 | lCode;
  }

  if (lCode <= FDCAN_DLC_BYTES_12)
  {
    return 12;
  }

  if (lCode <= FDCAN_DLC_BYTES_16)
  {
    return 16;
  }

  if (lCode <= FDCAN_DLC_BYTES_20)
  {
    return 20;
  }

  if (lCode <= FDCAN_DLC_BYTES_24)
  {
    return 24;
  }

  if (lCode <= FDCAN_DLC_BYTES_32)
  {
    return 32;
  }

  if (lCode <= FDCAN_DLC_BYTES_48)
  {
    return 48;
  }

  if (lCode <= FDCAN_DLC_BYTES_64)
  {
    return 64;
  }

  return FDCAN_DLC_BYTES_0;
} /* CANGetRxDataLength */

uint32_t CANGetTxDataLengthCode(uint8_t bLen)
{
  if (bLen <= 8)
  {
    return FDCAN_DLC_BYTES_0 | bLen;
  }

  if (bLen <= 12)
  {
    return FDCAN_DLC_BYTES_12;
  }

  if (bLen <= 16)
  {
    return FDCAN_DLC_BYTES_16;
  }

  if (bLen <= 20)
  {
    return FDCAN_DLC_BYTES_20;
  }

  if (bLen <= 24)
  {
    return FDCAN_DLC_BYTES_24;
  }

  if (bLen <= 32)
  {
    return FDCAN_DLC_BYTES_32;
  }

  if (bLen <= 48)
  {
    return FDCAN_DLC_BYTES_48;
  }

  if (bLen <= 64)
  {
    return FDCAN_DLC_BYTES_64;
  }

  return FDCAN_DLC_BYTES_0;
} /* CANGetTxDataLengthCode */

void CANRxRequest(tpSFDCANRxMsg pSRxMsg)
{
  tpSFDCANRxMsg pSRxReq =
    (tpSFDCANRxMsg) osMemoryPoolAlloc(FDCANRxReqsMemPoolHandle,
                                      0);

  if (pSRxReq != NULL)
  {
    memcpy(pSRxReq, pSRxMsg, sizeof(tSFDCANRxMsg));
    if (osMessageQueuePut(FDCANRxReqsQueHandle, &pSRxReq, 0, 0) != osOK)
    {
      osMemoryPoolFree(FDCANRxReqsMemPoolHandle, pSRxReq);
    }
  }
}

void CANTxRequest(uint8_t bDLC, uint8_t bIdType, uint16_t sMID, uint8_t *baData)
{
  tpSFDCANTxMsg pSTxMsg =
    (tpSFDCANTxMsg) osMemoryPoolAlloc(FDCANTxReqsMemPoolHandle,
                                      0);

  if (pSTxMsg != NULL)
  {
    if (bIdType == CAN_ID_TYPE_STD)
    {
      pSTxMsg->TxHeader.IdType = FDCAN_STANDARD_ID;
    }
    else
    {
      pSTxMsg->TxHeader.IdType = FDCAN_EXTENDED_ID;
    }

    pSTxMsg->hfdcan = &hfdcan1;
    pSTxMsg->TxHeader.Identifier = (uint32_t) sMID;
    pSTxMsg->TxHeader.FDFormat = FDCAN_CLASSIC_CAN;
    pSTxMsg->TxHeader.DataLength = CANGetTxDataLengthCode(bDLC);
    pSTxMsg->TxHeader.TxFrameType = FDCAN_DATA_FRAME;
    pSTxMsg->TxHeader.BitRateSwitch = FDCAN_BRS_OFF;
    pSTxMsg->TxHeader.MessageMarker = 0;
    pSTxMsg->TxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    pSTxMsg->TxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
    memcpy(pSTxMsg->Data, baData, bDLC);

    if (osMessageQueuePut(FDCANTxReqsQueHandle, &pSTxMsg, 0, 0) != osOK)
    {
      osMemoryPoolFree(FDCANTxReqsMemPoolHandle, pSTxMsg);
    }
  }
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  tasks */
void CANMsgParserTaskFunc(void *argument)
{
  UNUSED(argument);

  tpSFDCANRxMsg pSRxReq = NULL;

  CANStart(&hfdcan1);

  while (FOREVER)
  {
    if (osMessageQueueGet(FDCANRxReqsQueHandle, &pSRxReq, NULL,
                          osWaitForever)
        == osOK)
    {
      CanReqParser(pSRxReq);
      osMemoryPoolFree(FDCANRxReqsMemPoolHandle, pSRxReq);
    }
  }
}

void CANMsgSenderTaskFunc(void *argument)
{
  UNUSED(argument);

  tpSFDCANTxMsg pSTxMsg = NULL;

  while (FOREVER)
  {
    if (osMessageQueueGet(FDCANTxReqsQueHandle, &pSTxMsg, NULL,
                          osWaitForever)
        == osOK)
    {
      CANSendMessage(pSTxMsg);
      CANWaitTransmissionComplete(pSTxMsg->hfdcan);
      osMemoryPoolFree(FDCANTxReqsMemPoolHandle, pSTxMsg);
    }
  }
}
