/*
 * App/Domain/Intersection/SignalGroup.c
 *
 * Signal group state machine implementation.
 * Ported from legacy Tasks/Src/Program.c (SG state handling).
 *
 * State transitions:
 *   CLOSED → OPENING → OPEN → GREEN_FLASH → CLOSING → CLOSED
 */
#include "Domain/Intersection/SignalGroup.h"
#include <string.h>

/* Map SG state to a lamp color for the signal output port */
static SignalColor_t state_to_color(SignalGroupState_t state)
{
  switch (state)
  {
      case SG_STATE_OPEN:
      { return SIGNAL_COLOR_GREEN; }

      case SG_STATE_GREEN_FLASH:
      { return SIGNAL_COLOR_FLASH; }

      case SG_STATE_OPENING:
      { return SIGNAL_COLOR_GREEN;                             /* Opening signal */
      }

      case SG_STATE_CLOSING:
      { return SIGNAL_COLOR_YELLOW; }

      case SG_STATE_CLOSED:
      { return SIGNAL_COLOR_RED; }

      case SG_STATE_FLASH:
      { return SIGNAL_COLOR_FLASH; }

      default:
      { return SIGNAL_COLOR_OFF; }
  }
}

void SG_Reset(SignalGroupRuntime_t *rt)
{
  memset(rt, 0, sizeof(*rt));
  rt->state = SG_STATE_CLOSED;
}

void SG_Open(uint8_t sgIdx,
             SignalGroupRuntime_t      *rt,
             const SignalGroupConfig_t *cfg,
             ISignalOutputPort_t       *signalOut)
{
  if (rt->state == SG_STATE_CLOSED)
  {
    if (cfg->openingDuration > 0U)
    {
      rt->state = SG_STATE_OPENING;
      rt->stateElapsedSeconds = 0U;
    }
    else
    {
      rt->state = SG_STATE_OPEN;
      rt->stateElapsedSeconds = 0U;
    }

    SignalOutput_SetLamp(signalOut, cfg->firstOutputIndex,
                         state_to_color(rt->state));
    SignalOutput_Flush(signalOut);
  }

  (void) sgIdx;  /* sgIdx retained for SNMP trap payloads elsewhere */
}

void SG_Close(uint8_t sgIdx,
              SignalGroupRuntime_t      *rt,
              const SignalGroupConfig_t *cfg,
              ISignalOutputPort_t       *signalOut)
{
  if (rt->state == SG_STATE_OPEN)
  {
    if (cfg->pedestrianClearance > 0U)
    {
      rt->state = SG_STATE_GREEN_FLASH;
    }
    else if (cfg->yellowChangeInterval > 0U)
    {
      rt->state = SG_STATE_CLOSING;
    }
    else
    {
      rt->state = SG_STATE_CLOSED;
    }

    rt->stateElapsedSeconds = 0U;
    SignalOutput_SetLamp(signalOut, cfg->firstOutputIndex,
                         state_to_color(rt->state));
    SignalOutput_Flush(signalOut);
  }

  (void) sgIdx;
}

void SG_Tick(uint8_t sgIdx,
             SignalGroupRuntime_t      *rt,
             const SignalGroupConfig_t *cfg,
             ISignalOutputPort_t       *signalOut)
{
  if ((rt->state == SG_STATE_CLOSED) || (rt->state == SG_STATE_OPEN) )
  {
    return;     /* No time-driven transition in these stable states */
  }

  rt->stateElapsedSeconds++;

  switch (rt->state)
  {
      case SG_STATE_OPENING:
      {
        if (rt->stateElapsedSeconds >= cfg->openingDuration)
        {
          rt->state = SG_STATE_OPEN;
          rt->stateElapsedSeconds = 0U;
          SignalOutput_SetLamp(signalOut,
                               cfg->firstOutputIndex,
                               SIGNAL_COLOR_GREEN);
        }

        break;
      }

      case SG_STATE_GREEN_FLASH:
      {
        if (rt->stateElapsedSeconds >= cfg->pedestrianClearance)
        {
          rt->state = SG_STATE_CLOSING;
          rt->stateElapsedSeconds = 0U;
          SignalOutput_SetLamp(signalOut,
                               cfg->firstOutputIndex,
                               SIGNAL_COLOR_YELLOW);
        }

        break;
      }

      case SG_STATE_CLOSING:
      {
        if (rt->stateElapsedSeconds >= cfg->yellowChangeInterval)
        {
          rt->state = SG_STATE_CLOSED;
          rt->stateElapsedSeconds = 0U;
          SignalOutput_SetLamp(signalOut,
                               cfg->firstOutputIndex,
                               SIGNAL_COLOR_RED);
        }

        break;
      }

      default:
      {
        break;
      }
  } /* switch */

  (void) sgIdx;
} /* SG_Tick */

bool SG_IsClosed(const SignalGroupRuntime_t *rt)
{
  return rt->state == SG_STATE_CLOSED;
}

bool SG_IsOpen(const SignalGroupRuntime_t *rt)
{
  return rt->state == SG_STATE_OPEN;
}

void SG_UpdateFaults(uint8_t sgIdx,
                     SignalGroupRuntime_t      *rt,
                     const SignalGroupConfig_t *cfg,
                     bool redBroken,
                     bool yellowBroken,
                     bool greenBroken,
                     ISnmpNotifierPort_t        *snmpNotifier)
{
  bool wasRedCritical = rt->redLampCritical;

  rt->redLampBroken = redBroken;
  rt->yellowLampBroken = yellowBroken;
  rt->greenLampBroken = greenBroken;

  /* Critical = broken count meets or exceeds configuration threshold.
   * The adapter counts broken lamps per SG; we receive the aggregated flag. */
  rt->redLampCritical = redBroken;

  /* Fire SNMP trap only on new critical fault (avoid repeat traps) */
  if (rt->redLampCritical && !wasRedCritical)
  {
    SnmpNotifier_SendTrap(snmpNotifier, SNMP_TRAP_LAMP_FAILURE,
                          (uint32_t) sgIdx);
  }

  (void) cfg;  /* cfg used for threshold in full implementation */
}
