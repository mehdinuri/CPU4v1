#pragma once

/*
 * Tests/Fixtures/TimingPlan_4Phase.h
 *
 * A minimal 4-phase Intersection fixture used by unit and integration tests.
 *
 * Phase layout (North-South / East-West split):
 *   Phase 0: SG 0, SG 1   (NS through-traffic)
 *   Phase 1: SG 2, SG 3   (EW through-traffic)
 *   Phase 2: SG 4          (NS pedestrian)
 *   Phase 3: SG 5          (EW pedestrian)
 *
 * Conflict matrix:
 *   SG 0 Conflicts with SG 2, SG 3, SG 4, SG 5
 *   SG 1 Conflicts with SG 2, SG 3, SG 4, SG 5
 *   SG 2 Conflicts with SG 0, SG 1
 *   SG 3 Conflicts with SG 0, SG 1
 *   SG 4 Conflicts with SG 0, SG 1
 *   SG 5 Conflicts with SG 0, SG 1
 */
#include "Domain/Intersection/Types.h"

/* Number of SGs and phases in this fixture */
#define FIXTURE_4P_SG_COUNT     6U
#define FIXTURE_4P_PHASE_COUNT  4U

/* Build a SignalGroupConfig_t for a simple vehicle SG. */
static inline SignalGroupConfig_t make_vehicle_sg(uint8_t outputIdx,
                                                  uint8_t yellowDuration,
                                                  uint8_t redClearance)
{
  SignalGroupConfig_t cfg;
  int i;

  for (i = 0; i < SIGNAL_GROUPS_MAX; i++)
  {
    cfg.Conflicts[i].hasConflict = false;
    cfg.Conflicts[i].redClearanceInterval = 0U;
  }

  cfg.type = SG_TYPE_VEHICLE_MAINWAY;
  cfg.openingSignalIdx = 0U;
  cfg.closingSignalIdx = 0U;
  cfg.openingDuration = 0U;
  cfg.yellowChangeInterval = yellowDuration;
  cfg.pedestrianClearance = 0U;
  cfg.pedestrianWalk = 0U;
  cfg.flashSignalIdx = 0U;
  cfg.firstOutputIndex = outputIdx;
  cfg.criticalRedLampCount = 1U;
  (void) redClearance;

  return cfg;
}

/* Build a SignalGroupConfig_t for a pedestrian SG. */
static inline SignalGroupConfig_t make_ped_sg(uint8_t outputIdx,
                                              uint8_t walkTime,
                                              uint8_t clearTime)
{
  SignalGroupConfig_t cfg;
  int i;

  for (i = 0; i < SIGNAL_GROUPS_MAX; i++)
  {
    cfg.Conflicts[i].hasConflict = false;
    cfg.Conflicts[i].redClearanceInterval = 0U;
  }

  cfg.type = SG_TYPE_PEDESTRIAN;
  cfg.openingSignalIdx = 0U;
  cfg.closingSignalIdx = 0U;
  cfg.openingDuration = 0U;
  cfg.yellowChangeInterval = 0U;
  cfg.pedestrianClearance = clearTime;
  cfg.pedestrianWalk = walkTime;
  cfg.flashSignalIdx = 0U;
  cfg.firstOutputIndex = outputIdx;
  cfg.criticalRedLampCount = 1U;

  return cfg;
}

/*
 * Populate sgConfigs[0..5] with the 4-phase fixture signal groups,
 * including the NS/EW Conflict pairs.
 */
static inline void Fixture4P_BuildSGs(
  SignalGroupConfig_t sgConfigs[FIXTURE_4P_SG_COUNT])
{
  /* NS vehicles: SG 0, SG 1 */
  sgConfigs[0] = make_vehicle_sg(0U, 3U, 2U);
  sgConfigs[1] = make_vehicle_sg(3U, 3U, 2U);
  /* EW vehicles: SG 2, SG 3 */
  sgConfigs[2] = make_vehicle_sg(6U, 3U, 2U);
  sgConfigs[3] = make_vehicle_sg(9U, 3U, 2U);
  /* NS pedestrian: SG 4 */
  sgConfigs[4] = make_ped_sg(12U, 5U, 4U);
  /* EW pedestrian: SG 5 */
  sgConfigs[5] = make_ped_sg(15U,
                             5U,
                             4U);

  /* NS vehicles Conflict with EW everything */
  sgConfigs[0].Conflicts[2].hasConflict = true;
  sgConfigs[0].Conflicts[3].hasConflict = true;
  sgConfigs[0].Conflicts[4].hasConflict = true;
  sgConfigs[0].Conflicts[5].hasConflict = true;
  sgConfigs[1].Conflicts[2].hasConflict = true;
  sgConfigs[1].Conflicts[3].hasConflict = true;
  sgConfigs[1].Conflicts[4].hasConflict = true;
  sgConfigs[1].Conflicts[5].hasConflict = true;

  /* EW vehicles Conflict with NS everything (mirror) */
  sgConfigs[2].Conflicts[0].hasConflict = true;
  sgConfigs[2].Conflicts[1].hasConflict = true;
  sgConfigs[3].Conflicts[0].hasConflict = true;
  sgConfigs[3].Conflicts[1].hasConflict = true;
  sgConfigs[4].Conflicts[0].hasConflict = true;
  sgConfigs[4].Conflicts[1].hasConflict = true;
  sgConfigs[5].Conflicts[0].hasConflict = true;
  sgConfigs[5].Conflicts[1].hasConflict = true;
}

/* Standard 4-phase configs */
static inline void Fixture4P_BuildPhases(
  PhaseConfig_t phases[FIXTURE_4P_PHASE_COUNT])
{
  /* Phase 0: SG 0 + SG 1, min 10s, max 60s */
  phases[0].signalGroupMask = (1UL << 0U) | (1UL << 1U);
  phases[0].minGreenTime = 10U;
  phases[0].maxGreenTime = 60U;

  /* Phase 1: SG 2 + SG 3, min 10s, max 60s */
  phases[1].signalGroupMask = (1UL << 2U) | (1UL << 3U);
  phases[1].minGreenTime = 10U;
  phases[1].maxGreenTime = 60U;

  /* Phase 2: SG 4 (NS ped), min 5s, max 20s */
  phases[2].signalGroupMask = (1UL << 4U);
  phases[2].minGreenTime = 5U;
  phases[2].maxGreenTime = 20U;

  /* Phase 3: SG 5 (EW ped), min 5s, max 20s */
  phases[3].signalGroupMask = (1UL << 5U);
  phases[3].minGreenTime = 5U;
  phases[3].maxGreenTime = 20U;
}
