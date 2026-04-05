#pragma once

/*
 * App/Domain/NTCIP/NTCIP1202.h
 *
 * NTCIP 1202 (Object Definitions for Actuated Traffic Signal Controllers)
 * status and fault objects.
 *
 * Provides Domain-facing GET functions that read live ProgramCtx_t state.
 * The LWIP SNMP adapter calls these from its OID get callbacks.
 * Fault traps are emitted by Domain code via ISNMPNotifierPort, not here.
 */
#include "Domain/Intersection/Program.h"

/* ---------------------------------------------------------------------------
 * Phase current status bitmask (NTCIP 1202 §3.8.1)
 * ---------------------------------------------------------------------------*/
typedef uint8_t Ntcip1202PhaseStatus_t;

#define NTCIP1202_PHASE_STATUS_DONT_WALK     0x01U  /* Ped don't walk signal active */
#define NTCIP1202_PHASE_STATUS_WALK          0x02U  /* Ped walk active */
#define NTCIP1202_PHASE_STATUS_PED_CLEAR     0x04U  /* Pedestrian clearance interval */
#define NTCIP1202_PHASE_STATUS_MIN_RECALL    0x08U  /* Min recall active */
#define NTCIP1202_PHASE_STATUS_MAX_RECALL    0x10U  /* Max recall active */
#define NTCIP1202_PHASE_STATUS_OLT_FLASH     0x20U  /* Overlap/turn flash */
#define NTCIP1202_PHASE_STATUS_ACTIVE        0x40U  /* Phase currently running */
#define NTCIP1202_PHASE_STATUS_MAX_OUT       0x80U  /* Max green time elapsed */

/* ---------------------------------------------------------------------------
 * Detector status (NTCIP 1202 §3.11)
 * ---------------------------------------------------------------------------*/
typedef struct
{
  uint8_t DetectorCallStatus;       /* Non-zero = demand present */
  uint16_t DetectorOccupancyMs;     /* Cumulative occupancy this period */
  uint8_t DetectorVolume;           /* Demand count in period */
  bool DetectorFault;               /* True if Detector broken */
} Ntcip1202DetectorEntry_t;

/* ---------------------------------------------------------------------------
 * Alarm (fault) entry (NTCIP 1202 §3.15)
 * ---------------------------------------------------------------------------*/
typedef enum
{
  NTCIP1202_ALARM_NONE               = 0,
  NTCIP1202_ALARM_LAMP_FAILURE       = 1,     /* Red lamp failure */
  NTCIP1202_ALARM_DETECTOR_FAILURE   = 2,
  NTCIP1202_ALARM_CONFLICT_FAULT     = 3,
  NTCIP1202_ALARM_POWER_FAILURE      = 4,
  NTCIP1202_ALARM_COMM_FAILURE       = 5,
} Ntcip1202AlarmType_t;

typedef struct
{
  Ntcip1202AlarmType_t alarmType;
  uint8_t objectIndex;                  /* SG index or Detector index */
  bool active;
} Ntcip1202AlarmEntry_t;

/* System date/time (NTCIP 1201 §2.4.6, used by 1202 as well) */
typedef struct
{
  uint16_t year;
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
} Ntcip1202DateTime_t;

/* ---------------------------------------------------------------------------
 * Phase status GET
 * ---------------------------------------------------------------------------*/

/**
 * Return the NTCIP 1202 phase status bitmask for the given phase (0-based).
 * Returns 0 if phaseIdx is out of range.
 */
Ntcip1202PhaseStatus_t Ntcip1202_GetPhaseStatus(const ProgramCtx_t *ctx,
                                                uint8_t phaseIdx);

/* ---------------------------------------------------------------------------
 * Detector table GET
 * ---------------------------------------------------------------------------*/

/**
 * Fill out a Detector status row. Returns false if detIdx out of range.
 */
bool Ntcip1202_GetDetector(const ProgramCtx_t *ctx, uint8_t detIdx,
                           Ntcip1202DetectorEntry_t *out);

/* ---------------------------------------------------------------------------
 * Alarm table GET
 * ---------------------------------------------------------------------------*/

/**
 * Scan the current runtime for active faults and populate out[].
 * outCapacity is the max entries to write; returns the number written.
 * The caller typically sizes outCapacity = SIGNAL_GROUPS_MAX + DETECTORS_MAX.
 */
uint8_t Ntcip1202_GetActiveAlarms(const ProgramCtx_t *ctx,
                                  Ntcip1202AlarmEntry_t *out,
                                  uint8_t outCapacity);

/* ---------------------------------------------------------------------------
 * System date/time
 * ---------------------------------------------------------------------------*/

/**
 * Read the current system time via the ISystemClockPort and return a
 * human-readable Ntcip1202DateTime_t. Epoch→calendar conversion is
 * intentionally minimal (leap years handled, no DST applied here).
 */
Ntcip1202DateTime_t Ntcip1202_GetDateTime(const ProgramCtx_t *ctx);

/**
 * Write system time from an SNMP SET. Converts calendar to epoch and calls
 * SystemClock_SetEpoch(). Returns false if the time values are invalid.
 */
bool Ntcip1202_SetDateTime(ProgramCtx_t *ctx, const Ntcip1202DateTime_t *dt);
