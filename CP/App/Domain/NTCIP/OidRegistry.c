/*
 * App/Domain/NTCIP/OidRegistry.c
 *
 * OID handler dispatch — maps OidObjectId_t to NTCIP1201/1202 calls.
 */
#include "Domain/NTCIP/OidRegistry.h"
#include "Domain/NTCIP/NTCIP1201.h"
#include "Domain/NTCIP/NTCIP1202.h"

/* ---------------------------------------------------------------------------
 * GET
 * ---------------------------------------------------------------------------*/

OidResult_t OidRegistry_Get(const ProgramCtx_t *ctx, OidObjectId_t id,
                            uint8_t index, OidValue_t *out)
{
  if (out == (void *) 0)
  {
    return OID_RESULT_BAD_VALUE;
  }

  switch (id)
  {
      /* ---- NTCIP 1201 scalars ---- */
      case OID_MAX_PHASES:
      {
        out->intVal = Ntcip1201_MaxPhases();

        return OID_RESULT_OK;
      }

      case OID_UNIT_CONTROL_MODE:
      {
        Ntcip1201UnitStatus_t s = Ntcip1201_GetUnitStatus(ctx);

        out->intVal = s.unitControlMode;

        return OID_RESULT_OK;
      }

      case OID_ACTIVE_TIMING_PLAN:
      {
        Ntcip1201UnitStatus_t s = Ntcip1201_GetUnitStatus(ctx);

        out->intVal = s.activeTimingPlanIndex;

        return OID_RESULT_OK;
      }

      /* ---- NTCIP 1201 phaseTable ---- */
      case OID_PHASE_MIN_GREEN_n:
      {
        Ntcip1201PhaseEntry_t e;

        if (!Ntcip1201_GetPhase(ctx, index, &e))
        {
          return OID_RESULT_RANGE_ERR;
        }

        out->intVal = e.phaseMinGreenTime;

        return OID_RESULT_OK;
      }

      case OID_PHASE_MAX_GREEN_n:
      {
        Ntcip1201PhaseEntry_t e;

        if (!Ntcip1201_GetPhase(ctx, index, &e))
        {
          return OID_RESULT_RANGE_ERR;
        }

        out->intVal = e.phaseMaxGreenTime;

        return OID_RESULT_OK;
      }

      case OID_PHASE_YELLOW_CHANGE_n:
      {
        Ntcip1201PhaseEntry_t e;

        if (!Ntcip1201_GetPhase(ctx, index, &e))
        {
          return OID_RESULT_RANGE_ERR;
        }

        out->intVal = e.phaseYellowChangeInterval;

        return OID_RESULT_OK;
      }

      case OID_PHASE_WALK_n:
      {
        Ntcip1201PhaseEntry_t e;

        if (!Ntcip1201_GetPhase(ctx, index, &e))
        {
          return OID_RESULT_RANGE_ERR;
        }

        out->intVal = e.phaseWalk;

        return OID_RESULT_OK;
      }

      case OID_PHASE_PED_CLEARANCE_n:
      {
        Ntcip1201PhaseEntry_t e;

        if (!Ntcip1201_GetPhase(ctx, index, &e))
        {
          return OID_RESULT_RANGE_ERR;
        }

        out->intVal = e.phasePedestrianClearance;

        return OID_RESULT_OK;
      }

      /* ---- NTCIP 1201 phaseStatus ---- */
      case OID_PHASE_STATUS_n:
      {
        Ntcip1201PhaseStatus_t s;

        if (!Ntcip1201_GetPhaseStatus(ctx, index, &s))
        {
          return OID_RESULT_RANGE_ERR;
        }

        out->intVal = s.isActive ? 1U : 0U;

        return OID_RESULT_OK;
      }

      case OID_PHASE_ELAPSED_GREEN_n:
      {
        Ntcip1201PhaseStatus_t s;

        if (!Ntcip1201_GetPhaseStatus(ctx, index, &s))
        {
          return OID_RESULT_RANGE_ERR;
        }

        out->intVal = s.elapsedGreenSeconds;

        return OID_RESULT_OK;
      }

      /* ---- NTCIP 1202 phase current status ---- */
      case OID_PHASE_CURRENT_STATUS_n:
      {
        out->intVal = Ntcip1202_GetPhaseStatus(ctx, index);

        return OID_RESULT_OK;
      }

      /* ---- NTCIP 1202 Detector table ---- */
      case OID_DETECTOR_CALL_STATUS_n:
      {
        Ntcip1202DetectorEntry_t d;

        if (!Ntcip1202_GetDetector(ctx, index, &d))
        {
          return OID_RESULT_RANGE_ERR;
        }

        out->intVal = d.DetectorCallStatus;

        return OID_RESULT_OK;
      }

      case OID_DETECTOR_OCCUPANCY_n:
      {
        Ntcip1202DetectorEntry_t d;

        if (!Ntcip1202_GetDetector(ctx, index, &d))
        {
          return OID_RESULT_RANGE_ERR;
        }

        out->intVal = d.DetectorOccupancyMs;

        return OID_RESULT_OK;
      }

      case OID_DETECTOR_VOLUME_n:
      {
        Ntcip1202DetectorEntry_t d;

        if (!Ntcip1202_GetDetector(ctx, index, &d))
        {
          return OID_RESULT_RANGE_ERR;
        }

        out->intVal = d.DetectorVolume;

        return OID_RESULT_OK;
      }

      case OID_DETECTOR_FAULT_n:
      {
        Ntcip1202DetectorEntry_t d;

        if (!Ntcip1202_GetDetector(ctx,
                                   index,
                                   &d))
        {
          return OID_RESULT_RANGE_ERR;
        }

        out->intVal = d.DetectorFault ? 1U : 0U;

        return OID_RESULT_OK;
      }

      /* ---- NTCIP 1202 system date/time ---- */
      case OID_SYSTEM_DATE_TIME:
      {
        Ntcip1202DateTime_t dt = Ntcip1202_GetDateTime(ctx);

        /* Encode as 8-byte DateAndTime (SNMPv2 TEXTUAL-CONVENTION) */
        out->strVal[0] = (uint8_t) (dt.year >> 8U);
        out->strVal[1] = (uint8_t) (dt.year & 0xFFU);
        out->strVal[2] = dt.month;
        out->strVal[3] = dt.day;
        out->strVal[4] = dt.hour;
        out->strVal[5] = dt.minute;
        out->strVal[6] = dt.second;
        out->strVal[7] = 0U;       /* deci-seconds */
        out->strLen = 8U;

        return OID_RESULT_OK;
      }

      /* ---- NTCIP 1202 alarm count ---- */
      case OID_ALARM_COUNT:
      {
        Ntcip1202AlarmEntry_t alarms[SIGNAL_GROUPS_MAX + DETECTORS_MAX];
        uint8_t n = Ntcip1202_GetActiveAlarms(ctx, alarms,
                                              (uint8_t) (SIGNAL_GROUPS_MAX
                                                         + DETECTORS_MAX));

        out->intVal = n;

        return OID_RESULT_OK;
      }

      default:
      {
        return OID_RESULT_NOT_FOUND;
      }
  } /* switch */
} /* OidRegistry_Get */

/* ---------------------------------------------------------------------------
 * SET
 * ---------------------------------------------------------------------------*/

OidResult_t OidRegistry_Set(ProgramCtx_t *ctx, OidObjectId_t id,
                            uint8_t index, const OidValue_t *value)
{
  if (value == (void *) 0)
  {
    return OID_RESULT_BAD_VALUE;
  }

  switch (id)
  {
      case OID_UNIT_CONTROL_MODE:
      {
        return Ntcip1201_SetControlMode(ctx, (uint8_t) value->intVal)
               ? OID_RESULT_OK : OID_RESULT_BAD_VALUE;
      }

      case OID_PHASE_MIN_GREEN_n:
      {
        return Ntcip1201_SetPhaseMinGreen(ctx, index, (uint8_t) value->intVal)
               ? OID_RESULT_OK : OID_RESULT_BAD_VALUE;
      }

      case OID_PHASE_MAX_GREEN_n:
      {
        return Ntcip1201_SetPhaseMaxGreen(ctx, index, (uint8_t) value->intVal)
               ? OID_RESULT_OK : OID_RESULT_BAD_VALUE;
      }

      case OID_PHASE_YELLOW_CHANGE_n:
      {
        return Ntcip1201_SetPhaseYellowChange(ctx,
                                              index,
                                              (uint8_t) value->intVal)
               ? OID_RESULT_OK : OID_RESULT_BAD_VALUE;
      }

      case OID_SYSTEM_DATE_TIME:
      {
        if (value->strLen < 7U)
        {
          return OID_RESULT_BAD_VALUE;
        }

        Ntcip1202DateTime_t dt;

        dt.year = ((uint16_t) value->strVal[0] << 8U) | value->strVal[1];
        dt.month = value->strVal[2];
        dt.day = value->strVal[3];
        dt.hour = value->strVal[4];
        dt.minute = value->strVal[5];
        dt.second = value->strVal[6];

        return Ntcip1202_SetDateTime(ctx, &dt)
               ? OID_RESULT_OK : OID_RESULT_BAD_VALUE;
      }

      /* Read-only objects */
      case OID_MAX_PHASES:
      case OID_PHASE_STATUS_n:
      case OID_PHASE_ELAPSED_GREEN_n:
      case OID_PHASE_CURRENT_STATUS_n:
      case OID_DETECTOR_CALL_STATUS_n:
      case OID_DETECTOR_OCCUPANCY_n:
      case OID_DETECTOR_VOLUME_n:
      case OID_DETECTOR_FAULT_n:
      case OID_ALARM_COUNT:
      case OID_ALARM_ENTRY_n:
      {
        return OID_RESULT_READ_ONLY;
      }

      default:
      {
        return OID_RESULT_NOT_FOUND;
      }
  } /* switch */
} /* OidRegistry_Set */
