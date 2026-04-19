/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Include Files */
#include "program.h"

#include <string.h>

#include "MCS.h"
#include "MCSAsynch.h"
#include "MLM.h"
#include "MSM.h"
#include "PPPOSAsynch.h"
#include "cpmpcomm.h"
#include "lcd.h"
#include "main.h"
#include "signalCardDrv.h"
#include "ui.h"
#include "i2c.h"

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Definitions */

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Members */
uint8_t fAllSGsAreInSecure;
tSTaskProgramRuntime STaskProgramRuntime;

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Methods */
void ProgramSigProgChangeSet(uint8_t bState)
{
  STaskProgramRuntime.SFlags.fProgramChanged = bState;
}

uint8_t ProgramSigProgChangeGet(void)
{
  return STaskProgramRuntime.SFlags.fProgramChanged;
}

uint8_t ProgramStateGet(void)
{
  return STaskProgramRuntime.bProgramState;
}

void ProgramStateSet(uint8_t bState)
{
  STaskProgramRuntime.bProgramState = bState;
}

uint8_t ProgramCurrentTransitionGet(void)
{
  return 1;
}

uint8_t ProgramCurrentNoGet(void)
{
  return STaskProgramRuntime.STransitionCur.bValue2 & TRANSITION_VALUE_NO_GET;
}

uint8_t ProgramTargetNoGet(void)
{
  return STaskProgramRuntime.STransitionSel.bValue2 & TRANSITION_VALUE_NO_GET;
}

void SetProgramLoadingStatus(uint8_t bNewStatus)
{
  STaskProgramRuntime.bProgramLoadingStatus = bNewStatus;
}

uint8_t ProgramLoadingStatusGet(void)
{
  return STaskProgramRuntime.bProgramLoadingStatus;
}

void LoadProgramStarts(void)
{
  ProgramStateSet(PROGRAM_STATE_LOADING);
  WorkScheduleDefaultSettings(); /* load default settings */
}

void LoadProgramEnds(void)
{
  ProgramStateSet(PROGRAM_STATE_DARK);
  WorkScheduleUpdate(); /* update settings */
  SetRuntimesInit(); /* init to normal mode running */
}

uint8_t ProgramReadData(void)
{
  uint8_t fProgramLoadingFlag;

  SetProgramLoadingStatus(PROGRAM_LOADING_JUST_STARTED);

  if (!ProgramDataStartMagicRead())
  {
    SetProgramLoadingStatus(PROGRAM_READ_ERR_FLASH);

    return FALSE;
  }

  if (ProgramDataStartMagicGet(0) == SCP_START_MAGIC)
  {
    SetProgramLoadingStatus(PROGRAM_LOADING_IN_PROGRESS);

    if (GetProgramLoadingFlag(&fProgramLoadingFlag) == FALSE)
    {
      SetProgramLoadingStatus(PROGRAM_READ_ERR_EEPROM);

      return FALSE;
    }

    if (fProgramLoadingFlag == TRUE)
    {
      SetProgramLoadingStatus(PROGRAM_LOADING_ERROR); /* program loaded before */
      LogRequest(LOG_REQ_APPEND, NULL, EVENT_MCT_CONFIGURATION_ERROR, 2, 0, 0,
                 0);
      SetProgramLoadingFlag(FALSE);

      SRuntimes.SaSignalStateRuntimes[SignalStateRuntimeCurNoGet()].
      bExecutionMode =
        SIGNAL_STATE_EXEC_MODE_PROGRAM_ERROR;
    }
    else
    {
      if (ProgramDataGet()) /* read flash data to ram */
      {
        ProgramRelativeDataInit();
        SetProgramLoadingStatus(PROGRAM_LOADING_SUCCESS);

        return TRUE;
      }
      else
      {
        SetProgramLoadingStatus(PROGRAM_LOADING_ERROR);
        SetProgramLoadingFlag(FALSE);
      }
    }
  }
  else
  {
    SetProgramLoadingStatus(
      PROGRAM_READ_ERR_FLASH);   /* flash control value error meaning flash is */
    /* empty or wrong or corrupted */
  }

  return FALSE;
} /* ProgramReadData */

uint8_t StateCurrentGet(void)
{
  return STaskProgramRuntime.bCurrentState;
}

void StateCurrentSet(uint8_t bNewState)
{
  STaskProgramRuntime.bCurrentState = bNewState;

  switch (bNewState)
  {
      case STATES_PHASE:
      case STATES_PHASE_TRANSITION:
      case STATES_SEQ:
      {
        if (GetPowerRelay())
        {
          SRuntimes.SaSignalStateRuntimes[SignalStateRuntimeCurNoGet()].
          bExecutionMode = SIGNAL_STATE_EXEC_MODE_PROGRAM;
          if (InputTotalGet(INPUT_TYPE_DETECTOR))
          {
            uint8_t bSGNo = 0;
            uint8_t fOwned = FALSE;

            for (bSGNo = 0; bSGNo < SGTotalGet(); bSGNo++)
            {
              uint8_t bLDNo = 0;

              for (bLDNo = 0;
                   bLDNo < InputTotalGet(INPUT_TYPE_DETECTOR);
                   bLDNo++)
              {
                if (InputOwnerSGGet(INPUT_TYPE_DETECTOR, bLDNo) == bSGNo + 1)
                {
                  fOwned = TRUE;
                  break;
                }
              }

              if (!fOwned)
              {
                break;
              }
            }

            if (fOwned)
            {
              SRuntimes.SaSignalStateRuntimes[SignalStateRuntimeCurNoGet()].
              bPlanMode =
                SIGNAL_STATE_PLAN_MODE_FULLY_ACTUATED;
            }
            else
            {
              SRuntimes.SaSignalStateRuntimes[SignalStateRuntimeCurNoGet()].
              bPlanMode =
                SIGNAL_STATE_PLAN_MODE_HALF_ACTUATED;
            }
          }
          else
          {
            SRuntimes.SaSignalStateRuntimes[SignalStateRuntimeCurNoGet()].
            bPlanMode = SIGNAL_STATE_PLAN_MODE_FIXED_PLAN;
          }
        }
        else
        {
          SRuntimes.SaSignalStateRuntimes[SignalStateRuntimeCurNoGet()].
          bExecutionMode = SIGNAL_STATE_EXEC_MODE_TEST_SP;
        }

        break;
      }

      case STATES_FLASH:
      {
        SRuntimes.SaSignalStateRuntimes[SignalStateRuntimeCurNoGet()].
        bExecutionMode = SIGNAL_STATE_EXEC_MODE_FLASH;
        break;
      }

      case STATES_NO_CONTROL:
      {
        SRuntimes.SaSignalStateRuntimes[SignalStateRuntimeCurNoGet()].
        bExecutionMode = SIGNAL_STATE_EXEC_MODE_ALL_DARK;
        break;
      }

      case STATES_CLOSED:
      {
        SRuntimes.SaSignalStateRuntimes[SignalStateRuntimeCurNoGet()].
        bExecutionMode = SIGNAL_STATE_EXEC_MODE_ALL_RED;
        break;
      }
  } /* switch */
} /* StateCurrentSet */

void StateCurrentInit(void)
{
  uint8_t bSGNo;

  /* initial state of device is no control. In this state, signal must be dark */
  /* to provide safe signal assignment */
  memset(&STaskProgramRuntime.STransitionCur, 0, sizeof(tSTransition));
  memset(&STaskProgramRuntime.STransitionSel, 0, sizeof(tSTransition));

  DataRuntimeInit();
  StateCurrentSet(STATES_NO_CONTROL);

  STaskProgramRuntime.STransitionCur.bTo = StateCurrentGet(); /* this is also current state */
  for (bSGNo = 0; bSGNo < SGTotalGet(); bSGNo++)
  {
    SGRuntimeDataSet(bSGNo, SignalsDefinedDarkGet(), SIGNAL_GROUP_STATE_NONE);
  }
}

uint8_t MaxTransitionTime(uint8_t *pbWillBeClosed, uint8_t *pbWillBeOpened)
{
  uint8_t bSimOpenTime = 0;
  uint8_t bSGNo;
  uint8_t bFromPhase = ProgramCurrentNoGet();
  uint8_t bToPhase = ProgramTargetNoGet();

  /* find max. simultaneous opening time (green flash plus closing plus */
  /* clearance) */
  for (bSGNo = 0; bSGNo < SGTotalGet(); bSGNo++)
  {
    if (SGIsFlasher(bSGNo + 1) == FALSE)
    {
      if (bFromPhase)
      {
        if (PhaseHasSG(bToPhase - 1,
                       bSGNo) && (PhaseHasSG(bFromPhase - 1, bSGNo) == FALSE))
        {
          uint8_t bOtherSGNo;

          for (bOtherSGNo = 0; bOtherSGNo < SGTotalGet(); bOtherSGNo++)
          {
            if (PhaseHasSG(bFromPhase - 1,
                           bOtherSGNo) && (PhaseHasSG(bToPhase - 1,
                                                      bOtherSGNo) == FALSE))
            {
              uint8_t bOpenTime = (SGClearanceGet(bOtherSGNo,
                                                  bSGNo)
                                   + SGClosingDurGet(bOtherSGNo)
                                   + SGGreenFlashDurGet(bOtherSGNo));

              if (bSimOpenTime <= bOpenTime)
              {
                bSimOpenTime = bOpenTime;
                *pbWillBeOpened = (bSGNo + 1);
                *pbWillBeClosed = (bOtherSGNo + 1);
              }
            }
          }
        }
      }
      else
      {
        if (PhaseHasSG(bToPhase - 1, bSGNo))
        {
          *pbWillBeOpened = (bSGNo + 1);
        }
      }
    }
  }

  return bSimOpenTime;
} /* MaxTransitionTime */

uint8_t MaxTransitionTimeSG(uint8_t bOtherSGNo)
{
  uint8_t bSimOpenTime = 0;
  uint8_t bSGNo;
  uint8_t bFromPhase = ProgramCurrentNoGet();
  uint8_t bToPhase = ProgramTargetNoGet();

  /* find max. simultaneous opening time (green flash plus closing plus */
  /* clearance) */
  for (bSGNo = 0; bSGNo < SGTotalGet(); bSGNo++)
  {
    if (SGIsFlasher(bSGNo + 1) == FALSE)
    {
      if (PhaseHasSG(bToPhase - 1,
                     bSGNo) && (PhaseHasSG(bFromPhase - 1, bSGNo) == FALSE))
      {
        if (PhaseHasSG(bFromPhase - 1,
                       bOtherSGNo) && (PhaseHasSG(bToPhase - 1,
                                                  bOtherSGNo) == FALSE))
        {
          uint8_t bOpenTime = (SGClearanceGet(bOtherSGNo,
                                              bSGNo)
                               + SGClosingDurGet(bOtherSGNo)
                               + SGGreenFlashDurGet(bOtherSGNo));

          if (bSimOpenTime <= bOpenTime)
          {
            bSimOpenTime = bOpenTime;
          }
        }
      }
    }
  }

  return bSimOpenTime;
}

void UpdateSignals(void)
{
  uint8_t bSGNo;
  uint8_t bFromPhase = ProgramCurrentNoGet();
  uint8_t bToPhase = ProgramTargetNoGet();

  SeizeSGData();

  /* --------------------- If SG Is Flasher, It Has Flash Signal, Independently */
  /* From Current State ------------------------------------ // */
  switch (StateCurrentGet())
  {
      case STATES_SECURE_TRANSITION:
      {
        fAllSGsAreInSecure = TRUE;
        for (bSGNo = 0; bSGNo < SGTotalGet(); bSGNo++)
        {
          if (SGSignalGet(bSGNo) != SignalsDefinedBlockingGet())
          {
            fAllSGsAreInSecure = FALSE;
            if (SGSignalGet(bSGNo) == SGClosingSignalGet(bSGNo))
            {
              if (SGDurationGet(bSGNo) >= SGClosingDurGet(bSGNo))
              {
                SGRuntimeDataSet(bSGNo, SignalsDefinedBlockingGet(),
                                 SIGNAL_GROUP_STATE_CLOSED);
              }
            }
            else if (SGSignalGet(bSGNo) == SignalsDefinedDarkGet())
            {
              SGRuntimeDataSet(bSGNo, SignalsDefinedBlockingGet(),
                               SIGNAL_GROUP_STATE_CLOSED);
            }
            else
            {
              if (SGSignalGet(bSGNo) != SGOpeningSignalGet(bSGNo))
              {
                if (SGSignalGet(bSGNo) == SignalsDefinedFreeGet())
                {
                  if (SGDurationGet(bSGNo) >= 10)
                  {
                    if ((SGGreenFlashDurGet(bSGNo) > 0)
                        && (SGSignalGet(bSGNo)
                            != SignalsDefinedGreenFlashGet()))
                    {
                      SGRuntimeDataSet(bSGNo, SignalsDefinedGreenFlashGet(),
                                       SIGNAL_GROUP_STATE_GREEN_FLASH);
                    }
                    else
                    {
                      SGRuntimeDataSet(bSGNo, SGClosingSignalGet(bSGNo),
                                       SIGNAL_GROUP_STATE_CLOSING);
                    }
                  }
                }
                else
                {
                  if (SGSignalGet(bSGNo) == SignalsDefinedGreenFlashGet())
                  {
                    if (SGGreenFlashDurGet(bSGNo) > 0)
                    {
                      if (SGDurationGet(bSGNo) >= SGGreenFlashDurGet(bSGNo))
                      {
                        SGRuntimeDataSet(bSGNo, SGClosingSignalGet(bSGNo),
                                         SIGNAL_GROUP_STATE_CLOSING);
                      }
                    }
                    else
                    {
                      SGRuntimeDataSet(bSGNo, SGClosingSignalGet(bSGNo),
                                       SIGNAL_GROUP_STATE_CLOSING);
                    }
                  }
                  else
                  {
                    SGRuntimeDataSet(bSGNo, SGClosingSignalGet(bSGNo),
                                     SIGNAL_GROUP_STATE_CLOSING);
                  }
                }
              }
              else
              {
                if (SGDurationGet(bSGNo) >= SGOpeningDurGet(bSGNo))
                {
                  SGRuntimeDataSet(bSGNo, SGClosingSignalGet(bSGNo),
                                   SIGNAL_GROUP_STATE_CLOSING);
                }
              }
            }
          }
          else
          {
            SGRuntimeDataSet(bSGNo, SignalsDefinedBlockingGet(),
                             SIGNAL_GROUP_STATE_CLOSED);
          }
        }

        if (fAllSGsAreInSecure)
        {
          if (UserStateReqGet() != STATES_NONE)
          {
            UserStateReqSet(STATES_NONE);
            StateCurrentSet(UserStateCurrentGet());
          }
          else
          {
            StateCurrentSet(STATES_CLOSED);
          }
        }

        break;
      }

      case STATES_NO_CONTROL:
      {
        for (bSGNo = 0; bSGNo < SGTotalGet(); bSGNo++)
        {
          SGRuntimeDataSet(bSGNo, SignalsDefinedDarkGet(),
                           SIGNAL_GROUP_STATE_NONE);
        }

        break;
      }

      case STATES_FLASH:
      {
        for (bSGNo = 0; bSGNo < SGTotalGet(); bSGNo++)
        {
          SGRuntimeDataSet(bSGNo, SGFlashSignalGet(bSGNo),
                           SIGNAL_GROUP_STATE_FLASH);
        }

        break;
      }

      case STATES_CLOSED:
      {
        for (bSGNo = 0; bSGNo < SGTotalGet(); bSGNo++)
        {
          SGRuntimeDataSet(bSGNo, SignalsDefinedBlockingGet(),
                           SIGNAL_GROUP_STATE_CLOSED);
        }

        break;
      }

      case STATES_SEQ:
      {
        for (bSGNo = 0; bSGNo < SGTotalGet(); bSGNo++)
        {
          if (SGIsFlasher(bSGNo + 1))
          {
            SGRuntimeDataSet(bSGNo, SGFlashSignalGet(bSGNo),
                             SIGNAL_GROUP_STATE_FLASHER);
          }
          else
          {
            SGRuntimeDataSet(bSGNo, SeqStepSGSignalGet(SeqCurrentGet() - 1,
                                                       SeqCurrentStepGet(),
                                                       bSGNo),
                             SIGNAL_GROUP_STATE_SEQ_SIGNAL);
          }
        }

        break;
      }

      case STATES_PHASE:
      {
        for (bSGNo = 0; bSGNo < SGTotalGet(); bSGNo++)
        {
          if (SGIsFlasher(bSGNo + 1))
          {
            SGRuntimeDataSet(bSGNo, SGFlashSignalGet(bSGNo),
                             SIGNAL_GROUP_STATE_FLASHER);
          }
          else if (PhaseHasSG(bFromPhase - 1, bSGNo))
          {
            SGRuntimeDataSet(bSGNo, SignalsDefinedFreeGet(),
                             SIGNAL_GROUP_STATE_OPEN);
          }
          else
          {
            SGRuntimeDataSet(bSGNo, SignalsDefinedBlockingGet(),
                             SIGNAL_GROUP_STATE_CLOSED);
          }
        }

        break;
      }

      case STATES_PHASE_TRANSITION:
      {
        /* in here, primitive object is phases, there are current and target */
        /* phases, the aim is to switch from current phase to target phase in a */
        /* secure way caring about opening/closing/green flash/clearance durations */
        /* calculate the total durations of (GreenFlashDuration + ClosingDuration) */
        /* for each signal groups the maximum of them is used for extending the */
        /* green duration of others if fSimultaneousClosure is set */

        /* first loop through signal groups and start to close the ones that does */
        /* not belong to the next phase the decision to open groups must be made */
        /* later after closure operations of all groups the termination of closing */
        /* and opening states can also be managed in this loop */
        for (bSGNo = 0; bSGNo < SGTotalGet(); bSGNo++)
        {
          if (SGIsFlasher(bSGNo + 1))
          {
            SGRuntimeDataSet(bSGNo, SGFlashSignalGet(bSGNo),
                             SIGNAL_GROUP_STATE_FLASHER);
          }
          else
          {
            switch (SGStateGet(bSGNo))
            {
                case SIGNAL_GROUP_STATE_GREEN_FLASH:
                {
                  /* close group if green flash duration has elapsed */
                  if (SGDurationGet(bSGNo) >= SGGreenFlashDurGet(bSGNo))
                  {
                    if (SGClosingDurGet(bSGNo))
                    {
                      SGRuntimeDataSet(bSGNo, SGClosingSignalGet(bSGNo),
                                       SIGNAL_GROUP_STATE_CLOSING);
                    }
                    else
                    {
                      SGRuntimeDataSet(bSGNo, SignalsDefinedBlockingGet(),
                                       SIGNAL_GROUP_STATE_CLOSED);
                    }
                  }

                  break;
                }

                case SIGNAL_GROUP_STATE_CLOSING:
                {
                  /* close group if closing duration has elapsed */
                  if (SGDurationGet(bSGNo) >= SGClosingDurGet(bSGNo))
                  {
                    SGRuntimeDataSet(bSGNo, SignalsDefinedBlockingGet(),
                                     SIGNAL_GROUP_STATE_CLOSED);
                  }

                  break;
                }

                case SIGNAL_GROUP_STATE_OPENING:
                {
                  /* open group if opening duration has elapsed */
                  if (SGDurationGet(bSGNo) >= SGOpeningDurGet(bSGNo))
                  {
                    SGRuntimeDataSet(bSGNo, SignalsDefinedFreeGet(),
                                     SIGNAL_GROUP_STATE_OPEN);
                  }

                  break;
                }

                case SIGNAL_GROUP_STATE_OPEN:
                {
                  /* since this group is open it is included in the current phase */
                  /* see if it is included in the next phase, if not, it will be */
                  /* closed if simultaneous closing doesn't exist, apply state */
                  /* transitions here, else do it later */
                  if (PhaseHasSG(bToPhase - 1, bSGNo) == FALSE)
                  {
                    uint8_t fStartClosure = TRUE;

                    if (STaskProgramRuntime.STransitionSel.bValue1
                        & TRANSITION_VALUE_SIM_GET)
                    {
                      uint8_t bIndex = 0;
                      uint8_t bMaxClosureTimeOfAllSG = 0; /* transition max. clearance time */
                      uint8_t bSGNoFirstClosure = 0; /* owner of transition max. clearance time */
                      uint8_t bRemainingTime = 0;

                      /* find the group that has maximum value of (green flash plus */
                      /* closing signal) duration */
                      for (bIndex = 0; bIndex < SGTotalGet(); bIndex++)
                      {
                        if (PhaseHasSG(bFromPhase - 1, bIndex))
                        {
                          if (PhaseHasSG(bToPhase - 1,
                                         bIndex) == FALSE)
                          {
                            if (SGGreenFlashDurGet(bIndex)
                                + SGClosingDurGet(bIndex)
                                >= bMaxClosureTimeOfAllSG)
                            {
                              bMaxClosureTimeOfAllSG =
                                SGGreenFlashDurGet(bIndex)
                                + SGClosingDurGet(bIndex);
                              bSGNoFirstClosure = bIndex;
                              /* there may be other SGs that have the same closure */
                              /* duration, these will also start closing operation */
                            }
                          }
                        }
                      }

                      /* the group that started closure operation is not the bSGNo */
                      /* group */
                      if (bSGNoFirstClosure != bSGNo)
                      {
                        /* to decide start of closure operation of bSGNo we must */
                        /* find remaining time to the end of the closure operation */
                        /* of first closed sg, this end point is also end point for */
                        /* all SGs because we are in simultaneous closure operation */
                        switch (SGStateGet(bSGNoFirstClosure))
                        {
                            case SIGNAL_GROUP_STATE_GREEN_FLASH:
                            {
                              bRemainingTime =
                                (SGGreenFlashDurGet(bSGNoFirstClosure)
                                 + SGClosingDurGet(bSGNoFirstClosure))
                                - SGDurationGet(
                                  bSGNoFirstClosure);
                              break;
                            }

                            case SIGNAL_GROUP_STATE_CLOSING:
                            {
                              bRemainingTime =
                                (SGClosingDurGet(bSGNoFirstClosure))
                                - SGDurationGet(bSGNoFirstClosure);
                              break;
                            }

                            case SIGNAL_GROUP_STATE_CLOSED:
                            {
                              bRemainingTime = 0;
                              break;
                            }

                            case SIGNAL_GROUP_STATE_OPEN:
                            {
                              bRemainingTime =
                                (SGGreenFlashDurGet(bSGNoFirstClosure)
                                 + SGClosingDurGet(bSGNoFirstClosure));
                              break;
                            }
                        }

                        if (bRemainingTime
                            > (SGGreenFlashDurGet(bSGNo)
                               + SGClosingDurGet(bSGNo)))
                        {
                          fStartClosure = FALSE;
                        }
                      }
                    }
                    else if ((STaskProgramRuntime.STransitionSel.bValue2
                              & TRANSITION_VALUE_SIM_GET))
                    {
                      uint8_t bSGWillBeClosed = 0;
                      uint8_t bSGWillBeOpened = 0;
                      uint8_t bSimOpenSafetyTime = 0;

                      MaxTransitionTime(&bSGWillBeClosed, &bSGWillBeOpened);
                      /* remaining times differ according to the current state of */
                      /* the one that will be closed. */
                      switch (SGStateGet(bSGWillBeClosed - 1))
                      {
                          case SIGNAL_GROUP_STATE_OPEN:
                          {
                            bSimOpenSafetyTime =
                              SGClearanceGet(bSGWillBeClosed - 1,
                                             bSGWillBeOpened - 1)
                              + SGClosingDurGet(
                                bSGWillBeClosed - 1)
                              + SGGreenFlashDurGet(bSGWillBeClosed - 1);
                            break;
                          }

                          case SIGNAL_GROUP_STATE_GREEN_FLASH:
                          {
                            bSimOpenSafetyTime =
                              SGClearanceGet(bSGWillBeClosed - 1,
                                             bSGWillBeOpened - 1)
                              + SGClosingDurGet(
                                bSGWillBeClosed - 1)
                              + (SGGreenFlashDurGet(
                                   bSGWillBeClosed - 1)
                                 - SGDurationGet(bSGWillBeClosed - 1));
                            break;
                          }

                          case SIGNAL_GROUP_STATE_CLOSING:
                          {
                            bSimOpenSafetyTime =
                              SGClearanceGet(bSGWillBeClosed - 1,
                                             bSGWillBeOpened - 1)
                              + (SGClosingDurGet(
                                   bSGWillBeClosed - 1)
                                 - SGDurationGet(bSGWillBeClosed - 1));
                            break;
                          }

                          case SIGNAL_GROUP_STATE_CLOSED:
                          {
                            bSimOpenSafetyTime =
                              SGClearanceGet(bSGWillBeClosed - 1,
                                             bSGWillBeOpened - 1)
                              - SGDurationGet(
                                bSGWillBeClosed - 1);
                            break;
                          }

                          default:
                          {
                            fStartClosure = FALSE;
                            break;
                          }
                      } /* switch */

                      if (bSimOpenSafetyTime > MaxTransitionTimeSG(bSGNo))
                      {
                        fStartClosure = FALSE;
                      }
                    }

                    if (fStartClosure)
                    {
                      if (SGGreenFlashDurGet(bSGNo))
                      {
                        SGRuntimeDataSet(bSGNo, SignalsDefinedGreenFlashGet(),
                                         SIGNAL_GROUP_STATE_GREEN_FLASH);
                      }
                      else if (SGClosingDurGet(bSGNo))
                      {
                        SGRuntimeDataSet(bSGNo, SGClosingSignalGet(bSGNo),
                                         SIGNAL_GROUP_STATE_CLOSING);
                      }
                      else
                      {
                        SGRuntimeDataSet(bSGNo, SignalsDefinedBlockingGet(),
                                         SIGNAL_GROUP_STATE_CLOSED);
                      }
                    }
                  }

                  break;
                }
            } /* switch */
          }
        }

        if (STaskProgramRuntime.STransitionSel.bValue2
            & TRANSITION_VALUE_SIM_GET)
        {
          uint8_t bSGWillBeClosed = 0;
          uint8_t bSGWillBeOpened = 0;
          uint8_t bSimOpenSafetyTime = 0;

          MaxTransitionTime(&bSGWillBeClosed, &bSGWillBeOpened);
          /* time is in advance, simultaneous safety time is decreasing, calculate */
          /* current simultaneous safety time */
          if (bSGWillBeClosed && bSGWillBeOpened)
          {
            uint8_t fSecure = TRUE;

            switch (SGStateGet(bSGWillBeClosed - 1))
            {
                case SIGNAL_GROUP_STATE_GREEN_FLASH:
                {
                  bSimOpenSafetyTime = SGClearanceGet(bSGWillBeClosed - 1,
                                                      bSGWillBeOpened - 1)
                                       + SGClosingDurGet(bSGWillBeClosed - 1)
                                       + (SGGreenFlashDurGet(bSGWillBeClosed
                                                             - 1)
                                          - SGDurationGet(bSGWillBeClosed - 1));
                  break;
                }

                case SIGNAL_GROUP_STATE_CLOSING:
                {
                  bSimOpenSafetyTime = SGClearanceGet(bSGWillBeClosed - 1,
                                                      bSGWillBeOpened - 1)
                                       + (SGClosingDurGet(bSGWillBeClosed - 1)
                                          - SGDurationGet(bSGWillBeClosed - 1));
                  break;
                }

                case SIGNAL_GROUP_STATE_CLOSED:
                {
                  bSimOpenSafetyTime = SGClearanceGet(bSGWillBeClosed - 1,
                                                      bSGWillBeOpened - 1)
                                       - SGDurationGet(bSGWillBeClosed - 1);
                  break;
                }

                default:
                {
                  fSecure = FALSE;
                  break;
                }
            }

            if (fSecure)
            {
              for (bSGNo = 0; bSGNo < SGTotalGet(); bSGNo++)
              {
                if (SGIsFlasher(bSGNo + 1) == FALSE)
                {
                  if (PhaseHasSG(bToPhase - 1, bSGNo))
                  {
                    if ((SGStateGet(bSGNo) == SIGNAL_GROUP_STATE_CLOSED)
                        || (SGStateGet(bSGNo)
                            ==
                            SIGNAL_GROUP_STATE_SEQ_SIGNAL))
                    {
                      /* if current simultaneous safety time (remaining time to */
                      /* end of simultaneous opening) is equal to opening duration */
                      /* of group, open group */
                      if (SGOpeningDurGet(bSGNo) >= bSimOpenSafetyTime)
                      {
                        if (SGOpeningDurGet(bSGNo))
                        {
                          SGRuntimeDataSet(bSGNo, SGOpeningSignalGet(bSGNo),
                                           SIGNAL_GROUP_STATE_OPENING);
                        }
                        else
                        {
                          SGRuntimeDataSet(bSGNo, SignalsDefinedFreeGet(),
                                           SIGNAL_GROUP_STATE_OPEN);
                        }
                      }
                    }
                  }
                }
              }
            }
          }
          else if (bSGWillBeOpened)
          {
            for (bSGNo = 0; bSGNo < SGTotalGet(); bSGNo++)
            {
              if (SGIsFlasher(bSGNo + 1) == FALSE)
              {
                if (PhaseHasSG(bToPhase - 1, bSGNo))
                {
                  if ((SGStateGet(bSGNo) == SIGNAL_GROUP_STATE_CLOSED)
                      || (SGStateGet(bSGNo)
                          ==
                          SIGNAL_GROUP_STATE_SEQ_SIGNAL))
                  {
                    /* if current simultaneous safety time (remaining time to end */
                    /* of simultaneous opening) is equal to opening duration of */
                    /* group, open group */
                    if (SGOpeningDurGet(bSGNo))
                    {
                      SGRuntimeDataSet(bSGNo, SGOpeningSignalGet(bSGNo),
                                       SIGNAL_GROUP_STATE_OPENING);
                    }
                    else
                    {
                      SGRuntimeDataSet(bSGNo, SignalsDefinedFreeGet(),
                                       SIGNAL_GROUP_STATE_OPEN);
                    }
                  }
                }
              }
            }
          }
        }
        else
        {
          for (bSGNo = 0; bSGNo < SGTotalGet(); bSGNo++)
          {
            if (SGIsFlasher(bSGNo + 1) == FALSE)
            {
              if (PhaseHasSG(bToPhase - 1, bSGNo))
              {
                /* this group belongs to the next phase, if it is closed, it will */
                /* be opened */
                if (SGStateGet(bSGNo) == SIGNAL_GROUP_STATE_CLOSED)
                {
                  uint8_t fSecure = TRUE;
                  uint8_t bOtherSGNo;

                  /* check if it is ok to open this group */
                  /* bOtherSGNo is the group that will be closed if it is not in */
                  /* the next phase */
                  for (bOtherSGNo = 0; bOtherSGNo < SGTotalGet(); bOtherSGNo++)
                  {
                    if (SGIsFlasher(bSGNo + 1) == FALSE)
                    {
                      /* comment here for conflicts groups that are in phases */
                      /* which are not if(PhaseHasSG(bCurrentPhase, bOtherSGNo) && */
                      /* PhaseHasSG(bTargetPhase, bOtherSGNo) == FALSE) */
                      {
                        /* if there is a group that conflicts with the group that */
                        /* will be opened, the latter must wait for closing of the */
                        /* former */
                        if (SGConflictGet(bOtherSGNo, bSGNo))
                        {
                          int16_t bSafetyTime = 0;

                          /* there is a group that should not be open at the same */
                          /* time with this sg. if this group has an opening */
                          /* duration, it can start opening if, at the end of its */
                          /* opening state, the clearance duration requirement */
                          /* would be satisfied if it doesn't have an opening */
                          /* duration, it can go to open state if clearance */
                          /* duration requirement is satisfied */
                          switch (SGStateGet(bOtherSGNo))
                          {
                              case SIGNAL_GROUP_STATE_GREEN_FLASH:
                              {
                                bSafetyTime = SGClearanceGet(bOtherSGNo,
                                                             bSGNo)
                                              + SGClosingDurGet(bOtherSGNo)
                                              + (SGGreenFlashDurGet(
                                                   bOtherSGNo)
                                                 - SGDurationGet(bOtherSGNo));
                                break;
                              }

                              case SIGNAL_GROUP_STATE_CLOSING:
                              {
                                bSafetyTime = SGClearanceGet(bOtherSGNo, bSGNo)
                                              + (SGClosingDurGet(bOtherSGNo)
                                                 - SGDurationGet(bOtherSGNo));
                                break;
                              }

                              case SIGNAL_GROUP_STATE_CLOSED:
                              {
                                bSafetyTime = SGClearanceGet(bOtherSGNo,
                                                             bSGNo)
                                              - SGDurationGet(bOtherSGNo);
                                break;
                              }

                              default:
                              {
                                fSecure = FALSE;
                                break;
                              }
                          }

                          /* is it secure to open this group */
                          if (bSafetyTime > SGOpeningDurGet(bSGNo))
                          {
                            fSecure = FALSE;
                          }
                        }
                      }
                    }
                  }

                  if (fSecure)
                  {
                    /* ----------------------------- It Is Safe To Open This SG */
                    /* ---------------------------------------------- // */

                    if (SGOpeningDurGet(bSGNo))
                    {
                      SGRuntimeDataSet(bSGNo, SGOpeningSignalGet(bSGNo),
                                       SIGNAL_GROUP_STATE_OPENING);
                    }
                    else
                    {
                      SGRuntimeDataSet(bSGNo, SignalsDefinedFreeGet(),
                                       SIGNAL_GROUP_STATE_OPEN);
                    }
                  }
                }
              }
            }
          }
        }

        /* ------------------------------------- Is Transition Completed ? */
        /* ----------------------------------------------------------- // */

        /* if all signal groups are opened in the phase for the selected */
        /* transition, state must be changed to state phase from state phase */
        /* transition. */
        for (bSGNo = 0; bSGNo < SGTotalGet(); bSGNo++)
        {
          if (SGIsFlasher(bSGNo + 1) == FALSE)
          {
            if (PhaseHasSG(bToPhase - 1, bSGNo))
            {
              if (SGStateGet(bSGNo) != SIGNAL_GROUP_STATE_OPEN)
              {
                break; /* transition is not completed */
              }
            }
          }
        }

        if (bSGNo == SGTotalGet())
        {
          /* ------------------------------------- Transition Is Completed */
          /* -------------------------------------------------------- // */

          uint8_t bRuleAddress = STaskProgramRuntime.STransitionSel.bRule;

          /* execute statements when rule is true, new phase is started in the */
          /* following statements */
          StatementExecuteRange(RuleTOpsStartGet(bRuleAddress),
                                RuleTOpsEndGet(bRuleAddress));

          /* phase transition is completed, switch current state */
          StateCurrentSet(STATES_PHASE);
          memcpy(&STaskProgramRuntime.STransitionCur,
                 &STaskProgramRuntime.STransitionSel,
                 sizeof(tSTransition));
        }

        break;
      }
  } /* switch */

  ReleaseSGData();
} /* UpdateSignals */

void InitProgramTask(void)
{
  StateCurrentSet(STATES_NONE);

  memset(&STaskProgramRuntime.STransitionCur, 0, sizeof(tSTransition));
  memset(&STaskProgramRuntime.STransitionSel, 0, sizeof(tSTransition));

  STaskProgramRuntime.STransitionCur.bTo = STATES_NO_CONTROL;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Tasks */
void ProgramTaskFunc(void *argument)
{
  UNUSED(argument);

  uint8_t bTraNo = 0, bCounterNo = 0;

  InitProgramTask();

  /* init current state data */
  StateCurrentInit();

  /* inform program loading starts */
  ProgramStateSet((uint8_t) PROGRAM_STATE_LOADING);

  /* get data and update program data according to schedule */
  if (ProgramReadData() == FALSE)
  {
    DataInit(UI_REQ_TYPE_NONE, TRUE);
    ProgramDataSet();
  }

  DataValidate();

  ProgramRelativeDataInit();

  WorkScheduleUpdate();

  /* init set data */
  SetRuntimesInit();

  /* inform program loading ends */
  ProgramStateSet(PROGRAM_STATE_DARK); /* only different from program loading */

  ResetCPMPComm();

  while (FOREVER)
  {
    osThreadFlagsWait(THREAD_FLAGS_PROGRAM_SEC_ELAPSED, osFlagsWaitAll,
                      osWaitForever);

    UIRuntimeTimeoutsChecks();

    /* examine traffic actuated periodical values */
    IOPerValsInit();

    /* one second has elapsed, increment period of signal program */
    SigProgCurTimeInPerInc();

    if ((ProgramStateGet() != PROGRAM_STATE_LOADING)
        && (CPMPStateGet() == PACKET_TYPE_CP_DEFAULT))
    {
      if (UserStateRunning())
      {
        StateCurrentSet(UserStateCurrentGet()); /* ignore work schedule */
      }
      else if ((SetSigModeIsOK() == FALSE) || GetPoliceButtonState())
      {
        /* all sets are in emergency method so it is not necessary to examine */
        /* work schedule */
      }
      else
      {
        /* --------------------------------------- Current State Jobs */
        /* -------------------------------------------- // */
        switch (StateCurrentGet())
        {
            case STATES_PHASE:
            {
              uint8_t bPhaseNo = ProgramCurrentNoGet();
              uint8_t bPhaseDur = WorkPlanPhaseDurGet(bPhaseNo - 1);

              PhaseDurInc(bPhaseNo - 1);

              SRuntimes.SaSignalStateRuntimes[SignalStateRuntimeCurNoGet()].
              bPhaseNo = bPhaseNo;
              SRuntimes.SaSignalStateRuntimes[SignalStateRuntimeCurNoGet()].
              bStepNo = 1;
              SRuntimes.SaSignalStateRuntimes[SignalStateRuntimeCurNoGet()].
              bStepTime = bPhaseDur;
              SRuntimes.SaSignalStateRuntimes[SignalStateRuntimeCurNoGet()].
              bElapsedTime = PhaseElapsedDurGet(
                bPhaseNo - 1);
              SRuntimes.SaSignalStateRuntimes[SignalStateRuntimeCurNoGet()].
              bRemainingTime = bPhaseDur
                               -
                               PhaseElapsedDurGet(bPhaseNo - 1);
              SRuntimes.SaSignalStateRuntimes[SignalStateRuntimeCurNoGet()].
              bCycleTime = WorkPlanTotalPhaseDurGet();
              SRuntimes.SaSignalStateRuntimes[SignalStateRuntimeCurNoGet()].
              bTimeToNexCycle = WorkPlanTotalPhaseDurGet()
                                -
                                PhaseTotalElapsedDurGet();
              SRuntimes.SaSignalStateRuntimes[SignalStateRuntimeCurNoGet()].
              bNextPhase = ProgramTargetNoGet();
              break;
            }

            case STATES_SEQ:
            {
              SeqDurInc();

              SRuntimes.SaSignalStateRuntimes[SignalStateRuntimeCurNoGet()].
              bPhaseNo = SeqCurrentGet();
              SRuntimes.SaSignalStateRuntimes[SignalStateRuntimeCurNoGet()].
              bStepNo = SeqCurrentStepGet() + 1;
              SRuntimes.SaSignalStateRuntimes[SignalStateRuntimeCurNoGet()].
              bStepTime = SeqCurrentStepDurationGet();
              SRuntimes.SaSignalStateRuntimes[SignalStateRuntimeCurNoGet()].
              bElapsedTime =
                SeqCurrentStepCurrentDurationGet();
              SRuntimes.SaSignalStateRuntimes[SignalStateRuntimeCurNoGet()].
              bRemainingTime = SeqCurrentStepDurationGet()
                               -
                               SeqCurrentStepCurrentDurationGet();
              SRuntimes.SaSignalStateRuntimes[SignalStateRuntimeCurNoGet()].
              bCycleTime = SeqTotalExtDurGet()
                           +
                           SeqDurGet(SeqCurrentGet() - 1);
              SRuntimes.SaSignalStateRuntimes[SignalStateRuntimeCurNoGet()].
              bTimeToNexCycle = SeqTotalExtDurGet()
                                +
                                SeqDurGet(SeqCurrentGet() - 1)
                                -
                                SeqDurCurGet();
              break;
            }

            case STATES_PHASE_TRANSITION:
            {
              SRuntimes.SaSignalStateRuntimes[SignalStateRuntimeCurNoGet()].
              bNextPhase = ProgramTargetNoGet();
              SRuntimes.SaSignalStateRuntimes[SignalStateRuntimeCurNoGet()].
              fIsTransitionStep = 1;
              SRuntimes.SaSignalStateRuntimes[SignalStateRuntimeCurNoGet()].
              bTransitionStepNo = 1;
              break;
            }
        } /* switch */

        SGDurInc();

        WorkScheduleUpdate();

        /* ------------------------------ Increment Allocated and Running */
        /* Counters ---------------------------------------- // */

        for (bCounterNo = 1; bCounterNo <= COUNTERS_MAX; bCounterNo++)
        {
          if (CounterAllocatedGet(bCounterNo))
          {
            if (CounterRunningGet(bCounterNo))
            {
              CounterValueAdd(bCounterNo, 1);
            }
          }
        }

        if (StateCurrentGet() != STATES_PHASE_TRANSITION)
        {
          /* ------------------------------ Control If Rules Are True Or False */
          /* ------------------------------------------// */
          uint8_t bRuleNo = 0;
          uint8_t bTriggeredTransition = 0;
          uint8_t bTriggeredTransitionPriority = 0;

          /* Signal Program Changed? */
          if (ProgramSigProgChangeGet())
          {
            uint8_t bTransitionNo = 0;

            /* Check if there is a transition defined from this state */
            for (bTransitionNo = 1;
                 bTransitionNo <= TransitionTotalGet(SigProgCurNoGet());
                 bTransitionNo++)
            {
              if ((TransitionLockIsActive() && TransitionLockGet(bTraNo))
                  == FALSE)
              {
                /* check if transition is applicable, i.e. "from" state is */
                /* current state or any state */
                uint8_t bFrom = TransitionFromGet(bTransitionNo);

                if ((bFrom == StateCurrentGet()) || (bFrom == STATES_ANY))
                {
                  if ((bFrom == STATES_ANY)
                      || (TransitionFromNoGet(bTransitionNo)
                          == ProgramCurrentNoGet()))
                  {
                    /* There is a transition defined from this state */
                    ProgramSigProgChangeSet(FALSE);
                    break;
                  }
                }
              }
            }

            /* There is no defined transition from this state */
            /* So that, find the transition defined from No Control State */
            /* Check safety of this state for this transition */
            if (bTransitionNo > TransitionTotalGet(SigProgCurNoGet()))
            {
              for (bTransitionNo = 1;
                   bTransitionNo <= TransitionTotalGet(SigProgCurNoGet());
                   bTransitionNo++)
              {
                if ((TransitionLockIsActive() && TransitionLockGet(bTraNo))
                    == FALSE)
                {
                  if (TransitionFromGet(bTransitionNo) == STATES_NO_CONTROL)
                  {
                    switch (StateCurrentGet())
                    {
                        case STATES_PHASE:
                        {
                          PhaseStop(ProgramCurrentNoGet() - 1); /* stop current */
                          /* phase */
                          break;
                        }

                        case STATES_SEQ:
                        {
                          SeqStop(SeqCurrentGet() - 1);
                          break;
                        }
                    }

                    /* start the transition */
                    if (TransitionToGet(bTransitionNo) == STATES_PHASE)
                    {
                      if (StateCurrentGet() == STATES_PHASE)
                      {
                        /* if current state is phase and transition target is a */
                        /* phase, set state to phase transition "if true" */
                        /* statements will be executed at the end of the phase */
                        /* transition */
                        TransitionGet(SigProgCurNoGet(),
                                      bTransitionNo,
                                      &STaskProgramRuntime.STransitionSel);                                   /* STransitionSel is used */
                        /* when executing the phase */
                        /* transition */
                        StateCurrentSet(STATES_PHASE_TRANSITION);
                      }
                      else
                      {
                        /* if current state is not phase execute "if true" */
                        /* statements and switch to target phase */
                        StatementExecuteRange(RuleTOpsStartGet(
                                                TransitionRuleGet(
                                                  bTransitionNo)),
                                              RuleTOpsEndGet(TransitionRuleGet(
                                                               bTransitionNo)));
                        TransitionGet(SigProgCurNoGet(),
                                      bTransitionNo,
                                      &STaskProgramRuntime.STransitionCur);
                        StateCurrentSet(TransitionToGet(bTransitionNo));
                      }
                    }
                    else
                    {
                      StatementExecuteRange(RuleTOpsStartGet(TransitionRuleGet(
                                                               bTransitionNo)),
                                            RuleTOpsEndGet(TransitionRuleGet(
                                                             bTransitionNo)));
                      TransitionGet(SigProgCurNoGet(),
                                    bTransitionNo,
                                    &STaskProgramRuntime.STransitionCur);
                      StateCurrentSet(TransitionToGet(bTransitionNo));
                    }

                    ProgramSigProgChangeSet(FALSE);
                    break;
                  }
                }
              }
            }
          }

          for (bRuleNo = 1;
               bRuleNo <= RuleTotalGet(SigProgCurNoGet());
               bRuleNo++)
          {
            if (RuleState(bRuleNo))
            {
              uint8_t bTransitionNo = 0;
              uint8_t fTriggersTransition = FALSE;

              /* check if rule triggers a transition */
              for (bTransitionNo = 1;
                   bTransitionNo <= TransitionTotalGet(SigProgCurNoGet());
                   bTransitionNo++)
              {
                if ((TransitionLockIsActive() && TransitionLockGet(bTraNo))
                    == FALSE)
                {
                  if (TransitionRuleGet(bTransitionNo) == bRuleNo)
                  {
                    uint8_t bFrom = TransitionFromGet(bTransitionNo);

                    fTriggersTransition = TRUE;
                    /* check if transition is applicable, i.e. "from" state is */
                    /* current state or any state */
                    if ((bFrom == StateCurrentGet()) || (bFrom == STATES_ANY))
                    {
                      if ((bFrom == STATES_ANY)
                          || (TransitionFromNoGet(bTransitionNo)
                              == ProgramCurrentNoGet()))
                      {
                        /* check if this transition has the highest priority */
                        /* among applicable transitions */
                        uint8_t bTransitionPriority =
                          TransitionPriorityGet(bTransitionNo);

                        if (bTransitionPriority > bTriggeredTransitionPriority)
                        {
                          bTriggeredTransitionPriority = bTransitionPriority;
                          bTriggeredTransition = bTransitionNo;
                        }
                      }
                    }
                  }
                }
              }

              /* the "if true" statements of rules that trigger transition */
              /* will be executed when the transition is executed */
              if (fTriggersTransition == FALSE)
              {
                if ((RuleTOpsStartGet(bRuleNo)) && (RuleTOpsEndGet(bRuleNo))
                    && (RuleTOpsStartGet(bRuleNo) <= RuleTOpsEndGet(bRuleNo)))
                {
                  uint8_t bIndex = 0;

                  for (bIndex = RuleTOpsStartGet(bRuleNo);
                       bIndex <= RuleTOpsEndGet(bRuleNo);
                       bIndex++)
                  {
                    switch (StatementCommandGet(bIndex))
                    {
                        case COMMAND_COUNTER_START:
                        case COMMAND_COUNTER_STOP:
                        case COMMAND_PHASE_EXTEND:
                        case COMMAND_USER_STATE_REQ_END:
                        case COMMAND_PHASE_DELETE_RUN_INFO:
                        case COMMAND_SIG_PROG_PER_RESTART:
                        case COMMAND_SG_ADD_TO_PHASE:
                        case COMMAND_SG_SUB_FROM_PHASE:
                        case COMMAND_SG_ADD_TO_FLASHER_LIST:
                        case COMMAND_SG_SUB_FROM_FLASHER_LIST:
                        case COMMAND_USER_STATE_TO_CURRENT_STATE:
                        case COMMAND_SEQ_ADD_STEP:
                        case COMMAND_SEQ_REMOVE_SECONDS:
                        case COMMAND_TRANSITIONS_LOCK_ADD:
                        case COMMAND_TRANSITIONS_LOCK_END:
                        {
                          StatementExecute(bIndex);
                          break;
                        }

                        default:
                        {
                          break;
                        }
                    }
                  }
                }
              }
            }
            else
            {
              /* execute "if rule is false" statemats */
              if ((RuleFOpsStartGet(bRuleNo)) && (RuleFOpsEndGet(bRuleNo))
                  && (RuleFOpsStartGet(bRuleNo) <= RuleFOpsEndGet(bRuleNo)))
              {
                uint8_t bIndex = 0;

                for (bIndex = RuleFOpsStartGet(bRuleNo);
                     bIndex <= RuleFOpsEndGet(bRuleNo);
                     bIndex++)
                {
                  switch (StatementCommandGet(bIndex))
                  {
                      case COMMAND_COUNTER_START:
                      case COMMAND_COUNTER_STOP:
                      case COMMAND_PHASE_EXTEND:
                      case COMMAND_USER_STATE_REQ_END:
                      case COMMAND_PHASE_DELETE_RUN_INFO:
                      case COMMAND_SIG_PROG_PER_RESTART:
                      case COMMAND_SG_ADD_TO_PHASE:
                      case COMMAND_SG_SUB_FROM_PHASE:
                      case COMMAND_SG_ADD_TO_FLASHER_LIST:
                      case COMMAND_SG_SUB_FROM_FLASHER_LIST:
                      case COMMAND_USER_STATE_TO_CURRENT_STATE:
                      case COMMAND_SEQ_ADD_STEP:
                      case COMMAND_SEQ_REMOVE_SECONDS:
                      case COMMAND_TRANSITIONS_LOCK_ADD:
                      case COMMAND_TRANSITIONS_LOCK_END:
                      {
                        StatementExecute(bIndex);
                        break;
                      }

                      default:
                      {
                        break;
                      }
                  }
                }
              }
            }
          }

          if (bTriggeredTransition)
          {
            if ((StateCurrentGet() != STATES_PHASE)
                || ((StateCurrentGet() == STATES_PHASE)
                    && (
                      PhaseMinDurHasElapsed(ProgramCurrentNoGet() - 1))))
            {
              switch (StateCurrentGet())
              {
                  case STATES_PHASE:
                  {
                    PhaseStop(ProgramCurrentNoGet() - 1); /* stop current phase */
                    break;
                  }

                  case STATES_SEQ:
                  {
                    SeqStop(SeqCurrentGet() - 1);
                    break;
                  }
              }

              /* start the transition */
              if (TransitionToGet(bTriggeredTransition) == STATES_PHASE)
              {
                if (StateCurrentGet() == STATES_PHASE)
                {
                  /* if current state is phase and transition target is a phase, */
                  /* set state to phase transition "if true" statements will be */
                  /* executed at the end of the phase transition */
                  TransitionGet(SigProgCurNoGet(),
                                bTriggeredTransition,
                                &STaskProgramRuntime.STransitionSel);                                          /* STransitionSel is */
                  /* used when */
                  /* executing the */
                  /* phase transition */
                  StateCurrentSet(STATES_PHASE_TRANSITION);
                }
                else
                {
                  /* if current state is not phase execute "if true" statements */
                  /* and switch to target phase */
                  StatementExecuteRange(RuleTOpsStartGet(TransitionRuleGet(
                                                           bTriggeredTransition)),
                                        RuleTOpsEndGet(TransitionRuleGet(
                                                         bTriggeredTransition)));
                  TransitionGet(SigProgCurNoGet(),
                                bTriggeredTransition,
                                &STaskProgramRuntime.STransitionCur);
                  StateCurrentSet(TransitionToGet(bTriggeredTransition));
                }
              }
              else
              {
                StatementExecuteRange(RuleTOpsStartGet(TransitionRuleGet(
                                                         bTriggeredTransition)),
                                      RuleTOpsEndGet(TransitionRuleGet(
                                                       bTriggeredTransition)));
                TransitionGet(SigProgCurNoGet(),
                              bTriggeredTransition,
                              &STaskProgramRuntime.STransitionCur);
                StateCurrentSet(TransitionToGet(bTriggeredTransition));
              }
            }
          }
          else
          {
            if (UserStateReqGet() != STATES_NONE)
            {
              StateCurrentSet(STATES_SECURE_TRANSITION);
            }
          }
        }
      }

      switch (GetSSMTestSource())
      {
          case SSM_TEST_FROM_MMI:
          {
            break;
          }

          default:
          {
            UpdateSignals();
            break;
          }
      }
    }
    else
    {
      /* program loading and sending new data to mp processor is in progress */
      /* init all runtime data */
      StateCurrentInit();
    }

    SignalMaintenanceTask(EVENT_FLAGS_MAINTENANCE_PROGRAM_TASK_ACTIVE);
  }
} /* ProgramTaskFunc */
