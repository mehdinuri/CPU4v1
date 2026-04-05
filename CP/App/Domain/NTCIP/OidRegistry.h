#pragma once

/*
 * App/Domain/NTCIP/OidRegistry.h
 *
 * OID handler registry — transport-agnostic NTCIP object dispatch table.
 *
 * The LWIP SNMP adapter (App/Adapters/STM32/LWIPSNMPAdapter.c) registers its
 * OID subtree with the LWIP SNMP engine, then forwards GET/SET requests
 * to OidRegistry_Get() / OidRegistry_Set() defined here.
 *
 * Each object is addressed by a flat OidObjectId_t enum. The adapter
 * maps OID paths → enum values using a static lookup table.
 */
#include "Domain/Intersection/Program.h"
#include <stdint.h>
#include <stdbool.h>

/* ---------------------------------------------------------------------------
 * Flat object identifiers
 *
 * Naming follows NTCIP 1201/1202 object names; suffix _n means the object
 * is a table row indexed by the accompanying index parameter.
 * ---------------------------------------------------------------------------*/
typedef enum
{
  /* NTCIP 1201 scalars */
  OID_MAX_PHASES                  = 1U,
  OID_UNIT_CONTROL_MODE           = 2U,
  OID_ACTIVE_TIMING_PLAN          = 3U,

  /* NTCIP 1201 phaseTable (indexed by phaseIdx 0-based) */
  OID_PHASE_MIN_GREEN_n           = 10U,
  OID_PHASE_MAX_GREEN_n           = 11U,
  OID_PHASE_YELLOW_CHANGE_n       = 12U,    /* Per-SG: idx = first SG in phase */
  OID_PHASE_RED_CLEARANCE_n       = 13U,    /* Per-SG-pair */
  OID_PHASE_WALK_n                = 14U,
  OID_PHASE_PED_CLEARANCE_n       = 15U,

  /* NTCIP 1201 phaseStatusGroupTable (read-only) */
  OID_PHASE_STATUS_n              = 20U,
  OID_PHASE_ELAPSED_GREEN_n       = 21U,

  /* NTCIP 1202 phase current status (bitmask) */
  OID_PHASE_CURRENT_STATUS_n      = 30U,

  /* NTCIP 1202 DetectorTable */
  OID_DETECTOR_CALL_STATUS_n      = 40U,
  OID_DETECTOR_OCCUPANCY_n        = 41U,
  OID_DETECTOR_VOLUME_n           = 42U,
  OID_DETECTOR_FAULT_n            = 43U,

  /* NTCIP 1202 system date/time */
  OID_SYSTEM_DATE_TIME            = 50U,

  /* NTCIP 1202 alarms (read-only) */
  OID_ALARM_COUNT                 = 60U,
  OID_ALARM_ENTRY_n               = 61U,

  OID_UNKNOWN                     = 0xFFU,
} OidObjectId_t;

/* ---------------------------------------------------------------------------
 * GET / SET result codes
 * ---------------------------------------------------------------------------*/
typedef enum
{
  OID_RESULT_OK           = 0,
  OID_RESULT_NOT_FOUND    = 1,
  OID_RESULT_READ_ONLY    = 2,
  OID_RESULT_BAD_VALUE    = 3,
  OID_RESULT_RANGE_ERR    = 4,
} OidResult_t;

/* ---------------------------------------------------------------------------
 * Value union — supPorts integer, octet-string, and boolean OID values.
 * The type is implicit from the OidObjectId_t (caller knows the schema).
 * ---------------------------------------------------------------------------*/
typedef struct
{
  uint32_t intVal;              /* For integer / unsigned / boolean objects */
  uint8_t strVal[32];           /* For octet-string objects (e.g. dateTime) */
  uint8_t strLen;               /* Bytes valid in strVal */
} OidValue_t;

/* ---------------------------------------------------------------------------
 * Public API
 * ---------------------------------------------------------------------------*/

/**
 * Perform a GET for the given object.
 * @param ctx       Live Program context (read-only for GET)
 * @param id        Object identifier
 * @param index     Row index for table objects (ignored for scalars)
 * @param out       Filled with the current value on OID_RESULT_OK
 */
OidResult_t OidRegistry_Get(const ProgramCtx_t *ctx, OidObjectId_t id,
                            uint8_t index, OidValue_t *out);

/**
 * Perform a SET for the given object.
 * @param ctx       Live Program context (modified on success)
 * @param id        Object identifier
 * @param index     Row index for table objects
 * @param value     New value
 */
OidResult_t OidRegistry_Set(ProgramCtx_t *ctx, OidObjectId_t id,
                            uint8_t index, const OidValue_t *value);
