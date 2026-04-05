/*
 * App/Domain/Intersection/Conflict.c
 *
 * Green-green and yellow-green Conflict detection.
 * Ported from legacy Tasks/Src/Program.c (Conflict checking loop).
 */
#include "Domain/Intersection/Conflict.h"
#include <string.h>

/* A SG is "active" (non-red) when in one of these states */
static bool is_active(SignalGroupState_t s)
{
  return s == SG_STATE_OPEN
         || s == SG_STATE_OPENING
         || s == SG_STATE_GREEN_FLASH
         || s == SG_STATE_CLOSING;
}

static bool is_green(SignalGroupState_t s)
{
  return s == SG_STATE_OPEN || s == SG_STATE_OPENING
         || s == SG_STATE_GREEN_FLASH;
}

static bool is_yellow(SignalGroupState_t s)
{
  return s == SG_STATE_CLOSING;
}

ConflictType_t Conflict_Check(const SignalGroupConfig_t  *sgConfigs,
                              const SignalGroupRuntime_t *sgRuntimes,
                              uint8_t sgCount,
                              ISnmpNotifierPort_t        *snmpNotifier)
{
  for (uint8_t i = 0U; i < sgCount; i++)
  {
    if (!is_active(sgRuntimes[i].state))
    {
      continue;
    }

    for (uint8_t j = (uint8_t) (i + 1U); j < sgCount; j++)
    {
      if (!sgConfigs[i].Conflicts[j].hasConflict)
      {
        continue;
      }

      if (!is_active(sgRuntimes[j].state))
      {
        continue;
      }

      /* Both active and Conflict configured — determine type */
      ConflictType_t type;

      if (is_green(sgRuntimes[i].state) && is_green(sgRuntimes[j].state))
      {
        type = CONFLICT_GREEN_GREEN;
      }
      else if (is_yellow(sgRuntimes[i].state) || is_yellow(sgRuntimes[j].state))
      {
        type = CONFLICT_YELLOW_GREEN;
      }
      else
      {
        type = CONFLICT_GREEN_GREEN;         /* Fallback — treat as worst case */
      }

      SnmpNotifier_SendTrap(snmpNotifier, SNMP_TRAP_CONFLICT_FAULT,
                            ((uint32_t) i << 8U) | (uint32_t) j);

      return type;
    }
  }

  return CONFLICT_NONE;
} /* Conflict_Check */

uint8_t Conflict_GetClearanceSeconds(const SignalGroupConfig_t *sgA,
                                     uint8_t sgBIndex)
{
  if (sgBIndex >= SIGNAL_GROUPS_MAX)
  {
    return 0U;
  }

  if (!sgA->Conflicts[sgBIndex].hasConflict)
  {
    return 0U;
  }

  return sgA->Conflicts[sgBIndex].redClearanceInterval;
}

bool Conflict_Exists(const SignalGroupConfig_t *sgA, uint8_t sgBIndex)
{
  if (sgBIndex >= SIGNAL_GROUPS_MAX)
  {
    return false;
  }

  return sgA->Conflicts[sgBIndex].hasConflict;
}
