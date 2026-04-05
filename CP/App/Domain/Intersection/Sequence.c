/*
 * App/Domain/Intersection/Sequence.c
 *
 * Pre-timed Sequence execution engine.
 * Ported from legacy Tasks/Src/Program.c (Sequence handling logic).
 */
#include "Domain/Intersection/Sequence.h"
#include <string.h>

/* Extract 4-bit signal index for SG sgIdx from the packed step array */
static uint8_t unpack_signal(const SequenceConfig_t *cfg,
                             uint8_t step, uint8_t sgIdx)
{
  if ((step >= SEQUENCE_STEPS_MAX) || (sgIdx >= SIGNAL_GROUPS_MAX) )
  {
    return 0U;
  }

  uint8_t byte = cfg->stepSignals[step][sgIdx / 2U];
  uint8_t shift = (sgIdx & 1U) ? 4U : 0U;

  return (byte >> shift) & 0x0FU;
}

void Sequence_Reset(SequenceRuntime_t *rt)
{
  memset(rt, 0, sizeof(*rt));
}

void Sequence_ApplyStep(uint8_t step,
                        const SequenceConfig_t    *cfg,
                        const SignalGroupConfig_t *sgConfigs,
                        uint8_t sgCount,
                        ISignalOutputPort_t       *signalOut)
{
  if (step >= cfg->stepCount)
  {
    return;
  }

  for (uint8_t sg = 0U; sg < sgCount; sg++)
  {
    uint8_t sigIdx = unpack_signal(cfg, step, sg);
    /* Simple mapping: 0=red, 1=green, 2=yellow (extend as needed) */
    SignalColor_t color;

    switch (sigIdx)
    {
        case 1U:
        { color = SIGNAL_COLOR_GREEN;  break; }

        case 2U:
        { color = SIGNAL_COLOR_YELLOW; break; }

        case 3U:
        { color = SIGNAL_COLOR_FLASH;  break; }

        case 0U:      /* fall-through */
        default:
        { color = SIGNAL_COLOR_RED;    break; }
    }

    SignalOutput_SetLamp(signalOut, sgConfigs[sg].firstOutputIndex, color);
  }

  SignalOutput_Flush(signalOut);
}

bool Sequence_Tick(uint8_t seqIdx,
                   SequenceRuntime_t          *rt,
                   const SequenceConfig_t     *cfg,
                   const SignalGroupConfig_t  *sgConfigs,
                   uint8_t sgCount,
                   ISignalOutputPort_t        *signalOut)
{
  if (cfg->stepCount == 0U)
  {
    return true;     /* Empty Sequence = immediately done */
  }

  rt->stepElapsedSeconds++;

  if (rt->stepElapsedSeconds >= cfg->stepDurations[rt->currentStep])
  {
    rt->stepElapsedSeconds = 0U;
    rt->currentStep++;

    if (rt->currentStep >= cfg->stepCount)
    {
      rt->currentStep = 0U;
      rt->loopCount++;
      (void) seqIdx;

      return true;       /* One full loop completed */
    }

    /* Apply signal state for the new step */
    Sequence_ApplyStep(rt->currentStep, cfg, sgConfigs, sgCount, signalOut);
  }

  return false;
}
