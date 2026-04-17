/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  include files */
#include "cpmpcomm.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "CanMsgParser.h"
#include "MLM.h"
#include "gpio.h"
#include "HardwarePorts.h"
#include "lcd.h"
#include "program.h"

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Definitions */

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  members */
tSCPMPComm SCPMPComm;

uint8_t bComLEDToggleCnt; /* communication LED toggle counter */
uint16_t sCPMPErrorCounter; /* communication error counter */
uint8_t bErrCPMP; /* OS error flag of communication task */

uint8_t fTransmitFlashSignalPackage;

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  methods */
void SetFlashSignalTransmit(uint8_t fState)
{
  fTransmitFlashSignalPackage = fState;
}

uint8_t GetFlashSignalTransmit(void)
{
  return fTransmitFlashSignalPackage;
}

void ResetCPMPComm(void)
{
  CPMPStateSet(PACKET_TYPE_CP_SIGNAL_DEFS);
  CPMPSubStateSet(PACKET_SUB_TYPE_CP_SIGNAL_DEFS_1);

  SCPMPComm.bPacketNo = 0;
  sCPMPErrorCounter = 0;

  InitErrorInfo();

  SetFlashSignalTransmit(TRUE);
}

void CPMPStateSet(uint8_t nState)
{
  SCPMPComm.bCPMPState = nState;
}

uint8_t CPMPStateGet(void)
{
  return SCPMPComm.bCPMPState;
}

void CPMPSubStateSet(uint8_t bSubState)
{
  SCPMPComm.bCPMPSubState = bSubState;
}

uint8_t CPMPSubStateGet(void)
{
  return SCPMPComm.bCPMPSubState;
}

void CPMPResetErrCounter(void)
{
  sCPMPErrorCounter = 0;
}

void CPMPPacketIncrease(void)
{
  CPMPResetErrCounter();
  switch (CPMPStateGet())
  {
      case PACKET_TYPE_CP_SIGNAL_DEFS:
      {
        switch (CPMPSubStateGet())
        {
            case PACKET_SUB_TYPE_CP_SIGNAL_DEFS_1:
            {
              CPMPSubStateSet(PACKET_SUB_TYPE_CP_SIGNAL_DEFS_2);
              break;
            }

            case PACKET_SUB_TYPE_CP_SIGNAL_DEFS_2:
            {
              SCPMPComm.bPacketNo++;

              if (SCPMPComm.bPacketNo >= SIGNALS_MAX)
              {
                SCPMPComm.bPacketNo = 0;
                CPMPSubStateSet(PACKET_SUB_TYPE_CP_NONE);
                CPMPStateSet(PACKET_TYPE_CP_SIGNALS_DEFINED);
              }
              else
              {
                CPMPSubStateSet(PACKET_SUB_TYPE_CP_SIGNAL_DEFS_1);
              }

              break;
            }
        }

        break;
      }

      case PACKET_TYPE_CP_SIGNALS_DEFINED:
      {
        CPMPSubStateSet(PACKET_SUB_TYPE_CP_SG_DEFS_1);
        CPMPStateSet(PACKET_TYPE_CP_SG_DEFS);
        break;
      }

      case PACKET_TYPE_CP_SG_DEFS:
      {
        switch (CPMPSubStateGet())
        {
            case PACKET_SUB_TYPE_CP_SG_DEFS_1:
            {
              CPMPSubStateSet(PACKET_SUB_TYPE_CP_SG_DEFS_2);
              break;
            }

            case PACKET_SUB_TYPE_CP_SG_DEFS_2:
            {
              CPMPSubStateSet(PACKET_SUB_TYPE_CP_SG_DEFS_3);
              break;
            }

            case PACKET_SUB_TYPE_CP_SG_DEFS_3:
            {
              CPMPSubStateSet(PACKET_SUB_TYPE_CP_SG_DEFS_4);
              break;
            }

            case PACKET_SUB_TYPE_CP_SG_DEFS_4:
            {
              CPMPSubStateSet(PACKET_SUB_TYPE_CP_SG_DEFS_5);
              break;
            }

            case PACKET_SUB_TYPE_CP_SG_DEFS_5:
            {
              CPMPSubStateSet(PACKET_SUB_TYPE_CP_SG_DEFS_6);
              break;
            }

            case PACKET_SUB_TYPE_CP_SG_DEFS_6:
            {
              SCPMPComm.bPacketNo++;

              if (SCPMPComm.bPacketNo >= SIGNAL_GROUPS_MAX)
              {
                SCPMPComm.bPacketNo = 0;
                CPMPSubStateSet(PACKET_SUB_TYPE_CP_SO_DEFS_1);
                CPMPStateSet(PACKET_TYPE_CP_SO_DEFS);
              }
              else
              {
                CPMPSubStateSet(PACKET_SUB_TYPE_CP_SG_DEFS_1);
              }

              break;
            }
        } /* switch */

        break;
      }

      case PACKET_TYPE_CP_SO_DEFS:
      {
        switch (CPMPSubStateGet())
        {
            case PACKET_SUB_TYPE_CP_SO_DEFS_1:
            {
              CPMPSubStateSet(PACKET_SUB_TYPE_CP_SO_DEFS_2);
              break;
            }

            case PACKET_SUB_TYPE_CP_SO_DEFS_2:
            {
              SCPMPComm.bPacketNo++;
              if (SCPMPComm.bPacketNo >= SIGNAL_OUTPUTS_MAX)
              {
                SCPMPComm.bPacketNo = 0;
                CPMPSubStateSet(PACKET_SUB_TYPE_CP_CVS_DEFS_1);
                CPMPStateSet(PACKET_TYPE_CP_CVS_DEFS);
              }
              else
              {
                CPMPSubStateSet(PACKET_SUB_TYPE_CP_SO_DEFS_1);
              }

              break;
            }
        }

        break;
      }

      case PACKET_TYPE_CP_CVS_DEFS:
      {
        switch (CPMPSubStateGet())
        {
            case PACKET_SUB_TYPE_CP_CVS_DEFS_1:
            {
              CPMPSubStateSet(PACKET_SUB_TYPE_CP_CVS_DEFS_2);
              break;
            }

            case PACKET_SUB_TYPE_CP_CVS_DEFS_2:
            {
              CPMPSubStateSet(PACKET_SUB_TYPE_CP_CVS_DEFS_3);
              break;
            }

            case PACKET_SUB_TYPE_CP_CVS_DEFS_3:
            {
              CPMPSubStateSet(PACKET_SUB_TYPE_CP_CVS_DEFS_4);
              break;
            }

            case PACKET_SUB_TYPE_CP_CVS_DEFS_4:
            {
              CPMPSubStateSet(PACKET_SUB_TYPE_CP_CVS_DEFS_5);
              break;
            }

            case PACKET_SUB_TYPE_CP_CVS_DEFS_5:
            {
              CPMPSubStateSet(PACKET_SUB_TYPE_CP_NONE);
              CPMPStateSet(PACKET_TYPE_CP_CONFLICTS_EM);
              break;
            }
        }

        break;
      }

      case PACKET_TYPE_CP_CONFLICTS_EM:
      {
        CPMPStateSet(PACKET_TYPE_CP_FLASH_CFG);
        break;
      }

      case PACKET_TYPE_CP_FLASH_CFG:
      {
        CPMPStateSet(PACKET_TYPE_CP_DEFAULT);
        break;
      }

      case PACKET_TYPE_CP_DEFAULT:
      {
        CPMPStateSet(PACKET_TYPE_CP_DEFAULT);
        break;
      }
  } /* switch */
} /* CPMPPacketIncrease */

uint8_t CPMPTxCheck(uint16_t sId, uint8_t *pbData, uint8_t bDataLen)
{
  uint8_t bByteNo = 0;

  if (sId != SCPMPComm.sAnsId)
  {
    return TRUE;
  }

  for (bByteNo = 0; bByteNo < bDataLen; bByteNo++)
  {
    if (pbData[bByteNo] != SCPMPComm.bDataPrev[bByteNo])
    {
      return FALSE;
    }
  }

  return TRUE;
}

uint8_t CPMPPrepareMsg(uint8_t *pbDest,
                       uint8_t *pbSrc,
                       uint16_t *psIndex,
                       uint16_t sDataLen)
{
  uint8_t bCanMsgIndex = 0;

  memset(&(SCPMPComm.bDataNow), 0, sizeof(SCPMPComm.bDataNow));

  for (bCanMsgIndex = 0; bCanMsgIndex < 8; bCanMsgIndex++)
  {
    pbDest[bCanMsgIndex] = pbSrc[*psIndex];

    *psIndex = *psIndex + 1;

    if (*psIndex >= sDataLen)
    {
      bCanMsgIndex++;
      break;
    }
  }

  return bCanMsgIndex;
}

void CPMPCommPacketIncSet(uint8_t bState)
{
  SCPMPComm.bPacketInc = bState;
}

uint8_t CPMPCommPacketIncGet(void)
{
  return SCPMPComm.bPacketInc;
}

void CPMPComTaskFunc(void *argument)
{
  UNUSED(argument);

  while (FOREVER)
  {
    /* set relay state */
    switch (StateCurrentGet())
    {
        case STATES_NO_CONTROL:
        {
          SetPowerRelay(TRUE);
          break;
        }

        default:
        {
          if (CPMPStateGet() == PACKET_TYPE_CP_DEFAULT)
          {
            if (RelayStateRequestGet() == FALSE)
            {
              SetPowerRelay(TRUE);
            }
            else
            {
              /* control if lcd user request exist */
              if (GetLCDPowerRelayRequest())
              {
                /* control if lcd user permits relay can be on, if so, set relay */
                /* to on */
                if (GetLCDPowerRelay())
                {
                  SetPowerRelay(FALSE);
                }
                else
                {
                  SetPowerRelay(TRUE);
                }
              }
              else
              {
                if (GetPowerRelay())
                {
                  SetPowerRelay(FALSE);
                }
              }
            }
          }
          else
          {
            if (GetPowerRelay())
            {
              SetPowerRelay(FALSE);
            }
          }

          break;
        }
    } /* switch */

    if (CPMPCommPacketIncGet())
    {
      CPMPPacketIncrease();
      CPMPCommPacketIncSet(FALSE);
    }

    switch (CPMPStateGet())
    {
        case PACKET_TYPE_CP_NONE:
        {
          break;
        }

        case PACKET_TYPE_CP_SIGNAL_DEFS:
        {
          tSSignalDef SSignalDef;

          GetSignalDefs(SCPMPComm.bPacketNo, &SSignalDef);
          SCPMPComm.pbDataSrc = (uint8_t *) &SSignalDef;
          SCPMPComm.sDataLen = sizeof(tSSignalDef);

          switch (CPMPSubStateGet())
          {
              case PACKET_SUB_TYPE_CP_SIGNAL_DEFS_1:
              {
                if (SCPMPComm.bPacketNo < SIGNALS_MAX)
                {
                  SCPMPComm.sIndex = 0;
                  SCPMPComm.sPacketId = (CAN_TX_CP_EXT_ID_SIGNAL_DEFS_0
                                         | SCPMPComm.bPacketNo);
                  SCPMPComm.sAnsId = (CAN_TX_MP_EXT_ID_SIGNAL_DEFS_0
                                      | SCPMPComm.bPacketNo);
                }

                break;
              }

              case PACKET_SUB_TYPE_CP_SIGNAL_DEFS_2:
              {
                if (SCPMPComm.bPacketNo < SIGNALS_MAX)
                {
                  SCPMPComm.sIndex = 8;
                  SCPMPComm.sPacketId = (CAN_TX_CP_EXT_ID_SIGNAL_DEFS_1
                                         | SCPMPComm.bPacketNo);
                  SCPMPComm.sAnsId = (CAN_TX_MP_EXT_ID_SIGNAL_DEFS_1
                                      | SCPMPComm.bPacketNo);
                }

                break;
              }
          }

          break;
        }

        case PACKET_TYPE_CP_SIGNALS_DEFINED:
        {
          tSSignalsDefined SSignalsDefined;

          GetSignalsDefined(&SSignalsDefined);
          SCPMPComm.pbDataSrc = (uint8_t *) &SSignalsDefined;
          SCPMPComm.sDataLen = sizeof(tSSignalsDefined);

          SCPMPComm.sIndex = 0;
          SCPMPComm.sPacketId = CAN_TX_CP_EXT_ID_SIGNALS_DEFINED;
          SCPMPComm.sAnsId = (CAN_TX_MP_EXT_ID_SIGNALS_DEFINED
                              | SCPMPComm.bPacketNo);
          break;
        }

        case PACKET_TYPE_CP_SG_DEFS:
        {
          tSSGDef SSGDef;

          SGGet(SCPMPComm.bPacketNo, &SSGDef);
          SCPMPComm.pbDataSrc = (uint8_t *) &SSGDef;
          SCPMPComm.sDataLen = sizeof(tSSGDef);

          switch (CPMPSubStateGet())
          {
              case PACKET_SUB_TYPE_CP_SG_DEFS_1:
              {
                if (SCPMPComm.bPacketNo < SIGNAL_GROUPS_MAX)
                {
                  SCPMPComm.sIndex = 0;
                  SCPMPComm.sPacketId = (CAN_TX_CP_EXT_ID_SG_DEFS_0
                                         | SCPMPComm.bPacketNo);
                  SCPMPComm.sAnsId = (CAN_TX_MP_EXT_ID_SG_DEFS_0
                                      | SCPMPComm.bPacketNo);
                }

                break;
              }

              case PACKET_SUB_TYPE_CP_SG_DEFS_2:
              {
                if (SCPMPComm.bPacketNo < SIGNAL_GROUPS_MAX)
                {
                  SCPMPComm.sIndex = 8;
                  SCPMPComm.sPacketId = (CAN_TX_CP_EXT_ID_SG_DEFS_1
                                         | SCPMPComm.bPacketNo);
                  SCPMPComm.sAnsId = (CAN_TX_MP_EXT_ID_SG_DEFS_1
                                      | SCPMPComm.bPacketNo);
                }

                break;
              }

              case PACKET_SUB_TYPE_CP_SG_DEFS_3:
              {
                if (SCPMPComm.bPacketNo < SIGNAL_GROUPS_MAX)
                {
                  SCPMPComm.sIndex = 16;
                  SCPMPComm.sPacketId = (CAN_TX_CP_EXT_ID_SG_DEFS_2
                                         | SCPMPComm.bPacketNo);
                  SCPMPComm.sAnsId = (CAN_TX_MP_EXT_ID_SG_DEFS_2
                                      | SCPMPComm.bPacketNo);
                }

                break;
              }

              case PACKET_SUB_TYPE_CP_SG_DEFS_4:
              {
                if (SCPMPComm.bPacketNo < SIGNAL_GROUPS_MAX)
                {
                  SCPMPComm.sIndex = 24;
                  SCPMPComm.sPacketId = (CAN_TX_CP_EXT_ID_SG_DEFS_3
                                         | SCPMPComm.bPacketNo);
                  SCPMPComm.sAnsId = (CAN_TX_MP_EXT_ID_SG_DEFS_3
                                      | SCPMPComm.bPacketNo);
                }

                break;
              }

              case PACKET_SUB_TYPE_CP_SG_DEFS_5:
              {
                if (SCPMPComm.bPacketNo < SIGNAL_GROUPS_MAX)
                {
                  SCPMPComm.sIndex = 32;
                  SCPMPComm.sPacketId = (CAN_TX_CP_EXT_ID_SG_DEFS_4
                                         | SCPMPComm.bPacketNo);
                  SCPMPComm.sAnsId = (CAN_TX_MP_EXT_ID_SG_DEFS_4
                                      | SCPMPComm.bPacketNo);
                }

                break;
              }

              case PACKET_SUB_TYPE_CP_SG_DEFS_6:
              {
                if (SCPMPComm.bPacketNo < SIGNAL_GROUPS_MAX)
                {
                  SCPMPComm.sIndex = 40;
                  SCPMPComm.sPacketId = (CAN_TX_CP_EXT_ID_SG_DEFS_5
                                         | SCPMPComm.bPacketNo);
                  SCPMPComm.sAnsId = (CAN_TX_MP_EXT_ID_SG_DEFS_5
                                      | SCPMPComm.bPacketNo);
                }

                break;
              }
          } /* switch */

          break;
        }

        case PACKET_TYPE_CP_SO_DEFS:
        {
          tSSODef SSODef;

          GetSODef(SCPMPComm.bPacketNo, &SSODef);
          SCPMPComm.pbDataSrc = (uint8_t *) &SSODef;
          SCPMPComm.sDataLen = sizeof(tSSODef);

          switch (CPMPSubStateGet())
          {
              case PACKET_SUB_TYPE_CP_SO_DEFS_1:
              {
                if (SCPMPComm.bPacketNo < SIGNAL_OUTPUTS_MAX)
                {
                  SCPMPComm.sIndex = 0;
                  SCPMPComm.sPacketId = (CAN_TX_CP_EXT_ID_SO_DEFS_0
                                         | SCPMPComm.bPacketNo);
                  SCPMPComm.sAnsId = (CAN_TX_MP_EXT_ID_SO_DEFS_0
                                      | SCPMPComm.bPacketNo);
                }

                break;
              }

              case PACKET_SUB_TYPE_CP_SO_DEFS_2:
              {
                if (SCPMPComm.bPacketNo < SIGNAL_OUTPUTS_MAX)
                {
                  SCPMPComm.sIndex = 8;
                  SCPMPComm.sPacketId = (CAN_TX_CP_EXT_ID_SO_DEFS_1
                                         | SCPMPComm.bPacketNo);
                  SCPMPComm.sAnsId = (CAN_TX_MP_EXT_ID_SO_DEFS_1
                                      | SCPMPComm.bPacketNo);
                }

                break;
              }
          }

          break;
        }

        case PACKET_TYPE_CP_CVS_DEFS:
        {
          tSCVSDef SCVSDef;

          GetCVSDef(&SCVSDef);
          SCPMPComm.pbDataSrc = (uint8_t *) &SCVSDef;
          SCPMPComm.sDataLen = sizeof(tSCVSDef);

          switch (CPMPSubStateGet())
          {
              case PACKET_SUB_TYPE_CP_CVS_DEFS_1:
              {
                SCPMPComm.sIndex = 0;
                SCPMPComm.sPacketId = (CAN_TX_CP_EXT_ID_CVS_DEFS_0
                                       | SCPMPComm.bPacketNo);
                SCPMPComm.sAnsId = (CAN_TX_MP_EXT_ID_CVS_DEFS_0
                                    | SCPMPComm.bPacketNo);
                break;
              }

              case PACKET_SUB_TYPE_CP_CVS_DEFS_2:
              {
                SCPMPComm.sIndex = 8;
                SCPMPComm.sPacketId = (CAN_TX_CP_EXT_ID_CVS_DEFS_1
                                       | SCPMPComm.bPacketNo);
                SCPMPComm.sAnsId = (CAN_TX_MP_EXT_ID_CVS_DEFS_1
                                    | SCPMPComm.bPacketNo);
                break;
              }

              case PACKET_SUB_TYPE_CP_CVS_DEFS_3:
              {
                SCPMPComm.sIndex = 16;
                SCPMPComm.sPacketId = (CAN_TX_CP_EXT_ID_CVS_DEFS_2
                                       | SCPMPComm.bPacketNo);
                SCPMPComm.sAnsId = (CAN_TX_MP_EXT_ID_CVS_DEFS_2
                                    | SCPMPComm.bPacketNo);
                break;
              }

              case PACKET_SUB_TYPE_CP_CVS_DEFS_4:
              {
                SCPMPComm.sIndex = 24;
                SCPMPComm.sPacketId = (CAN_TX_CP_EXT_ID_CVS_DEFS_3
                                       | SCPMPComm.bPacketNo);
                SCPMPComm.sAnsId = (CAN_TX_MP_EXT_ID_CVS_DEFS_3
                                    | SCPMPComm.bPacketNo);
                break;
              }

              case PACKET_SUB_TYPE_CP_CVS_DEFS_5:
              {
                SCPMPComm.sIndex = 32;
                SCPMPComm.sPacketId = (CAN_TX_CP_EXT_ID_CVS_DEFS_4
                                       | SCPMPComm.bPacketNo);
                SCPMPComm.sAnsId = (CAN_TX_MP_EXT_ID_CVS_DEFS_4
                                    | SCPMPComm.bPacketNo);
                break;
              }
          } /* switch */

          break;
        }

        case PACKET_TYPE_CP_CONFLICTS_EM:
        {
          tSConflictsEM SConflictsEM;

          GetConflictsEM(&SConflictsEM);
          SCPMPComm.pbDataSrc = (uint8_t *) &SConflictsEM;
          SCPMPComm.sDataLen = sizeof(tSConflictsEM);

          SCPMPComm.sIndex = 0;
          SCPMPComm.sPacketId = (CAN_TX_CP_EXT_ID_CONFLICTS_EM
                                 | SCPMPComm.bPacketNo);
          SCPMPComm.sAnsId = (CAN_TX_MP_EXT_ID_CONFLICTS_EM
                              | SCPMPComm.bPacketNo);
          break;
        }

        case PACKET_TYPE_CP_FLASH_CFG:
        {
          uint16_t sEMFlashPeriod = 0;

          sEMFlashPeriod = FlashPeriodEmergencyGet();

          SCPMPComm.pbDataSrc = (uint8_t *) &(sEMFlashPeriod);
          SCPMPComm.sDataLen = sizeof(sEMFlashPeriod);

          SCPMPComm.sIndex = 0;
          SCPMPComm.sPacketId = (CAN_TX_CP_EXT_ID_FLASH_CFG
                                 | SCPMPComm.bPacketNo);
          SCPMPComm.sAnsId = (CAN_TX_MP_EXT_ID_FLASH_CFG | SCPMPComm.bPacketNo);
          break;
        }

        case PACKET_TYPE_CP_DEFAULT:
        {
          tSPeripheralStates SCurrPeriphState;

          GetPeripheralStates(&(SCurrPeriphState));

          SCPMPComm.pbDataSrc = (uint8_t *) &(SCurrPeriphState);
          SCPMPComm.sDataLen = sizeof(tSPeripheralStates);
          SCPMPComm.sIndex = 0;
          SCPMPComm.sPacketId = (CAN_TX_CP_EXT_ID_DEFAULT);
          SCPMPComm.sAnsId = (CAN_TX_MP_EXT_ID_DEFAULT);
          break;
        }
    } /* switch */

    SCPMPComm.bMsgLen = CPMPPrepareMsg((uint8_t *) &(SCPMPComm.bDataNow),
                                       (uint8_t *) (SCPMPComm.pbDataSrc),
                                       &(SCPMPComm.sIndex), SCPMPComm.sDataLen);

    if (SCPMPComm.bMsgLen)
    {
      memcpy(&(SCPMPComm.bDataPrev), &(SCPMPComm.bDataNow), 8);
      CANTxRequest(SCPMPComm.bMsgLen, CAN_ID_TYPE_EXT, SCPMPComm.sPacketId,
                   (uint8_t *) &(SCPMPComm.bDataNow));
    }

    sCPMPErrorCounter++;
    if (sCPMPErrorCounter >= CPMP_ERROR_LIMIT)
    {
      CPMPResetErrCounter();
      LogRequest(LOG_REQ_APPEND_ASYNCH,
                 NULL,
                 EVENT_CPMP_COMM_CP_TIMEOUT,
                 0,
                 0,
                 0,
                 0);
      ApplyEMToAllSets(EMERGENCY_METHOD_FLASH, EVENT_CPMP_COMM_CP_TIMEOUT);
    }

    bComLEDToggleCnt++;
    if (CPMPStateGet() == PACKET_TYPE_CP_DEFAULT)
    {
      if (bComLEDToggleCnt >= 5)
      {
        bComLEDToggleCnt = 0;
        StatusLEDToggle(&g_commLEDPort);
      }
    }
    else
    {
      if (bComLEDToggleCnt >= 50)
      {
        bComLEDToggleCnt = 0;
        StatusLEDToggle(&g_commLEDPort);
      }
    }

    SignalMaintenanceTask(EVENT_FLAGS_MAINTENANCE_CPMPCOM_TASK_ACTIVE);

    osDelay(10);
  }
} /* CPMPComTaskFunc */
