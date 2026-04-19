/* This module includes the task that performs communications with the cards */
/* that are attached to the CAN Bus of the maestro controller device, these */
/* cards are SSMs, PSMs and IOs. */

/* The task communicates with these cards every 10ms, it recieves messages from */
/* SSM and PSM at every rising edge of the (conditioned to a 100Hz square wave), */
/* it recieves messages from IO at every falling edge, it sends messages to SSM, */
/* PSM, IO at every falling edge */

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Include Files */
#include "signalCardDrv.h"

#include <string.h>

#include "CanMsgParser.h"
#include "cpmpcomm.h"
#include "data.h"
#include "iwdg.h"
#include "main.h"
#include "time.h"
#include "gpio.h"
#include "MLM.h"
#include "HardwarePorts.h"
/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Definitions */

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Members */

tSCanCpuSO SaCanCpuSOMsg[CAN_SO_MSG_MAX]; /* signal output states are copied */
/* into this buffer */
tSCanCpuSO SCanCpuSO;

uint8_t bStreamCounter;
uint8_t bPeriodCounter;
uint8_t bLoopStatusRequestPeriodCnt;
uint8_t bLoopDedOpModeEntryReqPeriodCnt;

uint8_t bInputStatusCheckCntr;
uint8_t baLoopErrorCounters[TOTAL_LOOP_DEDECTOR_NUM];
uint8_t bInputErrorCounter;

static uint8_t fSignalsSeized;

/*  Mutex for Signal Groups */
/*  OS Error Flag */

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Methods */
uint8_t SeizeSGData(void)
{
  return osMutexAcquire(SignalGroupsMutexHandle, osWaitForever) == osOK;
}

void ReleaseSGData(void)
{
  osMutexRelease(SignalGroupsMutexHandle);
}

void PrepareTransmitCANPacket(uint16_t sMessageID)
{
  switch (sMessageID)
  {                              /* her case su anlama geliyor : gelen/g�nderilen  id degerine g�re payload ne olacak? */
      case CAN_MID_VERSION:
      {
        CANTxRequest(0, CAN_ID_TYPE_STD, sMessageID, NULL);
        break;
      }

      case CAN_MID_CPU_SO0:
      case CAN_MID_CPU_SO1:   /* anlik fiziksel lamba �ikis durumu /renk/ flash ya da normal // cpu-> saha karti */
      {
        uint8_t bSONo;
        uint8_t bSGNo;  /* sg den so ya map yapiliyor bir sg nin birden fazla fiziksel �ikisi olabilir. */
        uint8_t fFirstStep = TRUE; /* tSCanCpuSO has two 32 bit union, in first */
        /* step, first union is assigned */
        uint8_t bLastSONo, bFirstSONo;
        uint8_t bSetIndex;

        memset(&SCanCpuSO, 0, sizeof(tSCanCpuSO));

        /* seize signal group data */
        if (sMessageID == CAN_MID_CPU_SO0)
        {
          fSignalsSeized = SeizeSGData();
        }

        if (fSignalsSeized)
        {
          /* according to SO numbers, last and first SO numbers will change */
          if (sMessageID == CAN_MID_CPU_SO0)
          {
            bFirstSONo = 0;
            bLastSONo = 48;
          }
          else
          {
            bFirstSONo = 48;
            bLastSONo = 96;
          }

          for (bSONo = bFirstSONo; bSONo < bLastSONo; bSONo++)
          {
            bSetIndex = bSONo - bFirstSONo;
            bSGNo = GetSOOwner(bSONo);

            if (bSGNo)
            {
              uint8_t bSignal, bVoltages;
              uint16_t sPeriod;

              bSignal = SGSignalGet(bSGNo - 1);   /* g�ncel sinyalin sonucu aliniyor.  update signals sonucu okunuyor burasi updatesignals ve data.c ile alakali */
              bVoltages = SignalVoltagesGet(bSignal); /* burasi renk kismi ile ilgili sari kirmizi yesilden hangileri var hangileri yok kontrol� */
              sPeriod = SubSignalHasFlash(bSignal, GetSOType(bSONo)); /* flash mi yoksa sabit yanma mi kontrol� // eger flash ise period d�nd�r�yor degil ise sifir d�nd�r�yor. */

              switch (GetSSMTestSource())
              {
                  case SSM_TEST_FROM_MMI:
                  {
                    uint8_t bOnSO = GetOnSONo();

                    if (bSONo == bOnSO)
                    {
                      if (fFirstStep)
                      {
                        SCanCpuSO.USOOn0.lSOOn |= laValue2Bit[bSetIndex];
                      }
                      else
                      {
                        SCanCpuSO.USOOn1.lSOOn |= laValue2Bit[bSetIndex - 32]; /* 32 is size of array */
                      }
                    }

                    break;
                  }

                  default:
                  {
                    if ((sPeriod == 0) /* if subsignal has not flash */
                        || ((sPeriod > 0) && FlashOnGet(sPeriod))) /* subsignal has flash and flash is */
                    /* on */
                    {
                      /* if the signal includes this output, light on it */
                      if (bVoltages & GetSOType(bSONo))
                      {
                        if (fFirstStep)
                        {
                          SCanCpuSO.USOOn0.lSOOn |= laValue2Bit[bSetIndex];
                        }
                        else
                        {
                          SCanCpuSO.USOOn1.lSOOn |= laValue2Bit[bSetIndex - 32]; /* 32 is size of array */
                        }
                      }
                    }

                    break;
                  }
              } /* switch */
            }

            /* array last index is accessed */
            if (bSetIndex == 31)
            {
              fFirstStep = FALSE;
            }
          }
        }
        else  /* eger sg verisi mutex ile g�venli alinamadiysa yarim veri �retme, son basarili can outputun kopyasini tekrar kullan */
        {
          if (sMessageID == CAN_MID_CPU_SO0)
          {
            memcpy(&SCanCpuSO,
                   &SaCanCpuSOMsg[CAN_SO_MSG_FIR],
                   sizeof(tSCanCpuSO));
          }
          else if (sMessageID == CAN_MID_CPU_SO1)
          {
            memcpy(&SCanCpuSO,
                   &SaCanCpuSOMsg[CAN_SO_MSG_SEC],
                   sizeof(tSCanCpuSO));
          }
        }

        /* release signal group data */
        if (fSignalsSeized && (sMessageID == CAN_MID_CPU_SO1) )
        {
          ReleaseSGData();  /* mutex lock s0 hazirlanirken alindi s1 hazirlandiktan sonra birakiliyor */
          fSignalsSeized = FALSE;
        }

        if (sMessageID == CAN_MID_CPU_SO0)
        {
          memcpy(&SaCanCpuSOMsg[CAN_SO_MSG_FIR], &SCanCpuSO,
                 sizeof(tSCanCpuSO));
          CANTxRequest(CAN_DLC_VALUES_MB_ALL, CAN_ID_TYPE_STD, sMessageID,
                       (uint8_t *) (&SCanCpuSO));
        }
        else if (sMessageID == CAN_MID_CPU_SO1)
        {
          memcpy(&SaCanCpuSOMsg[CAN_SO_MSG_SEC], &SCanCpuSO,
                 sizeof(tSCanCpuSO));
          CANTxRequest(CAN_DLC_VALUES_MB_ALL, CAN_ID_TYPE_STD, sMessageID,
                       (uint8_t *) (&SCanCpuSO));
        }

        if (sMessageID == CAN_MID_CPU_SO1)
        {
          ReleaseSGData();
        }

        break;
      }

      case CAN_MID_LOOP_DEDECTOR_ENTER_OPERATIONAL_MODE:  /* loop detector kartini �alisma moduna sok */
      {
        uint8_t baData[] = { 0x01, 0x00 };

        CANTxRequest(sizeof(baData), CAN_ID_TYPE_STD, sMessageID, baData);
        break;
      }

      case CAN_MID_LOOP_DETECTOR_STATUS_REQUEST0:  /* loop detectorden status bilgisi isteniyor   cpu-> detector mod�l� */
      {
        /* 0x40: SDO Command; read
        ** 0x00: Sub-index; read entire 0x6000h object
        ** 0x60: Index (low-byte) 6000h digital inputs
        ** 0x01: Index (high-byte)
        ** Bytes 4-7: not used
        */

        uint8_t baData[] = { 0x40, 0x00, 0x60, 0x01, 0x00, 0x00, 0x00, 0x00 };
        uint8_t bLDMNo = 0;

        for (bLDMNo = 0; bLDMNo < TOTAL_LOOP_DEDECTOR_NUM; bLDMNo++)
        {
          sMessageID = CAN_MID_LOOP_DETECTOR_STATUS_REQUEST0 + bLDMNo;
          CANTxRequest(sizeof(baData),
                       CAN_ID_TYPE_STD,
                       sMessageID,
                       baData);

          baLoopErrorCounters[bLDMNo]++;
          if (baLoopErrorCounters[bLDMNo] >= CAN_LOOP_DETECTOR_MAX_COM_ERRORS)
          {
            uint8_t baData = 0xF0;
            uint8_t bIOMNo = bLDMNo / LD_NUM_FOR_1_IO_CARD;

            baLoopErrorCounters[bLDMNo] = 0;

            if ((GetErrorInfoPtr()->bErrLDM & laValue2Bit[bLDMNo]) >> bLDMNo
                == 0)
            {
              if (BrokenInputSettingsLoopFlagGet())
              {
                LogRequest(LOG_REQ_APPEND_ASYNCH, NULL, EVENT_MODULE_MISSING,
                           (SIG_DEV_LD_FIRST + bLDMNo), 0, 0, 0);
              }

              GetErrorInfoPtr()->bErrLDM |= laValue2Bit[bLDMNo];
              SRuntimes.sLDMStatus &= ~(laValue2Bit[bLDMNo]);
            }

            if (SaCanDetectorIOInputs[bIOMNo].fIsPhysicallyDriven)
            {
              SetLDInputs(bLDMNo, bIOMNo, &baData);
            }
          }
        }

        break;
      }

      case CAN_MID_CPU_IO_OUTPUTS:
      {
        tpSCanCpuIOOutputs pSCanCpuIOOutputs; /* structure size is not equal to */
        /* CAN_DLC_VALUES_MB_ALL */
        uint8_t sendThis[CAN_DLC_VALUES_MB_ALL];

        pSCanCpuIOOutputs = (tpSCanCpuIOOutputs) sendThis;

        memset(&sendThis, 0, sizeof(sendThis));

        GetIOOutputs(pSCanCpuIOOutputs);

        CANTxRequest(CAN_DLC_VALUES_MB_ALL,
                     CAN_ID_TYPE_STD,
                     sMessageID,
                     sendThis);
        break;
      }

      case CAN_MID_CPU_DATE_TIME:
      {                                      /* zamqn bilgisi tasir, bu bilgiye  ihtiya� duyan mod�ller olabilir */
        tSCanCpuDateTime SCanCpuDateTime;
        tSTime STimeNow;

        memset(&STimeNow, 0, sizeof(STimeNow));
        memset(&SCanCpuDateTime, 0, sizeof(tSCanCpuDateTime));

        TimeGet(&STimeNow);

        SCanCpuDateTime.bHour = STimeNow.SCurrentTime.Hours;
        SCanCpuDateTime.bMinute = STimeNow.SCurrentTime.Minutes;
        SCanCpuDateTime.bSecond = STimeNow.SCurrentTime.Seconds;
        SCanCpuDateTime.bYear = STimeNow.SCurrentDate.Year;
        SCanCpuDateTime.bMonth = STimeNow.SCurrentDate.Month;
        SCanCpuDateTime.bDay = STimeNow.SCurrentDate.Date;

        CANTxRequest(CAN_DLC_VALUES_MB_ALL, CAN_ID_TYPE_STD, sMessageID,
                     (uint8_t *) (&SCanCpuDateTime));
        break;
      }

      case CAN_MID_CPU_EVENT:
      {
        break;
      }

      case CAN_MID_MODULE_CONTROL:
      {
        break;
      }

      case CAN_MID_CPU_FLASH_SIGNALS0:
      case CAN_MID_CPU_FLASH_SIGNALS1:  /* hangi outputlar flash sinyal grubuna dahil?  //tanim/konfig�rasyon mesaji // flash modunda hangi outputlar flash ailesine dahil // flash periyodu nedir ? */
      {
        tSCanCpuFlashSignals SCanCpuFlashSignals;
        uint8_t bSONo;
        uint8_t fFirstStep = TRUE; /* tSCanCpuFlashSignals has two 32 bit union, */
        /* in first step, , first union is assigned */
        uint8_t bLastSONo, bFirstSONo;

        memset(&SCanCpuFlashSignals, 0, sizeof(tSCanCpuFlashSignals));

        SCanCpuFlashSignals.UFlashSignals1.SFlashSignals.bFlashPeriod0 =
          (uint8_t) (FlashPeriodEmergencyGet() / 10);

        /* according to SO numbers, last and first SO numbers will change */
        if (sMessageID == CAN_MID_CPU_FLASH_SIGNALS0)
        {
          bFirstSONo = 0;
          bLastSONo = 48;
        }
        else
        {
          bFirstSONo = 48;
          bLastSONo = 96;
        }

        for (bSONo = bFirstSONo; bSONo < bLastSONo; bSONo++)
        {
          uint8_t bSetIndex = bSONo - bFirstSONo;
          uint8_t bSGNo = GetSOOwner(bSONo);

          if (bSGNo)
          {
            uint8_t bSignal = SGFlashSignalGet(bSGNo - 1);
            uint8_t bVoltages = SignalVoltagesGet(bSignal);

            /* if the signal includes the bSONo, add bSONo */
            if (bVoltages & GetSOType(bSONo))
            {
              if (fFirstStep)
              {
                SCanCpuFlashSignals.UFlashSignals0.lFlashSignals |=
                  laValue2Bit[bSetIndex];
              }
              else
              {
                SCanCpuFlashSignals.UFlashSignals1.lFlashSignals |=
                  laValue2Bit[bSetIndex - 32];                                                 /* 32 is size of array */
              }
            }
          }

          /* array last index is accessed */
          if (bSetIndex == 31)
          {
            fFirstStep = FALSE;
          }
        }

        if (sMessageID == CAN_MID_CPU_FLASH_SIGNALS0)
        {
          CANTxRequest(CAN_DLC_VALUES_MB_ALL, CAN_ID_TYPE_STD, sMessageID,
                       (uint8_t *) (&SCanCpuFlashSignals));
        }
        else if (sMessageID == CAN_MID_CPU_FLASH_SIGNALS1)
        {
          CANTxRequest(CAN_DLC_VALUES_MB_ALL, CAN_ID_TYPE_STD, sMessageID,
                       (uint8_t *) (&SCanCpuFlashSignals));
        }

        break;
      }

      default:
      {
        break;
      }
  } /* switch */
} /* PrepareTransmitCANPacket */

void TransmitCANPackets(void)
{
  PrepareTransmitCANPacket(CAN_MID_VERSION);
  PrepareTransmitCANPacket(CAN_MID_CPU_SO0);
  PrepareTransmitCANPacket(CAN_MID_CPU_SO1);
  PrepareTransmitCANPacket(CAN_MID_CPU_IO_OUTPUTS);

  bPeriodCounter++;
  if (bPeriodCounter == CAN_CPU_DATE_TIME_FLASH_SIGNALS_PERIOD)
  {
    PrepareTransmitCANPacket(CAN_MID_CPU_DATE_TIME);
    bPeriodCounter = 0;
  }

  bLoopDedOpModeEntryReqPeriodCnt++;
  if (bLoopDedOpModeEntryReqPeriodCnt == CAN_LOOP_DEDECTOR_OP_MODE_ENTRY_PERIOD)
  {
    bLoopDedOpModeEntryReqPeriodCnt = 0;
    PrepareTransmitCANPacket(CAN_MID_LOOP_DEDECTOR_ENTER_OPERATIONAL_MODE);
  }

  bLoopStatusRequestPeriodCnt++;
  if (bLoopStatusRequestPeriodCnt >= CAN_LOOP_STATUS_REQUEST_PERIOD)
  {
    PrepareTransmitCANPacket(CAN_MID_LOOP_DETECTOR_STATUS_REQUEST0);
    bLoopStatusRequestPeriodCnt = 0;
  }

  bStreamCounter++;
  if (bStreamCounter == CAN_MMI_SIGNALS_STREAM_PERIOD)
  {
    bStreamCounter = 0;
  }

  if (GetFlashSignalTransmit())
  {
    PrepareTransmitCANPacket(CAN_MID_CPU_FLASH_SIGNALS0);
    PrepareTransmitCANPacket(CAN_MID_CPU_FLASH_SIGNALS1);
    SetFlashSignalTransmit(FALSE);
  }

  bInputStatusCheckCntr++;
  if (bInputStatusCheckCntr > CAN_INPUT_STATUS_CHECK_PERIOD)
  {
    bInputStatusCheckCntr = 0;

    bInputErrorCounter++;
    if (bInputErrorCounter >= CAN_INPUT_MAX_COM_ERRORS)
    {
      tSCanDigitalIOInputs SIOInputs;

      memset(&SIOInputs, 0, sizeof(SIOInputs));

      bInputErrorCounter = 0;

      if (GetErrorInfoPtr()->bErrIOM == 0)
      {
        if (BrokenInputSettingsDigitalFlagGet())
        {
          LogRequest(LOG_REQ_APPEND_ASYNCH, NULL, EVENT_MODULE_MISSING,
                     SIG_DEV_IO_FIRST,
                     0, 0, 0);
        }

        GetErrorInfoPtr()->bErrIOM = 1;
      }

      if (SaCanDigitalIOInputs[0].fIsPhysicallyDriven)
      {
        memcpy(&SIOInputs, &SaCanDigitalIOInputs[0], sizeof(SIOInputs));
        SIOInputs.sInputSafeStates = 0;
        SetIOInputs(0, &SIOInputs);
      }

      if (SaCanDigitalIOInputs[1].fIsPhysicallyDriven)
      {
        memcpy(&SIOInputs, &SaCanDigitalIOInputs[1], sizeof(SIOInputs));
        SIOInputs.sInputSafeStates = 0;
        SetIOInputs(1, &SIOInputs);
      }
    }
  }
} /* TransmitCANPackets */

void SigCardDrvInit(void)
{
  /* period counters */
  bPeriodCounter = 0; /* some packets are sent with different intervals */
  bLoopStatusRequestPeriodCnt = 0;
  bLoopDedOpModeEntryReqPeriodCnt = 0;

  bInputErrorCounter = 0;
  bInputStatusCheckCntr = 0;
  memset(baLoopErrorCounters, 0, sizeof(baLoopErrorCounters));
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Tasks */
void SignalCardDriveTaskFunc(void *argument)
{
  UNUSED(argument);

  uint8_t bIndex = 0;

  SigCardDrvInit();

  while (FOREVER)
  {
    IWDGRefresh();

    /* -------------------------------------- Use I/O Data */
    /* --------------------------------------------------- // */
    if (CPMPStateGet() == PACKET_TYPE_CP_DEFAULT)
    {
      UseIOValues();
    }
    switch (CPMPStateGet())
    {
        case PACKET_TYPE_CP_DEFAULT:
        {
          break;
        }

        default:
        {
          uint8_t bSGNo = 0;

          SeizeSGData();
          for (bSGNo = 0; bSGNo < SIGNAL_GROUPS_MAX; bSGNo++)
          {
            SGSignalSet(bSGNo, SignalsDefinedDarkGet());
          }

          ReleaseSGData();
          break;
        }
    }

    /* -------------------------------------- Flash State Assignment */
    /* ------------------------------------------------------- // */
    for (bIndex = 0; bIndex < SGTotalGet(); bIndex++)
    {
      if (SignalHasFlash(SGSignalGet(bIndex)))
      {
        /* if there is a flash signal between groups, increment flash signal */
        /* counter */
        FlashCntrInc();
        break;
      }
    }

    if (bIndex == SGTotalGet())
    {
      /* there is no flash signal between signal groups so keep counter value at */
      /* zero */
      FlashCntrClear();
    }

    /* transmit can packets */
    TransmitCANPackets();

    SignalMaintenanceTask(
      EVENT_FLAGS_MAINTENANCE_SIGNAL_CARD_DRIVE_TASK_ACTIVE);

    osDelay(10);
  }
} /* SignalCardDriveTaskFunc */
