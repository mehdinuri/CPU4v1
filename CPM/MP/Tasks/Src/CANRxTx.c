#include "CANRxTx.h"
#include "signalOutputCatch.h"
#include <string.h>
#include "signalCheck.h"
#include "gpio.h"
#include "maintenance.h"

/* ---------------------------------------------------------- Definitions ---------------------------------------------------------- */
#define LAMP_DIMMING_STATE_CHANGE_DURATION 2000

typedef struct _tSCPMPCommData
{
  uint16_t sIndex;

  tSPeripheralStates SPeripheralStates;
  tSSignalDef SaSignalDefs;
  tSSignalsDefined SSignalsDefined;
  tSSGDef SaSGDefs;
  tSSODef SaSODefs;
  tSCVSDef SCVSDef;
  tSConflictsEM SConflictsEM;
  uint16_t sFlashPerInEm;
} tSCPMPCommData, *tpSCPMPCommData;

/* ---------------------------------------------------------- Private Data ---------------------------------------------------------- */
static tSCPMPCommData SCPMPCommData;
static uint16_t sLampDimmingStateChangeDur = 0;
static uint8_t fLampDimmingState = FALSE;

/* ------------------------------------------------------------ Methods ------------------------------------------------------------- */
void CANMsgParserInit(void)
{
  memset(&SCPMPCommData, 0, sizeof(tSCPMPCommData));
}

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

void CanRxRequest(tpSCanRxMsg pSRxMsg)
{
  tpSCanRxMsg pSReq = (tpSCanRxMsg) osMemoryPoolAlloc(CANRxReqsMemPoolHandle,
                                                      0);

  if (pSReq != NULL)
  {
    memcpy(pSReq, pSRxMsg, sizeof(tSCanRxMsg));
    if (osMessageQueuePut(CANRxReqsQueueHandle, &pSReq, 0, 0) != osOK)
    {
      osMemoryPoolFree(CANRxReqsMemPoolHandle, pSReq);
    }
  }
}

void CanTxRequest(FDCAN_HandleTypeDef *hfdcan,
                  uint32_t lIDType,
                  uint32_t lID,
                  uint32_t lFrameType,
                  uint32_t lBitRateSwitch,
                  uint32_t lFDFormat,
                  uint8_t *baData,
                  uint8_t bDataLen)
{
  tpSCanTxMsg pSReq = (tpSCanTxMsg) osMemoryPoolAlloc(CANTxReqsMemPoolHandle,
                                                      0);

  if (pSReq != NULL)
  {
    memset(pSReq, 0, sizeof(tSCanTxMsg));

    pSReq->hfdcan = hfdcan;
    pSReq->TxHeader.IdType = lIDType;
    pSReq->TxHeader.Identifier = lID;
    pSReq->TxHeader.TxFrameType = lFrameType;
    pSReq->TxHeader.BitRateSwitch = lBitRateSwitch;
    pSReq->TxHeader.FDFormat = lFDFormat;
    pSReq->TxHeader.DataLength = CANGetTxDataLengthCode(bDataLen);
    pSReq->TxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    pSReq->TxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
    pSReq->TxHeader.MessageMarker = 0x00;
    memcpy(pSReq->baData, baData, bDataLen);

    if (osMessageQueuePut(CANTxReqsQueueHandle, &pSReq, 0, 0) != osOK)
    {
      osMemoryPoolFree(CANTxReqsMemPoolHandle, pSReq);
    }
  }
}

void CanReceive(uint8_t *pbDest,
                uint8_t *pbSource,
                uint16_t sDataLen,
                uint16_t *psIndex,
                uint16_t sDataSize)
{
  uint16_t sByteNo = 0;

  for (sByteNo = 0; sByteNo < sDataLen; sByteNo++)
  {
    pbDest[*psIndex] = pbSource[sByteNo];
    *psIndex = *psIndex + 1;

    if (*psIndex >= sDataSize)
    {
      break;
    }
  }
}

void CanProcessData(uint32_t lId, uint8_t *pbData, uint8_t bDataLen)
{
  uint8_t bPacketNo = (lId % 0x0100);
  uint16_t lAnsMsgId = (lId + 0x2000) + bPacketNo;

  switch (lId & 0xFF00)
  {
      case CAN_TX_CP_EXT_ID_DEFAULT:
      {
        CPMPCommResetCntr();

        SCPMPCommData.sIndex = 0;
        CanReceive((uint8_t *) &(SCPMPCommData.SPeripheralStates),
                   pbData,
                   bDataLen,
                   &(SCPMPCommData.sIndex),
                   sizeof(SCPMPCommData.SPeripheralStates));
        PeripheralStatesSet(&SCPMPCommData.SPeripheralStates);

        if (ProgramRestartStateGet())
        {
          RuntimesInit();
        }
        else
        {
          if (!CPCommStateGet())
          {
            RuntimesInit();
            CPCommStateSet(TRUE);
          }

          if (ClearSOPowersStateGet())
          {
            uint8_t bSONo = 0;

            for (bSONo = 0; bSONo < SIGNAL_OUTPUTS_MAX; bSONo++)
            {
              ClearSOPower(bSONo);
            }
          }

          if (LampDimmingStateGet())
          {
            if (!fLampDimmingState)
            {
              fLampDimmingState = TRUE;
              LampDimmingStateChangedSet(TRUE);
            }
          }
          else
          {
            if (fLampDimmingState)
            {
              fLampDimmingState = FALSE;
              LampDimmingStateChangedSet(TRUE);
            }
          }

          if (LampDimmingStateChangedGet())
          {
            if (sLampDimmingStateChangeDur++
                > LAMP_DIMMING_STATE_CHANGE_DURATION)
            {
              sLampDimmingStateChangeDur = 0;
              LampDimmingStateChangedSet(FALSE);
            }
          }

          if (RelayStateGet())
          {
            if (!GPIORelayPinGet())
            {
              GPIORelayPinSet(GPIO_PIN_SET);
            }
          }
          else
          {
            if (GPIORelayPinGet())
            {
              GPIORelayPinSet(GPIO_PIN_RESET);
            }
          }
        }

        CanTxRequest(&hfdcan1,
                     FDCAN_EXTENDED_ID,
                     lAnsMsgId,
                     FDCAN_DATA_FRAME,
                     FDCAN_BRS_OFF,
                     FDCAN_CLASSIC_CAN,
                     (uint8_t *) (PeripheralStatesGet()),
                     sizeof(tSPeripheralStates));
        break;
      }

      case CAN_TX_CP_EXT_ID_SIGNAL_DEFS_0:
      case CAN_TX_CP_EXT_ID_SIGNAL_DEFS_1:
      case CAN_TX_CP_EXT_ID_SIGNALS_DEFINED:
      case CAN_TX_CP_EXT_ID_SG_DEFS_0:
      case CAN_TX_CP_EXT_ID_SG_DEFS_1:
      case CAN_TX_CP_EXT_ID_SG_DEFS_2:
      case CAN_TX_CP_EXT_ID_SG_DEFS_3:
      case CAN_TX_CP_EXT_ID_SG_DEFS_4:
      case CAN_TX_CP_EXT_ID_SG_DEFS_5:
      case CAN_TX_CP_EXT_ID_SO_DEFS_0:
      case CAN_TX_CP_EXT_ID_SO_DEFS_1:
      case CAN_TX_CP_EXT_ID_CVS_DEFS_0:
      case CAN_TX_CP_EXT_ID_CVS_DEFS_1:
      case CAN_TX_CP_EXT_ID_CVS_DEFS_2:
      case CAN_TX_CP_EXT_ID_CVS_DEFS_3:
      case CAN_TX_CP_EXT_ID_CVS_DEFS_4:
      case CAN_TX_CP_EXT_ID_CONFLICTS_EM:
      case CAN_TX_CP_EXT_ID_FLASH_CFG:
      {
        if (CPCommStateGet())
        {
          CPCommStateSet(FALSE);
          DataInit();
        }

        break;
      }
  } /* switch */

  switch (lId & 0xFF00)
  {
      case CAN_TX_CP_EXT_ID_SIGNAL_DEFS_0:
      {
        SCPMPCommData.sIndex = 0;
        CanReceive((uint8_t *) &(SCPMPCommData.SaSignalDefs),
                   pbData,
                   bDataLen,
                   &(SCPMPCommData.sIndex),
                   sizeof(SCPMPCommData.SaSignalDefs));
        break;
      }

      case CAN_TX_CP_EXT_ID_SIGNAL_DEFS_1:
      {
        SCPMPCommData.sIndex = 8;
        CanReceive((uint8_t *) &(SCPMPCommData.SaSignalDefs),
                   pbData,
                   bDataLen,
                   &(SCPMPCommData.sIndex),
                   sizeof(SCPMPCommData.SaSignalDefs));

        SignalSet(bPacketNo, &(SCPMPCommData.SaSignalDefs));
        break;
      }

      case CAN_TX_CP_EXT_ID_SIGNALS_DEFINED:
      {
        SCPMPCommData.sIndex = 0;
        CanReceive((uint8_t *) &(SCPMPCommData.SSignalsDefined),
                   pbData,
                   bDataLen,
                   &(SCPMPCommData.sIndex),
                   sizeof(SCPMPCommData.SSignalsDefined));

        SignalsDefinedSet(&(SCPMPCommData.SSignalsDefined));
        break;
      }

      case CAN_TX_CP_EXT_ID_SG_DEFS_0:
      {
        SCPMPCommData.sIndex = 0;
        CanReceive((uint8_t *) &(SCPMPCommData.SaSGDefs), pbData, bDataLen,
                   &(SCPMPCommData.sIndex), sizeof(SCPMPCommData.SaSGDefs));
        break;
      }

      case CAN_TX_CP_EXT_ID_SG_DEFS_1:
      {
        SCPMPCommData.sIndex = 8;
        CanReceive((uint8_t *) &(SCPMPCommData.SaSGDefs), pbData, bDataLen,
                   &(SCPMPCommData.sIndex), sizeof(SCPMPCommData.SaSGDefs));
        break;
      }

      case CAN_TX_CP_EXT_ID_SG_DEFS_2:
      {
        SCPMPCommData.sIndex = 16;
        CanReceive((uint8_t *) &(SCPMPCommData.SaSGDefs), pbData, bDataLen,
                   &(SCPMPCommData.sIndex), sizeof(SCPMPCommData.SaSGDefs));
        break;
      }

      case CAN_TX_CP_EXT_ID_SG_DEFS_3:
      {
        SCPMPCommData.sIndex = 24;
        CanReceive((uint8_t *) &(SCPMPCommData.SaSGDefs), pbData, bDataLen,
                   &(SCPMPCommData.sIndex), sizeof(SCPMPCommData.SaSGDefs));
        break;
      }

      case CAN_TX_CP_EXT_ID_SG_DEFS_4:
      {
        SCPMPCommData.sIndex = 32;
        CanReceive((uint8_t *) &(SCPMPCommData.SaSGDefs), pbData, bDataLen,
                   &(SCPMPCommData.sIndex), sizeof(SCPMPCommData.SaSGDefs));
        break;
      }

      case CAN_TX_CP_EXT_ID_SG_DEFS_5:
      {
        SCPMPCommData.sIndex = 40;
        CanReceive((uint8_t *) &(SCPMPCommData.SaSGDefs), pbData, bDataLen,
                   &(SCPMPCommData.sIndex), sizeof(SCPMPCommData.SaSGDefs));

        SetSGDefinition(bPacketNo, &(SCPMPCommData.SaSGDefs));
        break;
      }

      case CAN_TX_CP_EXT_ID_SO_DEFS_0:
      {
        SCPMPCommData.sIndex = 0;
        CanReceive((uint8_t *) &(SCPMPCommData.SaSODefs), pbData, bDataLen,
                   &(SCPMPCommData.sIndex), sizeof(SCPMPCommData.SaSODefs));
        break;
      }

      case CAN_TX_CP_EXT_ID_SO_DEFS_1:
      {
        SCPMPCommData.sIndex = 8;
        CanReceive((uint8_t *) &(SCPMPCommData.SaSODefs), pbData, bDataLen,
                   &(SCPMPCommData.sIndex), sizeof(SCPMPCommData.SaSODefs));

        SetSODef(bPacketNo, &(SCPMPCommData.SaSODefs));
        break;
      }

      case CAN_TX_CP_EXT_ID_CVS_DEFS_0:
      {
        SCPMPCommData.sIndex = 0;
        CanReceive((uint8_t *) &(SCPMPCommData.SCVSDef), pbData, bDataLen,
                   &(SCPMPCommData.sIndex), sizeof(SCPMPCommData.SCVSDef));
        break;
      }

      case CAN_TX_CP_EXT_ID_CVS_DEFS_1:
      {
        SCPMPCommData.sIndex = 8;
        CanReceive((uint8_t *) &(SCPMPCommData.SCVSDef), pbData, bDataLen,
                   &(SCPMPCommData.sIndex), sizeof(SCPMPCommData.SCVSDef));
        break;
      }

      case CAN_TX_CP_EXT_ID_CVS_DEFS_2:
      {
        SCPMPCommData.sIndex = 16;
        CanReceive((uint8_t *) &(SCPMPCommData.SCVSDef), pbData, bDataLen,
                   &(SCPMPCommData.sIndex), sizeof(SCPMPCommData.SCVSDef));
        break;
      }

      case CAN_TX_CP_EXT_ID_CVS_DEFS_3:
      {
        SCPMPCommData.sIndex = 24;
        CanReceive((uint8_t *) &(SCPMPCommData.SCVSDef), pbData, bDataLen,
                   &(SCPMPCommData.sIndex), sizeof(SCPMPCommData.SCVSDef));
        break;
      }

      case CAN_TX_CP_EXT_ID_CVS_DEFS_4:
      {
        SCPMPCommData.sIndex = 32;
        CanReceive((uint8_t *) &(SCPMPCommData.SCVSDef), pbData, bDataLen,
                   &(SCPMPCommData.sIndex), sizeof(SCPMPCommData.SCVSDef));

        SetCVSDef(&(SCPMPCommData.SCVSDef));
        break;
      }

      case CAN_TX_CP_EXT_ID_CONFLICTS_EM:
      {
        SCPMPCommData.sIndex = 0;
        CanReceive((uint8_t *) &(SCPMPCommData.SConflictsEM),
                   pbData,
                   bDataLen,
                   &(SCPMPCommData.sIndex),
                   sizeof(SCPMPCommData.SConflictsEM));

        SetConflictsEMDef(&(SCPMPCommData.SConflictsEM));
        break;
      }

      case CAN_TX_CP_EXT_ID_FLASH_CFG:
      {
        SCPMPCommData.sIndex = 0;
        CanReceive((uint8_t *) &(SCPMPCommData.sFlashPerInEm),
                   pbData,
                   bDataLen,
                   &(SCPMPCommData.sIndex),
                   sizeof(SCPMPCommData.sFlashPerInEm));
        SetFlashPeriodInEmergency(SCPMPCommData.sFlashPerInEm);
        break;
      }

      default:
      {
        break;
      }
  } /* switch */

  CanTxRequest(&hfdcan1,
               FDCAN_EXTENDED_ID,
               lAnsMsgId,
               FDCAN_DATA_FRAME,
               FDCAN_BRS_OFF,
               FDCAN_CLASSIC_CAN,
               pbData,
               bDataLen);
} /* CanProcessData */

uint8_t CanReqParser(tpSCanRxMsg pRxMsg)
{
  switch (pRxMsg->RxHeader.IdType)
  {
      case FDCAN_EXTENDED_ID:
      {
        switch (pRxMsg->RxHeader.Identifier & 0xFF00)
        {
            case CAN_TX_CP_EXT_ID_SIGNAL_DEFS_0:
            case CAN_TX_CP_EXT_ID_SIGNAL_DEFS_1:
            case CAN_TX_CP_EXT_ID_SIGNALS_DEFINED:
            case CAN_TX_CP_EXT_ID_SG_DEFS_0:
            case CAN_TX_CP_EXT_ID_SG_DEFS_1:
            case CAN_TX_CP_EXT_ID_SG_DEFS_2:
            case CAN_TX_CP_EXT_ID_SG_DEFS_3:
            case CAN_TX_CP_EXT_ID_SG_DEFS_4:
            case CAN_TX_CP_EXT_ID_SG_DEFS_5:
            case CAN_TX_CP_EXT_ID_SO_DEFS_0:
            case CAN_TX_CP_EXT_ID_SO_DEFS_1:
            case CAN_TX_CP_EXT_ID_CVS_DEFS_0:
            case CAN_TX_CP_EXT_ID_CVS_DEFS_1:
            case CAN_TX_CP_EXT_ID_CVS_DEFS_2:
            case CAN_TX_CP_EXT_ID_CVS_DEFS_3:
            case CAN_TX_CP_EXT_ID_CVS_DEFS_4:
            case CAN_TX_CP_EXT_ID_CONFLICTS_EM:
            case CAN_TX_CP_EXT_ID_FLASH_CFG:
            case CAN_TX_CP_EXT_ID_DEFAULT:
            {
              CanProcessData(pRxMsg->RxHeader.Identifier,
                             (uint8_t *) &(pRxMsg->baData),
                             pRxMsg->RxHeader.DataLength);
              break;
            }
        }

        break;
      }

      case FDCAN_STANDARD_ID:
      {
        switch (pRxMsg->RxHeader.Identifier)
        {
            case CAN_MID_CPU_SO0:
            {
              SOIdentification(SO_SSM_GROUP_1, (tpSCanCpuSO) pRxMsg->baData);
              break;
            }

            case CAN_MID_CPU_SO1:
            {
              SOIdentification(SO_SSM_GROUP_2, (tpSCanCpuSO) pRxMsg->baData);
              osThreadResume(SOCatchTaskHandle);
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
              SetSSMVoltCurrentMeasurements(pRxMsg->RxHeader.Identifier
                                            -
                                            CAN_MID_SSM_VOLT_CURRENT_MEASUREMENTS0,
                                            pRxMsg->baData);
              SOCatchModuleActiveNowSet((pRxMsg->RxHeader.Identifier
                                         -
                                         CAN_MID_SSM_VOLT_CURRENT_MEASUREMENTS0),
                                        TRUE);
              break;
            }

            case CAN_MID_PSM_VOLT_MEASUREMENTS0:
            case CAN_MID_PSM_VOLT_MEASUREMENTS1:
            {
              SetPSMVoltageMeasurements(pRxMsg->RxHeader.Identifier
                                        - CAN_MID_PSM_VOLT_MEASUREMENTS0,
                                        pRxMsg->baData);
              SOCatchModuleActiveNowSet((pRxMsg->RxHeader.Identifier
                                         - CAN_MID_PSM_VOLT_MEASUREMENTS0)
                                        + SIG_DEV_PSM_FIRST,
                                        TRUE);
              break;
            }

            case CAN_MID_CPU_WORKING_LAMP_CHANGE_ACCEPT_TIME:
            {
              SetWorkingLampChangeAcceptCount(pRxMsg->baData[0]);
              break;
            }

            default:
            {
              break;
            }
        } /* switch */

        break;
      }
  } /* switch */

  return TRUE;
} /* CanReqParser */

void CANMsgParserTaskFunc(void const *argument)
{
  UNUSED(argument);

  tpSCanRxMsg pSRxMsg = NULL;

  CANMsgParserInit();
  CANStart(&hfdcan1);

  while (FOREVER)
  {
    if (osMessageQueueGet(CANRxReqsQueueHandle, &pSRxMsg, NULL,
                          osWaitForever) == osOK)
    {
      CanReqParser(pSRxMsg);
      osMemoryPoolFree(CANRxReqsMemPoolHandle, pSRxMsg);
    }
  }
}

void CANMsgSenderTaskFunc(void const *argument)
{
  UNUSED(argument);

  tpSCanTxMsg pSTxMsg = NULL;

  while (FOREVER)
  {
    if (osMessageQueueGet(CANTxReqsQueueHandle, &pSTxMsg, NULL,
                          osWaitForever) == osOK)
    {
      CANSendMessage(pSTxMsg);
      CANWaitTransmissionComplete(pSTxMsg->hfdcan);
      osMemoryPoolFree(CANTxReqsMemPoolHandle, pSTxMsg);
    }
  }
}
