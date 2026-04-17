/* //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */
/*                              misc header                               // */
/* //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */
#include "signalCheck.h"
#include "CANRxTx.h"
#include "signalOutputCatch.h"
#include <string.h>
#include "iwdg.h"
#include "maintenance.h"
/* //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */
/*                              Definitions                             // */

/* //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */
/*                              Private Data                              // */
/* //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */
/* Displayed signals array */
/* keeps the signal displayed at each signal group for assigned signals */
/* eg. [0][SIGNAL_RED] keeps the actual displayed signal while signal group 0 is assigned SIGNAL_RED */
static uint8_t baaDisplayedVSAssignedVoltages[SIGNAL_GROUPS_MAX][SIGNALS_MAX];

/* displayed signals: keeps the currently displayed signal at each signal group */
static uint8_t baDisplayedVoltages[SIGNAL_GROUPS_MAX];

/* previous signals: keeps the last displayed signal at each signal group */
static uint8_t baPreviousVoltages[SIGNAL_GROUPS_MAX];

static uint8_t baInvalidVoltages[SIGNAL_GROUPS_MAX];

static uint8_t baFirstInvalidSignalSequences[SIGNAL_GROUPS_MAX];
static uint8_t baSecondInvalidSignalSequences[SIGNAL_GROUPS_MAX];

static uint8_t baaYellowYellowConflictCounter[SIGNAL_GROUPS_MAX][
  SIGNAL_GROUPS_MAX];
static uint8_t baaYellowGreenConflictCounter[SIGNAL_GROUPS_MAX][
  SIGNAL_GROUPS_MAX];
static uint8_t baaGreenGreenConflictCounter[SIGNAL_GROUPS_MAX][SIGNAL_GROUPS_MAX];

/* ///////////////////////////////////////////////////////// */
/*                    Private Methods */
/* calculate the failure state of the lamps connected to an output according to a supplied power consumption value */
/* returns the number of working lamps */
/* newcurrent is the one that is consumed while only this output is active */
/* but prevCurrent is unknown */
static uint8_t DetermineSOLampsState(uint8_t bOutput,
                                     uint16_t sNewCurrent,
                                     uint16_t sPrevCurrent)
{
  UNUSED(sPrevCurrent);

  uint8_t bWorkingLamps;
  uint16_t sCurrentDiff = 0, sExpectedSOCurrent = 0, sLampCurrent = 0;

  if (bOutput < SIGNAL_OUTPUTS_MAX)
  {
    bWorkingLamps = GetSONoOfWorkingLamps(bOutput);
    sLampCurrent = GetSOMinPower(bOutput);
    sExpectedSOCurrent = sLampCurrent * bWorkingLamps;

    /* check for a dramatic change in output's current consumption */
    if (sNewCurrent >= sExpectedSOCurrent)
    {
      do
      {
        sCurrentDiff = sNewCurrent - sExpectedSOCurrent;
        if (sCurrentDiff >= (LAMP_CURRENT_PERCENTAGE * (double) sLampCurrent))
        {
          bWorkingLamps++;
          sExpectedSOCurrent += sLampCurrent;
        }
        else
        {
          break;
        }
      } while (sNewCurrent > sExpectedSOCurrent);
    }
    else /* if the current measured current is lower than expected */
    {
      do
      {
        sCurrentDiff = sExpectedSOCurrent - sNewCurrent;  /* get the difference */
        if (sCurrentDiff >= (LAMP_CURRENT_PERCENTAGE * ((double) sLampCurrent)))
        {
          bWorkingLamps--;
          sExpectedSOCurrent -= sLampCurrent;
        }
        else
        {
          break;
        }
      } while (sExpectedSOCurrent > sNewCurrent);
    }

    /* make sure this is a permanent working lamp count change and register events according to the change */
    if (bWorkingLamps != GetSONoOfWorkingLamps(bOutput)) /* if detected lamp count is not equal to expected lamp count */
    {
      if (bWorkingLamps == GetNoOfLastDetectedWorkingLamps(bOutput)) /* if detected lamp count is equal to currently decided lamp count */
      {
        if (GetWorkingLampChangeCounter(bOutput)
            < GetWorkingLampChangesAcceptCount())
        {
          if (IncreaseWorkingLampChangeCounter(bOutput)
              == GetWorkingLampChangesAcceptCount())
          {
            uint8_t bOwnerSG = GetSOOwner(bOutput) - 1;

            if (!LampDimmingStateChangedGet())
            {
              LogRequest(EVENT_SO_WORKING_LAMP_TOTAL_CHANGE,
                         (bOutput + 1),
                         bWorkingLamps,
                         GetSONoOfWorkingLamps(bOutput));

              SetSONoOfWorkingLamps(bOutput, bWorkingLamps); /* the position of this line is important */

              /* if there is only one lamp at this output, it is possible that there is an emergency method for this output */
              if ((GetSOEM(bOutput) != EMERGENCY_METHOD_NONE)
                  && (GetSONoOfWorkingLamps(bOutput) == 0))
              {
                SetEmergencyMethodSet(GetSOEM(bOutput), GetSGOwner(bOwnerSG));
              }

              /* the number of working lamps may be decreased or increased */
              if (bWorkingLamps < GetSONoOfLamps(bOutput))
              {
                switch (GetSOType(bOutput))
                {
                    case SIGNAL_OUTPUT_TYPE_RED:
                    {
                      /* check for last red and number of red failures */
                      if (GetSGNoOfLamps(bOwnerSG,
                                         SIGNAL_OUTPUT_TYPE_RED)
                          == GetSGNoOfFailedLamps(bOwnerSG,
                                                  SIGNAL_OUTPUT_TYPE_RED))
                      {
                        if (GetSGLastRedLampFailureEM(bOwnerSG))
                        {
                          SetEmergencyMethodSet(GetSGLastRedLampFailureEM(
                                                  bOwnerSG),
                                                GetSGOwner(bOwnerSG));
                        }

                        if (SGRuntimeLampFailLastRedGet(bOwnerSG) == FALSE)
                        {
                          LogRequest(EVENT_SG_LAST_RED_LAMP_FAILURE,
                                     GetSGOwner(bOwnerSG) + 1, (bOwnerSG + 1),
                                     1);
                        }

                        SGRuntimeLampFailLastRedSet(bOwnerSG, TRUE);
                      }
                      else
                      {
                        SGRuntimeLampFailLastRedSet(bOwnerSG, FALSE);
                        SGRuntimeLampFailRedAllSet(bOwnerSG, FALSE);

                        if (GetSGRedLampFailureNumber(bOwnerSG))
                        {
                          if (bWorkingLamps
                              <= GetSGRedLampFailureNumber(bOwnerSG))
                          {
                            if (GetSGRedLampFailureNumberEM(bOwnerSG))
                            {
                              SetEmergencyMethodSet(GetSGRedLampFailureNumberEM(
                                                      bOwnerSG),
                                                    GetSGOwner(bOwnerSG));
                            }

                            if (SGRuntimeLampFailNumberOfRedGet(bOwnerSG)
                                == FALSE)
                            {
                              LogRequest(EVENT_SG_NUMBER_OF_RED_LAMPS_FAILURE,
                                         (bOwnerSG + 1),
                                         0,
                                         0);
                            }

                            SGRuntimeLampFailNumberOfRedSet(bOwnerSG, TRUE);
                          }
                          else
                          {
                            SGRuntimeLampFailNumberOfRedSet(bOwnerSG,
                                                            FALSE);
                          }
                        }
                      }

                      SGRuntimeLampFailRedSet(bOwnerSG, TRUE);
                      LogRequest(EVENT_SG_RED_LAMP_FAILURE, (bOwnerSG + 1), 0,
                                 0);
                      break;
                    }

                    case SIGNAL_OUTPUT_TYPE_YELLOW:
                    {
                      SGRuntimeLampFailYellowSet(bOwnerSG, TRUE);
                      LogRequest(EVENT_SG_YELLOW_LAMP_FAILURE,
                                 (bOwnerSG + 1),
                                 0,
                                 0);
                      break;
                    }

                    case SIGNAL_OUTPUT_TYPE_GREEN:
                    {
                      SGRuntimeLampFailGreenSet(bOwnerSG, TRUE);
                      LogRequest(EVENT_SG_GREEN_LAMP_FAILURE,
                                 (bOwnerSG + 1),
                                 0,
                                 0);
                      break;
                    }
                } /* switch */

                if (bWorkingLamps > 0)
                {
                  switch (GetSOType(bOutput))
                  {
                      case SIGNAL_OUTPUT_TYPE_RED:
                      {
                        SGRuntimeLampFailLastRedSet(bOwnerSG, FALSE);
                        SGRuntimeLampFailRedAllSet(bOwnerSG, FALSE);
                        break;
                      }

                      case SIGNAL_OUTPUT_TYPE_YELLOW:
                      {
                        SGRuntimeLampFailYellowAllSet(bOwnerSG, FALSE);
                        break;
                      }

                      case SIGNAL_OUTPUT_TYPE_GREEN:
                      {
                        SGRuntimeLampFailGreenAllSet(bOwnerSG, FALSE);
                        break;
                      }
                  }
                }
                else
                {
                  switch (GetSOType(bOutput))
                  {
                      case SIGNAL_OUTPUT_TYPE_RED:
                      {
                        if (SGRuntimeLampFailRedAllGet(bOwnerSG) == FALSE)
                        {
                          LogRequest(EVENT_SG_ALL_RED_LAMPS_BROKEN,
                                     (bOwnerSG + 1), 0, 0);
                          SGRuntimeLampFailRedAllSet(bOwnerSG, TRUE);
                        }

                        break;
                      }

                      case SIGNAL_OUTPUT_TYPE_YELLOW:
                      {
                        if (SGRuntimeLampFailYellowAllGet(bOwnerSG) == FALSE)
                        {
                          LogRequest(EVENT_SG_ALL_YELLOW_LAMPS_BROKEN,
                                     (bOwnerSG + 1), 0, 0);
                          SGRuntimeLampFailYellowAllSet(bOwnerSG, TRUE);
                        }

                        break;
                      }

                      case SIGNAL_OUTPUT_TYPE_GREEN:
                      {
                        if (SGRuntimeLampFailGreenAllGet(bOwnerSG) == FALSE)
                        {
                          LogRequest(EVENT_SG_ALL_GREEN_LAMPS_BROKEN,
                                     (bOwnerSG + 1), 0, 0);
                          SGRuntimeLampFailGreenAllSet(bOwnerSG, TRUE);
                        }

                        break;
                      }
                  }
                }
              }
              else if (bWorkingLamps == GetSONoOfLamps(bOutput)) /* if detected Lamp count is equal to expected Lamp count */
              {
                uint8_t bOwnerSG = GetSOOwner(bOutput) - 1;

                switch (GetSOType(bOutput))
                {
                    case SIGNAL_OUTPUT_TYPE_RED:
                    {
                      SGRuntimeLampFailLastRedSet(bOwnerSG, FALSE);
                      SGRuntimeLampFailRedAllSet(bOwnerSG, FALSE);

                      if (SGRuntimeLampFailRedGet(bOwnerSG))
                      {
                        LogRequest(EVENT_SG_ALL_RED_LAMPS_SAFE,
                                   (bOwnerSG + 1),
                                   0,
                                   0);
                      }

                      SGRuntimeLampFailRedSet(bOwnerSG, FALSE);
                      break;
                    }

                    case SIGNAL_OUTPUT_TYPE_YELLOW:
                    {
                      SGRuntimeLampFailYellowAllSet(bOwnerSG, FALSE);

                      if (SGRuntimeLampFailYellowGet(bOwnerSG))
                      {
                        LogRequest(EVENT_SG_ALL_YELLOW_LAMPS_SAFE,
                                   (bOwnerSG + 1),
                                   0,
                                   0);
                      }

                      SGRuntimeLampFailYellowSet(bOwnerSG, FALSE);
                      break;
                    }

                    case SIGNAL_OUTPUT_TYPE_GREEN:
                    {
                      SGRuntimeLampFailGreenAllSet(bOwnerSG, FALSE);

                      if (SGRuntimeLampFailGreenGet(bOwnerSG))
                      {
                        LogRequest(EVENT_SG_ALL_GREEN_LAMPS_SAFE,
                                   (bOwnerSG + 1),
                                   0,
                                   0);
                      }

                      SGRuntimeLampFailGreenSet(bOwnerSG, FALSE);
                      break;
                    }
                } /* switch */
              }
            }
            else if (bWorkingLamps == 0)
            {
              if (GetSOType(bOutput) == SIGNAL_OUTPUT_TYPE_RED)
              {
                if (GetSGLastRedLampFailureEM(bOwnerSG))
                {
                  SetEmergencyMethodSet(GetSGLastRedLampFailureEM(bOwnerSG),
                                        GetSGOwner(bOwnerSG));
                }

                if (SGRuntimeLampFailLastRedGet(bOwnerSG) == FALSE)
                {
                  LogRequest(EVENT_SG_LAST_RED_LAMP_FAILURE,
                             GetSGOwner(bOwnerSG) + 1,
                             (bOwnerSG + 1),
                             1);
                }

                SGRuntimeLampFailLastRedSet(bOwnerSG,
                                            TRUE);

                if (SGRuntimeLampFailRedAllGet(bOwnerSG) == FALSE)
                {
                  LogRequest(EVENT_SG_ALL_RED_LAMPS_BROKEN, (bOwnerSG + 1), 0,
                             0);
                  SGRuntimeLampFailRedAllSet(bOwnerSG, TRUE);
                }
              }
            }
          }
        }
      }
      else /* if detected lamp count is not equal to currently decided lamp count */
      {
        ClearWorkingLampChangeCounter(bOutput);
        SetNoOfLastDetectedWorkingLamps(bOutput, bWorkingLamps);
        IncreaseWorkingLampChangeCounter(bOutput);
      }
    }
    else
    {
      ClearWorkingLampChangeCounter(bOutput);
      SetNoOfLastDetectedWorkingLamps(bOutput, bWorkingLamps);
    }

    return GetSONoOfWorkingLamps(bOutput);
  }
  else
  {
    return 0;
  }
} /* DetermineSOLampsState */

/* Determine conflict state: determines if there is a conflict according to displayed signals */
static void DetermineConflictState(uint8_t bSGNo1, uint8_t bSGNo2)
{
  if (ConflictingSG(bSGNo1,
                    bSGNo2))
  {
    if ((baDisplayedVoltages[bSGNo1] & SIGNAL_OUTPUT_TYPE_YELLOW)
        && (baDisplayedVoltages[bSGNo2] & SIGNAL_OUTPUT_TYPE_YELLOW))
    {
      baaYellowYellowConflictCounter[bSGNo1][bSGNo2] += 1;
      if (baaYellowYellowConflictCounter[bSGNo1][bSGNo2] >= MIN_CONFLICT)
      {
        baaYellowYellowConflictCounter[bSGNo1][bSGNo2] = 0;

        if (GetYYEM())
        {
          SetEmergencyMethodSet(GetYYEM(), GetSGOwner(bSGNo1));
        }

        if ((GetYYConflictState(bSGNo1,
                                bSGNo2) == FALSE) || (GetYYConflictState(bSGNo2,
                                                                         bSGNo1)
                                                      == FALSE) )
        {
          LogRequest(EVENT_YELLOW_YELLOW_CONFLICT, (bSGNo1 + 1), (bSGNo2 + 1),
                     0);
        }

        SetYYConflictState(bSGNo1, bSGNo2, TRUE);
        SetYYConflictState(bSGNo2, bSGNo1, TRUE);
      }
    }

    if (((baDisplayedVoltages[bSGNo1] & SIGNAL_OUTPUT_TYPE_YELLOW)
         && (baDisplayedVoltages[bSGNo2] & SIGNAL_OUTPUT_TYPE_GREEN))
        || ((baDisplayedVoltages[bSGNo1] & SIGNAL_OUTPUT_TYPE_GREEN)
            && (baDisplayedVoltages[bSGNo2] & SIGNAL_OUTPUT_TYPE_YELLOW)))
    {
      baaYellowGreenConflictCounter[bSGNo1][bSGNo2] += 1;
      if (baaYellowGreenConflictCounter[bSGNo1][bSGNo2] >= MIN_CONFLICT)
      {
        baaYellowGreenConflictCounter[bSGNo1][bSGNo2] = 0;
        if (GetYGEM())
        {
          SetEmergencyMethodSet(GetYGEM(), GetSGOwner(bSGNo1));
        }

        if ((GetYGConflictState(bSGNo1,
                                bSGNo2) == FALSE) || (GetYGConflictState(bSGNo2,
                                                                         bSGNo1)
                                                      == FALSE) )
        {
          LogRequest(EVENT_YELLOW_GREEN_CONFLICT, (bSGNo1 + 1), (bSGNo2 + 1),
                     0);
        }

        SetYGConflictState(bSGNo1, bSGNo2, TRUE);
        SetYGConflictState(bSGNo2, bSGNo1, TRUE);
      }
    }

    if ((baDisplayedVoltages[bSGNo1] & SIGNAL_OUTPUT_TYPE_GREEN)
        && (baDisplayedVoltages[bSGNo2] & SIGNAL_OUTPUT_TYPE_GREEN))
    {
      baaGreenGreenConflictCounter[bSGNo1][bSGNo2] += 1;
      if (baaGreenGreenConflictCounter[bSGNo1][bSGNo2] >= MIN_CONFLICT)
      {
        baaGreenGreenConflictCounter[bSGNo1][bSGNo2] = 0;
        if (GetGGEM())
        {
          SetEmergencyMethodSet(GetGGEM(), GetSGOwner(bSGNo1));
        }

        if ((GetGGConflictState(bSGNo1,
                                bSGNo2) == FALSE) || (GetGGConflictState(bSGNo2,
                                                                         bSGNo1)
                                                      == FALSE) )
        {
          LogRequest(EVENT_GREEN_GREEN_CONFLICT, (bSGNo1 + 1), (bSGNo2 + 1), 0);
        }

        SetGGConflictState(bSGNo1, bSGNo2, TRUE);
        SetGGConflictState(bSGNo2, bSGNo1, TRUE);
      }
    }
  }
} /* DetermineConflictState */

void SignalCheckRuntimeInit(void)
{
  memset(baaDisplayedVSAssignedVoltages, 0xff,
         sizeof(baaDisplayedVSAssignedVoltages));                                       /* assign 0xff to displayed signals so that if DARK is observed as first signal, it is recorded too */
  memset(baaYellowYellowConflictCounter, 0,
         sizeof(baaYellowYellowConflictCounter));
  memset(baaYellowGreenConflictCounter, 0,
         sizeof(baaYellowGreenConflictCounter));
  memset(baaGreenGreenConflictCounter, 0, sizeof(baaGreenGreenConflictCounter));
  memset(baDisplayedVoltages, 0, sizeof(baDisplayedVoltages));
  memset(baPreviousVoltages, 0, sizeof(baPreviousVoltages));
  memset(baInvalidVoltages, 0, sizeof(baInvalidVoltages));
  memset(baFirstInvalidSignalSequences, 0,
         sizeof(baFirstInvalidSignalSequences));
  memset(baSecondInvalidSignalSequences, 0,
         sizeof(baSecondInvalidSignalSequences));
}

void SignalCheckTaskFunc(void const *argument)
{
  UNUSED(argument);

  SignalCheckRuntimeInit();
  tpSNewMeasurements pSNewMeasurements = NULL;

  while (FOREVER)
  {
    if (osMessageQueueGet(NewMeasurementsQueueHandle,
                          &pSNewMeasurements,
                          NULL,
                          1000) == osOK)
    {
      if (CPCommStateGet())
      {
        uint8_t bSGTotal = GetSGTotal();

        /* if psm measurements are received, control for voltage and frequency limits */
        if (SOCatchIsCardActive(SIG_DEV_PSM_0)
            || SOCatchIsCardActive(SIG_DEV_PSM_1))
        {
          /* control frequence bounds */
          if (GetFrequency() < FREQUENCY_VALUE_LOWER_BOUND)
          {
            InitEventCounters(0, EC_FN | EC_FUB);

            if (IncrementEventCounter(0, EC_FLB))
            {
              if (GetFrequencyState() != EVENT_FREQUENCY_VALUE_LOWER_BOUND)
              {
                uint8_t bSetNo = 0;

                if (GetVoltageLimitsEM())
                {
                  for (bSetNo = 0; bSetNo < SIGNAL_SETS_MAX; bSetNo++)
                  {
                    SetEmergencyMethodSet(GetVoltageLimitsEM(), bSetNo);
                  }
                }

                SetFrequencyState(EVENT_FREQUENCY_VALUE_LOWER_BOUND);

                LogRequest(EVENT_FREQUENCY_VALUE_LOWER_BOUND,
                           0,
                           GetFrequency(),
                           FREQUENCY_VALUE_LOWER_BOUND);
              }
            }
          }
          else
          {
            InitEventCounters(0, EC_FLB);

            if (GetFrequency() > FREQUENCY_VALUE_UPPER_BOUND)
            {
              InitEventCounters(0, EC_FN);

              if (IncrementEventCounter(0, EC_FUB))
              {
                if (GetFrequencyState() != EVENT_FREQUENCY_VALUE_UPPER_BOUND)
                {
                  uint8_t bSetNo = 0;

                  if (GetVoltageLimitsEM())
                  {
                    for (bSetNo = 0; bSetNo < SIGNAL_SETS_MAX; bSetNo++)
                    {
                      SetEmergencyMethodSet(GetVoltageLimitsEM(), bSetNo);
                    }
                  }

                  SetFrequencyState(EVENT_FREQUENCY_VALUE_UPPER_BOUND);

                  LogRequest(EVENT_FREQUENCY_VALUE_UPPER_BOUND,
                             0,
                             GetFrequency(),
                             FREQUENCY_VALUE_UPPER_BOUND);
                }
              }
            }
            else
            {
              InitEventCounters(0, EC_FUB);

              if (IncrementEventCounter(0, EC_FN))
              {
                uint8_t fSwitchToNormalState = FALSE; /* this is switching to normal frequency */

                /* decide if frequency has normal value */
                switch (GetFrequencyState())
                {
                    case EVENT_NONE:
                    {
                      fSwitchToNormalState = TRUE;
                      break;
                    }

                    case EVENT_FREQUENCY_VALUE_LOWER_BOUND:
                    {
                      if (GetFrequency()
                          > (FREQUENCY_VALUE_LOWER_BOUND
                             + FREQUENCY_VALUE_HYSTERESIS))
                      {
                        fSwitchToNormalState = TRUE;
                      }

                      break;
                    }

                    case EVENT_FREQUENCY_VALUE_UPPER_BOUND:
                    {
                      if (GetFrequency()
                          < (FREQUENCY_VALUE_UPPER_BOUND
                             - FREQUENCY_VALUE_HYSTERESIS))
                      {
                        fSwitchToNormalState = TRUE;
                      }

                      break;
                    }

                    case EVENT_FREQUENCY_VALUE_NORMAL:
                    {
                      fSwitchToNormalState = FALSE;
                      break;
                    }

                    default:
                    {
                      fSwitchToNormalState = FALSE;
                      break;
                    }
                } /* switch */

                if (fSwitchToNormalState)
                {
                  SetFrequencyState(EVENT_FREQUENCY_VALUE_NORMAL);

                  LogRequest(EVENT_FREQUENCY_VALUE_NORMAL, 0, GetFrequency(),
                             0);
                }
                else
                {
                  InitEventCounters(0, EC_FN);
                }
              }
            }
          }

          /* control voltage bounds */
          if (GetVoltage() < VOLTAGE_VALUE_LOWER_BOUND)
          {
            InitEventCounters(0, EC_VN | EC_VUB);

            if (IncrementEventCounter(0, EC_VLB))
            {
              if (GetVoltageState() != EVENT_VOLTAGE_VALUE_LOWER_BOUND)
              {
                uint8_t bSetNo = 0;

                if (GetVoltageLimitsEM())
                {
                  for (bSetNo = 0; bSetNo < SIGNAL_SETS_MAX; bSetNo++)
                  {
                    SetEmergencyMethodSet(GetVoltageLimitsEM(), bSetNo);
                  }
                }

                SetVoltageState(EVENT_VOLTAGE_VALUE_LOWER_BOUND);

                LogRequest(EVENT_VOLTAGE_VALUE_LOWER_BOUND,
                           0,
                           GetVoltage() * 0.73029,
                           VOLTAGE_VALUE_LOWER_BOUND * 0.73029);                                                             /* For Display Network Voltage in Real AC Value. */
              }
            }
          }
          else
          {
            InitEventCounters(0, EC_VLB);

            if (GetVoltage() > VOLTAGE_VALUE_UPPER_BOUND)
            {
              InitEventCounters(0, EC_VN);

              if (IncrementEventCounter(0, EC_VUB))
              {
                if (GetVoltageState() != EVENT_VOLTAGE_VALUE_UPPER_BOUND)
                {
                  uint8_t bSetNo = 0;

                  if (GetVoltageLimitsEM())
                  {
                    for (bSetNo = 0; bSetNo < SIGNAL_SETS_MAX; bSetNo++)
                    {
                      SetEmergencyMethodSet(GetVoltageLimitsEM(), bSetNo);
                    }
                  }

                  SetVoltageState(EVENT_VOLTAGE_VALUE_UPPER_BOUND);

                  LogRequest(EVENT_VOLTAGE_VALUE_UPPER_BOUND,
                             0,
                             GetVoltage() * 0.73029,
                             VOLTAGE_VALUE_UPPER_BOUND * 0.73029);                                                            /* For Display Network Voltage in Real AC Value. */
                }
              }
            }
            else
            {
              InitEventCounters(0, EC_VUB);

              if (IncrementEventCounter(0, EC_VN))
              {
                uint8_t fSwitchToNormalVoltageState = FALSE;

                /* decide if voltage has normal value */
                switch (GetVoltageState())
                {
                    case EVENT_NONE:
                    {
                      fSwitchToNormalVoltageState = TRUE;
                      break;
                    }

                    case EVENT_VOLTAGE_VALUE_LOWER_BOUND:
                    {
                      if (GetVoltage()
                          > (VOLTAGE_VALUE_LOWER_BOUND
                             + VOLTAGE_VALUE_HYSTERESIS))
                      {
                        fSwitchToNormalVoltageState = TRUE;
                      }

                      break;
                    }

                    case EVENT_VOLTAGE_VALUE_UPPER_BOUND:
                    {
                      if (GetVoltage()
                          < (VOLTAGE_VALUE_UPPER_BOUND
                             - VOLTAGE_VALUE_HYSTERESIS))
                      {
                        fSwitchToNormalVoltageState = TRUE;
                      }

                      break;
                    }

                    case EVENT_VOLTAGE_VALUE_NORMAL:
                    {
                      fSwitchToNormalVoltageState = FALSE;
                      break;
                    }

                    default:
                    {
                      fSwitchToNormalVoltageState = FALSE;
                      break;
                    }
                } /* switch */

                if (fSwitchToNormalVoltageState)
                {
                  SetVoltageState(EVENT_VOLTAGE_VALUE_NORMAL);

                  LogRequest(EVENT_VOLTAGE_VALUE_NORMAL,
                             0,
                             GetVoltage() * 0.73029,
                             0);                                                       /* For Display Network Voltage in Real AC Value. */
                }
                else
                {
                  InitEventCounters(0, EC_VN);
                }
              }
            }
          }
        }

        /* determine if lamps at outputs are on or off, according to lamp on/off criteria */
        uint8_t bSGNo;

        for (bSGNo = 0; bSGNo < bSGTotal; bSGNo++)
        {
          uint8_t bSONo = GetSGFirstOutput(bSGNo);

          while (bSONo)
          {
            bSONo--;
            if (SOCatchIsCardActive((bSONo / SIGNAL_OUTPUTS_PER_SSM)))
            {
              uint16_t sCurrentMeasured, sPrevCurrent;
              uint8_t bCurrentGroupNo;
              uint8_t bOutput2, bOutput3;
              uint8_t bOutputInGruoup = (bSONo
                                         % SIGNAL_OUTPUTS_PER_CURRENT_GROUP);
              uint8_t fSOExpectedVoltage, fSO2ExpectedVoltage,
                      fSO3ExpectedVoltage;
              uint8_t fSOMeasuredVoltage, fSO2MeasuredVoltage,
                      fSO3MeasuredVoltage;

              if (bOutputInGruoup == 0)
              {
                bOutput2 = bSONo + 1;
                bOutput3 = bSONo + 2;
              }
              else if (bOutputInGruoup == 1)
              {
                bOutput2 = bSONo - 1;
                bOutput3 = bSONo + 1;
              }
              else
              {
                bOutput2 = bSONo - 1;
                bOutput3 = bSONo - 2;
              }

              /* voltages */
              fSOExpectedVoltage =
                pSNewMeasurements->SaSORuntimeFlags[bSONo].fSwitchPrevious;
              fSOMeasuredVoltage =
                pSNewMeasurements->SaSORuntimeFlags[bSONo].fVoltage;

              fSO2ExpectedVoltage =
                pSNewMeasurements->SaSORuntimeFlags[bOutput2].fSwitchPrevious;
              fSO2MeasuredVoltage =
                pSNewMeasurements->SaSORuntimeFlags[bOutput2].fVoltage;

              fSO3ExpectedVoltage =
                pSNewMeasurements->SaSORuntimeFlags[bOutput3].fSwitchPrevious;
              fSO3MeasuredVoltage =
                pSNewMeasurements->SaSORuntimeFlags[bOutput3].fVoltage;

              /* currents */
              bCurrentGroupNo = (bSONo / SIGNAL_OUTPUTS_PER_CURRENT_GROUP);
              sCurrentMeasured =
                pSNewMeasurements->SaSGCurrents[bCurrentGroupNo].sNow;
              sPrevCurrent =
                pSNewMeasurements->SaSGCurrents[bCurrentGroupNo].sPrev;

              if (fSOExpectedVoltage == FALSE) /* Switch Off */
              {
                InitEventCounters(bSONo,
                                  EC_SOC | EC_SOS_NFF | EC_SOS_NFN | EC_SOS_NN
                                  | EC_SOS_NNF_FIR | EC_SOS_NNF_SEC
                                  | EC_VSF_SEC);

                /* switch is off - the lamps at this output are desired to be off */
                /* in order for the lamps to be accepted as off, there should be no voltage detected */
                /* the current may also be checked to determine the cause of the malfunction */
                /* but there must be no other active output in the current measurement group */
                if (fSOMeasuredVoltage == FALSE) /* Switch Off - No Voltage */
                {
                  InitEventCounters(bSONo,
                                    EC_LDE_FIR | EC_LDE_SEC | EC_SSC_SEC
                                    | EC_SOS_FN);
                  /* Commented on Mr. Suha's request for emergent Kastamonu version upgrade */
                  /* SORuntimeFailDrivenExternallyFirstSet(bSONo, FALSE); */
                  /* SORuntimeFailDrivenExternallySecondSet(bSONo, FALSE); */

                  /* there is no voltage at the output --> the lamps at this output are accepted as off */
                  if (IncrementEventCounter(bSONo, EC_SOS_FF))
                  {
                    SetSOOnOffState(bSONo, FALSE);
                  }

                  /* assume that if there is current flowing and at least one of the other outputs is applied to be on, it is more possible */
                  /* that the current flowing belongs to it(them), leave this output in off state */
                  if (sCurrentMeasured > SO_MIN_POWER)  /* comparison must be '>' since both of them may be 0 */
                  {
                    if ((fSO2ExpectedVoltage == FALSE)
                        && (fSO3ExpectedVoltage == FALSE))
                    {
                      /* at here, all outputs are applied to be off, but there is current flowing, this situation may resulted from */
                      /* any of the outputs so if it is green or yellow output, we must assume that this output is on, if not-red output-, */
                      /* we should leave output in off state with assumption that the error belongs to green/yellow outputs */
                      if (GetSOType(bSONo) != SIGNAL_OUTPUT_TYPE_RED)
                      {
                        InitEventCounters(bSONo, EC_SOS_FF);  /* discard previous(above) state assignment */
                        if (IncrementEventCounter(bSONo, EC_SOS_FFN))
                        {
                          SetSOOnOffState(bSONo, TRUE);
                        }
                      }

                      /* a log on an assumption: there is current flowing but no voltage is detected, log the following */
                      if (SORuntimeFailShortCircuitFirstGet(bSONo) == FALSE)
                      {
                        if (IncrementEventCounter(bSONo, EC_SSC_FIR))
                        {
                          LogRequest(EVENT_SO_SWITCH_SHORT_CIRCUIT,
                                     (bSONo + 1),
                                     sCurrentMeasured,
                                     1);

                          SORuntimeFailShortCircuitFirstSet(bSONo, TRUE);
                        }
                      }

                      if (SORuntimeVoltageSensorFailureFirstGet(bSONo) == FALSE)
                      {
                        if (IncrementEventCounter(bSONo, EC_VSF_FIR))
                        {
                          LogRequest(EVENT_SO_VOLTAGE_SENSOR_FAILURE,
                                     (bSONo + 1),
                                     1,
                                     0);

                          SORuntimeVoltageSensorFailureFirstSet(bSONo, TRUE);
                        }
                      }
                    }
                    else
                    {
                      InitEventCounters(bSONo,
                                        EC_SSC_FIR | EC_VSF_FIR | EC_SOS_FFN);
                      SORuntimeFailShortCircuitFirstSet(bSONo, FALSE);
                      SORuntimeVoltageSensorFailureFirstSet(bSONo, FALSE);
                    }
                  }
                  else
                  {
                    InitEventCounters(bSONo,
                                      EC_SSC_FIR | EC_VSF_FIR | EC_SOS_FFN);
                    SORuntimeFailShortCircuitFirstSet(bSONo, FALSE);
                    SORuntimeVoltageSensorFailureFirstSet(bSONo, FALSE);
                  }
                }
                else /* Switch Off - Voltage detected */
                {
                  InitEventCounters(bSONo,
                                    EC_SSC_FIR | EC_VSF_FIR | EC_SOS_FF
                                    | EC_SOS_FFN);

                  /* we set the output to off but there is voltage, any type of output may be accepted as on */
                  if (IncrementEventCounter(bSONo, EC_SOS_FN))
                  {
                    SetSOOnOffState(bSONo, TRUE);
                  }

                  /* if there is no other active output at this current measurement group and there is current flowing, */
                  /* the switch circuit is broken (always short-circuit) */
                  /* the examined output has already off state, if other two SOs in the group is off, the following code should be run */
                  if (sCurrentMeasured < SO_MIN_POWER)
                  {
                    InitEventCounters(bSONo, EC_SSC_SEC | EC_LDE_SEC);

                    /* we wanted all outputs to be off but there is voltage on this output */
                    /* certain: lamps are on but are not supllied from the controller, probably a short circuit to mains in the field */
                    if (SORuntimeFailDrivenExternallyFirstGet(bSONo) == FALSE)
                    {
                      uint8_t bExpectedVoltages = fSOExpectedVoltage
                                                  | (fSO2ExpectedVoltage <<
                                                     1)
                                                  | (fSO3ExpectedVoltage << 2);
                      uint8_t bMeasuredVoltages = fSOMeasuredVoltage
                                                  | (fSO2MeasuredVoltage <<
                                                     1)
                                                  | (fSO3MeasuredVoltage << 2);

                      if (IncrementEventCounter(bSONo, EC_LDE_FIR))
                      {
                        LogRequest(EVENT_SO_LAMPS_DRIVEN_EXTERNALLY,
                                   (bSONo + 1),
                                   bExpectedVoltages,
                                   bMeasuredVoltages);

                        SORuntimeFailDrivenExternallyFirstSet(bSONo, TRUE);
                      }
                    }
                  }
                  else
                  {
                    InitEventCounters(bSONo, EC_LDE_FIR);
                    SORuntimeFailDrivenExternallyFirstSet(bSONo, FALSE);

                    /* assume that two voltage sensor won't be broken at the same time */
                    if ((fSO2ExpectedVoltage == FALSE)
                        && (fSO2MeasuredVoltage == FALSE)
                        && (fSO3ExpectedVoltage == FALSE)
                        && (fSO3MeasuredVoltage == FALSE))
                    {
                      InitEventCounters(bSONo, EC_LDE_SEC);
                      SORuntimeFailDrivenExternallySecondSet(bSONo, FALSE);

                      /* certain: we hope there will be no current due to other two outputs so assume the measured current flowing belongs */
                      /* to this output, switch short-circuit */
                      if (SORuntimeFailShortCircuitSecondGet(bSONo) == FALSE)
                      {
                        if (IncrementEventCounter(bSONo, EC_SSC_SEC))
                        {
                          LogRequest(EVENT_SO_SWITCH_SHORT_CIRCUIT,
                                     (bSONo + 1),
                                     sCurrentMeasured,
                                     2);

                          SORuntimeFailShortCircuitSecondSet(bSONo, TRUE);
                        }
                      }
                    }
                    else if ( ((fSO2ExpectedVoltage) && (fSO2MeasuredVoltage))
                              || ((fSO3ExpectedVoltage)
                                  && (fSO3MeasuredVoltage)))
                    {
                      InitEventCounters(bSONo, EC_SSC_SEC);
                      SORuntimeFailShortCircuitSecondSet(bSONo, FALSE);

                      /* certain: we hope other two outputs are set to on in this group so assume the measured current flowing belongs to them */
                      /* the measured voltage may be a short circuit to mains in the field */
                      if (SORuntimeFailDrivenExternallySecondGet(bSONo)
                          == FALSE)
                      {
                        if (IncrementEventCounter(bSONo, EC_LDE_SEC))
                        {
                          LogRequest(EVENT_SO_LAMPS_DRIVEN_EXTERNALLY,
                                     (bSONo + 1), 0, 0);

                          SORuntimeFailDrivenExternallySecondSet(bSONo, TRUE);
                        }
                      }
                    }
                    else
                    {
                      InitEventCounters(bSONo, EC_SSC_SEC | EC_LDE_SEC);
                    }
                  }
                }
              }
              else /* Switch On */
              {
                InitEventCounters(bSONo,
                                  EC_LDE_FIR | EC_LDE_SEC | EC_SSC_FIR
                                  | EC_SSC_SEC | EC_VSF_FIR | EC_SOS_FF
                                  | EC_SOS_FFN | EC_SOS_FN);

                /* switch is on - the lamps at this output are desired to be on */
                /* in order for the lamps to be accepted as on, there should be current enough to satisfy sPower consumption */
                /* however voltage should also be checked to determine the source of malfunction if there is any */
                if (fSOMeasuredVoltage == FALSE) /* Switch On - Voltage Off */
                {
                  InitEventCounters(bSONo,
                                    EC_SOS_NN | EC_SOS_NNF_FIR
                                    | EC_SOS_NNF_SEC);

                  /* we set the output to on but there is no voltage, if it is a red output, accept it is off */
                  /* if it is yellow/green output, assume that there may be a problem in voltage sensor, accept they are on */

                  if (GetSOType(bSONo) == SIGNAL_OUTPUT_TYPE_RED)
                  {
                    InitEventCounters(bSONo, EC_SOS_NFN);

                    if (IncrementEventCounter(bSONo, EC_SOS_NFF))
                    {
                      SetSOOnOffState(bSONo, FALSE);
                    }
                  }
                  else
                  {
                    InitEventCounters(bSONo, EC_SOS_NFF);

                    if (IncrementEventCounter(bSONo, EC_SOS_NFN))
                    {
                      SetSOOnOffState(bSONo, TRUE);
                    }
                  }

                  /* no voltage detected while the switch is on */
                  /* if there is also no current, then switch circuit is broken (always open-circuit) */
                  /* if there is current, then voltage detector circuit is broken */
                  if (sCurrentMeasured < SO_MIN_POWER)/* if there is no current */
                  {
                    InitEventCounters(bSONo, EC_VSF_SEC);
                    SORuntimeVoltageSensorFailureSecondSet(bSONo, FALSE);

                    /* there is no voltage because there is no current flowing due to switch open circuit */
                    if (SORuntimeFailOpenCircuitGet(bSONo) == FALSE)
                    {
                      if (IncrementEventCounter(bSONo, EC_SOC))
                      {
                        LogRequest(EVENT_SO_SWITCH_OPEN_CIRCUIT,
                                   (bSONo + 1),
                                   0,
                                   0);                                                  /* switch open-circuit */

                        SORuntimeFailOpenCircuitSet(bSONo, TRUE);
                      }
                    }
                  }
                  else /* if there is current */
                  {
                    InitEventCounters(bSONo, EC_SOC);
                    SORuntimeFailOpenCircuitSet(bSONo, FALSE);

                    if (SORuntimeVoltageSensorFailureSecondGet(bSONo) == FALSE)
                    {
                      if (IncrementEventCounter(bSONo, EC_VSF_SEC))
                      {
                        LogRequest(EVENT_SO_VOLTAGE_SENSOR_FAILURE,
                                   (bSONo + 1),
                                   2,
                                   sCurrentMeasured);                                                   /* voltage sensor circuit is broken */

                        SORuntimeVoltageSensorFailureSecondSet(bSONo, TRUE);
                      }
                    }

                    /* DetermineSOLampsState(bSONo, sCurrentMeasured, sPrevCurrent);  // check current further for lamp failures */
                  }
                }
                else /* Switch On - Voltage On */
                {
                  InitEventCounters(bSONo,
                                    EC_SOC | EC_VSF_SEC | EC_SOS_NFF
                                    | EC_SOS_NFN);
                  /* Commented on Mr. Suha's request for emergent Kastamonu version upgrade */
                  /* SORuntimeFailOpenCircuitSet(bSONo, FALSE); */
                  /* SORuntimeVoltageSensorFailureSecondSet(bSONo, FALSE); */

                  if (IncrementEventCounter(bSONo, EC_SOS_NN))
                  {
                    SetSOOnOffState(bSONo, TRUE);
                  }

                  /* there is also voltage, check currents to determine if lamps are on or off because of lamp failures */
                  /* voltage is detected, switch and voltage detector are ok */
                  /* check for lamp failures unless power recording is going on */
                  if ((fSO2ExpectedVoltage == FALSE)
                      && (fSO2MeasuredVoltage == FALSE)
                      && (fSO3ExpectedVoltage == FALSE)
                      && (fSO3MeasuredVoltage == FALSE))
                  {
                    /* assume the current flowing belongs to this output and record current for this output */
                    /* or apply this current value to broken-lamps-evaluation-routine */

                    if ((RecordingPower(bSONo) == FALSE)
                        && !LampDimmingStateChangedGet())                                 /* if power record is not done before */
                    {
                      /* accept lamps as on if currently recording */
                      if ((GetStartRecord(bSONo) == FALSE)
                          && (sCurrentMeasured < GetSOPower(bSONo)))                                    /* get sample if record didn't start */
                      {
                        SetOnlySOPower(bSONo, sCurrentMeasured);
                      }
                      else
                      {
                        SetStartRecord(bSONo, TRUE); /* set record status as started */
                        /* first increase */
                        if (GetStartMean(bSONo) == FALSE) /* check if this is the first time to get samples to calculate mean */
                        {
                          SetOnlySOPower(bSONo, 0);           /* set to 0 for the following additions */
                          SetStartMean(bSONo, TRUE);
                        }
                        else
                        {
                          uint16_t sPower = GetSOPower(bSONo);

                          if (SORuntimePowerRecordSampleGet(bSONo)
                              >= MIN_CURRENT_RECORD_SAMPLES_BEFORE_START)
                          {
                            sPower += sCurrentMeasured;
                          }

                          SetOnlySOPower(bSONo, sPower);
                          /* increase sample count, stops recording if reached max sample count, record mean powers and voltages */
                          IncreasePowerRecordSamples(bSONo);
                        }
                      }
                    }
                    else /* if power record is done */
                    {
                      DetermineSOLampsState(bSONo,
                                            sCurrentMeasured,
                                            sPrevCurrent);
                    }
                  }
                  else
                  {
                    /* check current for the sum of safe powers */
                    uint16_t sTotalSafePower;

                    sTotalSafePower = GetSOMinPower(bSONo);

                    if (fSO2ExpectedVoltage)
                    {
                      sTotalSafePower += GetSOMinPower(bOutput2);
                    }

                    if (fSO3ExpectedVoltage)
                    {
                      sTotalSafePower += GetSOMinPower(bOutput3);
                    }

                    if (sCurrentMeasured < sTotalSafePower)
                    {
                      /* there is current flowing, we do not know if this output has the measured current but it is possible */
                      /* we can set output to off if it is red, yellow/green outputs must still be set to on */

                      if (GetSOType(bSONo) == SIGNAL_OUTPUT_TYPE_RED)
                      {
                        InitEventCounters(bSONo, EC_SOS_NN);
                        if (IncrementEventCounter(bSONo, EC_SOS_NNF_SEC))
                        {
                          SetSOOnOffState(bSONo, FALSE);    /* none of the active outputs are consuming enough current, lamps at this one are surely off */
                        }
                      }
                    }
                    else
                    {
                      InitEventCounters(bSONo, EC_SOS_NNF_SEC);
                    }
                  }
                }
              }
            }

            bSONo = GetSONextOutput(bSONo);
          }
        }

        /* check signals according to determined lamp states */
        for (bSGNo = 0; bSGNo < bSGTotal; bSGNo++)
        {
          uint8_t bSetNo = GetSGOwner(bSGNo);
          uint8_t fSafeControl = TRUE;          /* store FALSE, any number of SO state certainity is not determined */
          uint8_t bAssignedVoltages = 0;

          baDisplayedVoltages[bSGNo] = 0;          /* determine the displayed signal at the signal group */
          uint8_t bSONo = GetSGFirstOutput(bSGNo);

          while (bSONo)
          {
            bSONo--;

            if (SOStateDetermined(bSONo))
            {
              uint8_t bSOType = GetSOType(bSONo);

              if (pSNewMeasurements->SaSORuntimeFlags[bSONo].fSwitchPrevious)
              {
                bAssignedVoltages |= bSOType;
              }

              if (GetSOOnOffState(bSONo))
              {
                baDisplayedVoltages[bSGNo] |= bSOType;  /* since signal output types are bits in signal values this works fine. */
              }

              bSONo = GetSONextOutput(bSONo);
            }
            else
            {
              fSafeControl = FALSE; /* if state of at least one output of group is not determined, it is not safe determining signal */
              break;
            }
          }

          if (fSafeControl)
          {
            /* bSignalDisplayed now contains the value of the displayed signal at the signal group */
            /* append to log if displayed signal is different than the last displayed for the assigned signal */
            if (baaDisplayedVSAssignedVoltages[bSGNo][bAssignedVoltages]
                != baDisplayedVoltages[bSGNo])
            {
              LogRequest(EVENT_SIGNAL_AT_SG,
                         (bSGNo + 1),
                         bAssignedVoltages,
                         baDisplayedVoltages[bSGNo]);
            }

            /* update displayed signal record for this assignment */
            baaDisplayedVSAssignedVoltages[bSGNo][bAssignedVoltages] =
              baDisplayedVoltages[bSGNo];

            if (baDisplayedVoltages[bSGNo] != baPreviousVoltages[bSGNo])  /* if displayed signal has started just now */
            {
              /* check if displayed signal is a valid signal or not */
              if (VoltagesValid(bSGNo, baDisplayedVoltages[bSGNo]) == FALSE)
              {
                if (GetInvalidSignalEM())
                {
                  SetEmergencyMethodSet(GetInvalidSignalEM(), bSetNo);
                }

                if (baDisplayedVoltages[bSGNo] != baInvalidVoltages[bSGNo]) /* prevent multiple logs for the same invalid signal. */
                {
                  LogRequest(EVENT_INVALID_SIGNAL,
                             (bSGNo + 1),
                             baDisplayedVoltages[bSGNo],
                             0);
                }

                baInvalidVoltages[bSGNo] = baDisplayedVoltages[bSGNo];
              }

              /* check displayed signal against previous signal to see if it is a proper follower or not */
              if (VoltagesCanFollowVoltages(baPreviousVoltages[bSGNo],
                                            baDisplayedVoltages[bSGNo])
                  == FALSE)
              {
                if (GetInvalidSignalSequenceEM())
                {
                  SetEmergencyMethodSet(GetInvalidSignalSequenceEM(), bSetNo);
                }

                if ((baFirstInvalidSignalSequences[bSGNo]
                     != baPreviousVoltages[bSGNo])
                    && (baSecondInvalidSignalSequences[bSGNo]
                        != baDisplayedVoltages[bSGNo]))
                {
                  LogRequest(EVENT_INVALID_SIGNAL_SEQUENCE,
                             (bSGNo + 1),
                             baPreviousVoltages[bSGNo],
                             baDisplayedVoltages[bSGNo]);
                }

                baFirstInvalidSignalSequences[bSGNo] =
                  baPreviousVoltages[bSGNo];
                baSecondInvalidSignalSequences[bSGNo] =
                  baDisplayedVoltages[bSGNo];
              }
            }

            /* update previous signal */
            baPreviousVoltages[bSGNo] = baDisplayedVoltages[bSGNo];
          }

          IWDGRefresh();
        }

        /* all displayed signals are determined, check for yellow-yellow, yellow-green and green-green conflicts */
        for (bSGNo = 0; bSGNo < bSGTotal; bSGNo++)
        {
          uint8_t bSGNo2;

          for (bSGNo2 = (bSGNo + 1); bSGNo2 < bSGTotal; bSGNo2++)
          {
            DetermineConflictState(bSGNo, bSGNo2);
          }

          IWDGRefresh();
        }
      }

      osMemoryPoolFree(NewMeasurementsMemPoolHandle, pSNewMeasurements);
    }

    MaintenanceSignalTask(EVENT_FLAGS_SIGNAL_CHECK_TASK_ACTIVE);
  }
} /* SignalCheckTaskFunc */
