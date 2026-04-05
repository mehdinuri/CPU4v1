#pragma once

/*
 * Tests/Fixtures/ConflictMatrix_6sg.h
 *
 * 6-signal-group Conflict fixture for Conflict detection tests.
 *
 * Topology (3-phase Intersection):
 *   SG 0: NS main       Conflicting: 2, 3, 4, 5
 *   SG 1: NS left-turn  Conflicting: 2, 3, 4, 5
 *   SG 2: EW main       Conflicting: 0, 1
 *   SG 3: EW left-turn  Conflicting: 0, 1
 *   SG 4: NS pedestrian Conflicting: 0, 1, 2, 3
 *   SG 5: EW pedestrian Conflicting: 0, 1, 2, 3
 *
 * Non-Conflicting pairs (can be green simultaneously):
 *   (0,1), (2,3), (4,5)
 */
#include "Domain/Intersection/Types.h"

#define FIXTURE_6SG_COUNT  6U

static inline void Fixture6SG_Build(
  SignalGroupConfig_t sgCfg[FIXTURE_6SG_COUNT])
{
  uint8_t i, j;

  /* Zero everything first */
  for (i = 0U; i < FIXTURE_6SG_COUNT; i++)
  {
    for (j = 0U; j < SIGNAL_GROUPS_MAX; j++)
    {
      sgCfg[i].Conflicts[j].hasConflict = false;
      sgCfg[i].Conflicts[j].redClearanceInterval = 0U;
    }

    sgCfg[i].type = SG_TYPE_VEHICLE_MAINWAY;
    sgCfg[i].yellowChangeInterval = 3U;
    sgCfg[i].pedestrianClearance = 0U;
    sgCfg[i].firstOutputIndex = (uint8_t) (i * 3U);
    sgCfg[i].criticalRedLampCount = 1U;
    sgCfg[i].openingSignalIdx = 0U;
    sgCfg[i].closingSignalIdx = 0U;
    sgCfg[i].openingDuration = 0U;
    sgCfg[i].pedestrianWalk = 0U;
    sgCfg[i].flashSignalIdx = 0U;
  }

  sgCfg[4].type = SG_TYPE_PEDESTRIAN;
  sgCfg[5].type = SG_TYPE_PEDESTRIAN;

  /* SG 0 and SG 1 Conflict with SG 2-5 */
  for (j = 2U; j < FIXTURE_6SG_COUNT; j++)
  {
    sgCfg[0].Conflicts[j].hasConflict = true;
    sgCfg[1].Conflicts[j].hasConflict = true;
  }

  /* SG 2 and SG 3 Conflict with SG 0-1 */
  sgCfg[2].Conflicts[0].hasConflict = true;
  sgCfg[2].Conflicts[1].hasConflict = true;
  sgCfg[3].Conflicts[0].hasConflict = true;
  sgCfg[3].Conflicts[1].hasConflict = true;
  /* SG 4 and SG 5 Conflict with SG 0-3 */
  for (j = 0U; j < 4U; j++)
  {
    sgCfg[4].Conflicts[j].hasConflict = true;
    sgCfg[5].Conflicts[j].hasConflict = true;
  }
} /* Fixture6SG_Build */
