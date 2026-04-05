#pragma once

/*
 * App/Domain/NTCIP/NTCIP1201.h
 *
 * NTCIP 1201 (UTMC General) phase timing object model.
 * Provides Domain-facing GET/SET functions that read/write ProgramCtx_t
 * directly — no SNMP transport coupling.
 *
 * The LWIP SNMP adapter calls these from its OID get/set callbacks.
 */
#include "Domain/Intersection/Program.h"

/* ---------------------------------------------------------------------------
 * NTCIP 1201 Phase table row — one entry per phase
 * ---------------------------------------------------------------------------*/
typedef struct
{
  uint8_t phaseNumber;                 /* 1-based phase number (NTCIP) */
  uint8_t phaseMinGreenTime;           /* seconds */
  uint8_t phaseMaxGreenTime;           /* seconds */
  uint8_t phaseYellowChangeInterval;   /* seconds */
  uint8_t phaseRedClearanceInterval;   /* seconds */
  uint8_t phaseWalk;                   /* seconds (pedestrian walk) */
  uint8_t phasePedestrianClearance;    /* seconds (ped clearance / green-flash) */
  bool phaseEnabled;
} Ntcip1201PhaseEntry_t;

/* Unit-level parameters */
typedef struct
{
  uint8_t maxPhases;           /* Total phases configured (read-only) */
  uint8_t unitControlMode;     /* Maps to ControlMode_t */
  uint8_t activeTimingPlanIndex;
} Ntcip1201UnitStatus_t;

/* Phase status (current runtime snapshot) */
typedef struct
{
  uint8_t phaseNumber;
  uint16_t elapsedGreenSeconds;
  bool isActive;
  bool minTimeElapsed;
  bool maxTimeElapsed;
} Ntcip1201PhaseStatus_t;

/* ---------------------------------------------------------------------------
 * Phase table GET/SET
 * ---------------------------------------------------------------------------*/

/**
 * Read NTCIP 1201 phase timing parameters for phaseIdx (0-based Domain index).
 * Returns false if phaseIdx is out of range.
 */
bool Ntcip1201_GetPhase(const ProgramCtx_t *ctx, uint8_t phaseIdx,
                        Ntcip1201PhaseEntry_t *out);

/**
 * Write phase timing parameters (SET from SNMP).
 * Validates that minGreen <= maxGreen before applying.
 * Also persists via IPersistentStoragePort (caller responsible for flushing).
 * Returns false on validation failure.
 */
bool Ntcip1201_SetPhaseMinGreen(ProgramCtx_t *ctx,
                                uint8_t phaseIdx,
                                uint8_t value);
bool Ntcip1201_SetPhaseMaxGreen(ProgramCtx_t *ctx,
                                uint8_t phaseIdx,
                                uint8_t value);
bool Ntcip1201_SetPhaseYellowChange(ProgramCtx_t *ctx,
                                    uint8_t sgIdx,
                                    uint8_t value);
bool Ntcip1201_SetPhaseRedClearance(ProgramCtx_t *ctx,
                                    uint8_t sgIdx,
                                    uint8_t sgJIdx,
                                    uint8_t value);

/* ---------------------------------------------------------------------------
 * Unit status GET/SET
 * ---------------------------------------------------------------------------*/

Ntcip1201UnitStatus_t Ntcip1201_GetUnitStatus(const ProgramCtx_t *ctx);

/**
 * Request a control mode change via SNMP SET.
 * Maps NTCIP controlMode integers to ControlMode_t.
 */
bool Ntcip1201_SetControlMode(ProgramCtx_t *ctx, uint8_t NTCIPControlMode);

/* ---------------------------------------------------------------------------
 * Phase status GET (read-only runtime state)
 * ---------------------------------------------------------------------------*/

bool Ntcip1201_GetPhaseStatus(const ProgramCtx_t *ctx, uint8_t phaseIdx,
                              Ntcip1201PhaseStatus_t *out);

/* ---------------------------------------------------------------------------
 * Max-values (read-only scalars)
 * ---------------------------------------------------------------------------*/

static inline uint8_t Ntcip1201_MaxPhases(void)
{
  return PHASES_MAX;
}

static inline uint8_t Ntcip1201_MaxDetectors(void)
{
  return DETECTORS_MAX;
}
