/*
 * Tests/Unit/Test_NtcipObjectDirectory.c
 *
 * Unit tests for raw-OID object routing and 1201/1202 handler behavior.
 */
#include "unity.h"

#include "Domain/Intersection/CpMpLinkService.h"
#include "Domain/Intersection/IntersectionController.h"
#include "Domain/Intersection/IntersectionEngine.h"
#include "Domain/NTCIP/NTCIP1201.h"
#include "Domain/NTCIP/NTCIP1202.h"
#include "Domain/NTCIP/MibVendor59748/CpMpDiagnosticsObjects.h"
#include "Domain/NTCIP/Core/NtcipDbTransactionService.h"
#include "Domain/NTCIP/Core/NtcipObjectDirectory.h"
#include "Domain/NTCIP/NtcipContext.h"
#include "MockConfigRepositoryAdapter.h"
#include "MockUnitAlarmAdapter.h"

#include <string.h>

static const uint32_t kGlobalSetIdParameterOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 1U, 1U, 0U
};
static const uint32_t kDbCreateTransactionOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 2U, 1U, 0U
};
static const uint32_t kDbTransactionIdOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 2U, 4U, 0U
};
static const uint32_t kDbVerifyStatusOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 2U, 6U, 0U
};
static const uint32_t kMaxAuxIoDigitalOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 7U, 1U, 0U
};
static const uint32_t kPhaseMinimumGreenOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 1U, 2U, 1U, 4U, 1U
};
static const uint32_t kPhaseMaximumInitialOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 1U, 2U, 1U, 12U, 1U
};
static const uint32_t kPhaseWalkOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 1U, 2U, 1U, 2U, 1U
};
static const uint32_t kPhasePassageOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 1U, 2U, 1U, 5U, 1U
};
static const uint32_t kPhaseRingOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 1U, 2U, 1U, 22U, 1U
};
static const uint32_t kPhaseStatusGroupGreensOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 4U, 1U, 4U, 1U
};
static const uint32_t kPhaseStatusGroupWalksOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 4U, 1U, 7U, 1U
};
static const uint32_t kPhaseStatusGroupVehCallsOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 4U, 1U, 8U, 1U
};
static const uint32_t kPhaseStatusGroupPedCallsOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 4U, 1U, 9U, 1U
};
static const uint32_t kPhaseControlGroupPhaseOmitOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 1U, 5U, 1U, 2U, 1U
};
static const uint32_t kPhaseControlGroupHoldOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 1U, 5U, 1U, 4U, 1U
};
static const uint32_t kPhaseControlGroupForceOffOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 1U, 5U, 1U, 5U, 1U
};
static const uint32_t kPhaseControlGroupVehCallOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 1U, 5U, 1U, 6U, 1U
};
static const uint32_t kPhaseControlGroupPedCallOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 1U, 5U, 1U, 7U, 1U
};
static const uint32_t kMaxRingControlGroupsOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 7U, 4U, 0U
};
static const uint32_t kMaxSequencesOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 7U, 2U, 0U
};
static const uint32_t kSequenceNumberOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 7U, 3U, 1U, 1U, 1U, 1U
};
static const uint32_t kSequenceRingNumberOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 7U, 3U, 1U, 2U, 1U, 1U
};
static const uint32_t kSequenceDataOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 7U, 3U, 1U, 3U, 1U, 1U
};
static const uint32_t kRingControlGroupStopTimeOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 7U, 5U, 1U, 2U, 1U
};
static const uint32_t kRingControlGroupForceOffOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 7U, 5U, 1U, 3U, 1U
};
static const uint32_t kRingControlGroupMax2Oid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 7U, 5U, 1U, 4U, 1U
};
static const uint32_t kRingControlGroupRedRestOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 7U, 5U, 1U, 7U, 1U
};
static const uint32_t kChannelControlSourceOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 8U, 2U, 1U, 2U, 1U
};
static const uint32_t kChannelControlTypeOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 8U, 2U, 1U, 3U, 1U
};
static const uint32_t kChannelStatusGroupGreensOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 8U, 4U, 1U, 4U, 1U
};
static const uint32_t kChannelStatusGroupYellowsOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 8U, 4U, 1U, 3U, 1U
};
static const uint32_t kOverlapTypeOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 9U, 2U, 1U, 2U, 1U
};
static const uint32_t kOverlapIncludedPhasesOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 9U, 2U, 1U, 3U, 1U
};
static const uint32_t kOverlapStatusGroupGreensOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 9U, 4U, 1U, 4U, 1U
};
static const uint32_t kCoordOperationalModeOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 4U, 1U, 0U
};
static const uint32_t kPatternCycleTimeOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 4U, 7U, 1U, 2U, 1U
};
static const uint32_t kCoordPatternStatusOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 4U, 10U, 0U
};
static const uint32_t kLocalFreeStatusOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 4U, 11U, 0U
};
static const uint32_t kCoordCycleStatusOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 4U, 12U, 0U
};
static const uint32_t kCoordSyncStatusOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 4U, 13U, 0U
};
static const uint32_t kVehicleDetectorCallPhaseOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 2U, 1U, 4U, 1U
};
static const uint32_t kVehicleDetectorOptionsOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 2U, 1U, 2U, 1U
};
static const uint32_t kVehicleDetectorSwitchPhaseOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 2U, 1U, 5U, 1U
};
static const uint32_t kVehicleDetectorDelayOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 2U, 1U, 6U, 1U
};
static const uint32_t kVehicleDetectorExtendOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 2U, 1U, 7U, 1U
};
static const uint32_t kVehicleDetectorQueueLimitOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 2U, 1U, 8U, 1U
};
static const uint32_t kVehicleDetectorNoActivityOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 2U, 1U, 9U, 1U
};
static const uint32_t kVehicleDetectorMaxPresenceOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 2U, 1U, 10U, 1U
};
static const uint32_t kVehicleDetectorErraticCountsOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 2U, 1U, 11U, 1U
};
static const uint32_t kVehicleDetectorFailTimeOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 2U, 1U, 12U, 1U
};
static const uint32_t kVehicleDetectorAlarmsOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 2U, 1U, 13U, 1U
};
static const uint32_t kVehicleDetectorReportedAlarmsOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 2U, 1U, 14U, 1U
};
static const uint32_t kVehicleDetectorResetOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 2U, 1U, 15U, 1U
};
static const uint32_t kVehicleDetectorOptions2Oid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 2U, 1U, 16U, 1U
};
static const uint32_t kVehicleDetectorPairedDetectorOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 2U, 1U, 17U, 1U
};
static const uint32_t kVehicleDetectorPairedDetectorSpacingOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 2U, 1U, 18U, 1U
};
static const uint32_t kVehicleDetectorAvgVehicleLengthOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 2U, 1U, 19U, 1U
};
static const uint32_t kVehicleDetectorLengthOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 2U, 1U, 20U, 1U
};
static const uint32_t kVehicleDetectorTravelModeOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 2U, 1U, 21U, 1U
};
static const uint32_t kVehicleDetectorStatusGroupActiveOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 4U, 1U, 2U, 1U
};
static const uint32_t kMaxVehicleDetectorControlGroupsOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 11U, 0U
};
static const uint32_t kVehicleDetectorControlGroupActuationOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 12U, 1U, 2U, 1U
};
static const uint32_t kPedestrianDetectorCallPhaseOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 7U, 1U, 2U, 1U
};
static const uint32_t kPedestrianDetectorNoActivityOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 7U, 1U, 3U, 1U
};
static const uint32_t kPedestrianDetectorMaxPresenceOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 7U, 1U, 4U, 1U
};
static const uint32_t kPedestrianDetectorErraticCountsOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 7U, 1U, 5U, 1U
};
static const uint32_t kPedestrianDetectorAlarmsOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 7U, 1U, 6U, 1U
};
static const uint32_t kPedestrianDetectorResetOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 7U, 1U, 7U, 1U
};
static const uint32_t kPedestrianButtonPushTimeOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 7U, 1U, 8U, 1U
};
static const uint32_t kPedestrianDetectorOptionsOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 7U, 1U, 9U, 1U
};
static const uint32_t kPedestrianDetectorStatusGroupActiveOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 9U, 1U, 2U, 1U
};
static const uint32_t kPedestrianDetectorControlGroupActuationOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 13U, 1U, 2U, 1U
};
static const uint32_t kUnitControlStatusOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 3U, 5U, 0U
};
static const uint32_t kUnitStartUpFlashOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 3U, 1U, 0U
};
static const uint32_t kUnitAutoPedestrianClearOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 3U, 2U, 0U
};
static const uint32_t kUnitBackupTimeOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 3U, 3U, 0U
};
static const uint32_t kUnitRedRevertOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 3U, 4U, 0U
};
static const uint32_t kUnitFlashStatusOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 3U, 6U, 0U
};
static const uint32_t kUnitControlOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 3U, 10U, 0U
};
static const uint32_t kMaxAlarmGroupsOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 3U, 11U, 0U
};
static const uint32_t kAlarmGroupNumberOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 3U, 12U, 1U, 1U, 1U
};
static const uint32_t kAlarmGroupStateOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 3U, 12U, 1U, 2U, 1U
};
static const uint32_t kUnitAlarmStatus2Oid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 3U, 7U, 0U
};
static const uint32_t kUnitAlarmStatus1Oid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 3U, 8U, 0U
};
static const uint32_t kShortAlarmStatusOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 3U, 9U, 0U
};
static const uint32_t kUnitStartUpFlashModeOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 3U, 18U, 0U
};
static const uint32_t kUnitMceTimeoutOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 3U, 15U, 0U
};
static const uint32_t kUnitMceIntAdvOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 3U, 16U, 0U
};
static const uint32_t kAscElevationOffsetOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 3U, 17U, 0U
};
static const uint32_t kUnitUserDefinedBackupTimeOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 3U, 19U, 0U
};
static const uint32_t kMaxUserDefinedBackupTimeContentOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 3U, 20U, 0U
};
static const uint32_t kUnitUserDefinedBackupTimeContentNumberOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 3U, 21U, 1U, 1U, 1U
};
static const uint32_t kUnitUserDefinedBackupTimeContentOidOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 3U, 21U, 1U, 2U, 1U
};
static const uint32_t kUnitUserDefinedBackupTimeContentDescriptionOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 3U, 21U, 1U, 3U, 1U
};
static const uint32_t kMaxGlobalSetIdsOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 3U, 24U, 0U
};
static const uint32_t kGlobalSetIdNumberOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 3U, 25U, 1U, 1U, 1U
};
static const uint32_t kGlobalSetIdOidOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 3U, 25U, 1U, 2U, 1U
};
static const uint32_t kUnitAlarmStatus4Oid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 3U, 27U, 0U
};
static const uint32_t kMaxSpecialFunctionOutputsOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 3U, 13U, 0U
};
static const uint32_t kSpecialFunctionOutputNumberOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 3U, 14U, 1U, 1U, 1U
};
static const uint32_t kSpecialFunctionOutputStateOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 3U, 14U, 1U, 2U, 1U
};
static const uint32_t kSpecialFunctionOutputControlOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 3U, 14U, 1U, 3U, 1U
};
static const uint32_t kSpecialFunctionOutputStatusOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 3U, 14U, 1U, 4U, 1U
};
static const uint32_t kUnitAlarmStatus3Oid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 3U, 26U, 0U
};
static const uint32_t kSystemPatternControlOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 4U, 14U, 0U
};
static const uint32_t kSystemSyncControlOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 4U, 15U, 0U
};
static const uint32_t kTimebaseAscPatternSyncOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 5U, 1U, 0U
};
static const uint32_t kMaxTimebaseAscActionsOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 5U, 2U, 0U
};
static const uint32_t kTimebaseAscPatternOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 5U, 3U, 1U, 2U, 1U
};
static const uint32_t kTimebaseAscAuxFunctionOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 5U, 3U, 1U, 3U, 1U
};
static const uint32_t kTimebaseAscActionStatusOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 5U, 4U, 0U
};
static const uint32_t kActionPlanControlOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 5U, 5U, 0U
};
static const uint32_t kMaxPreemptsOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 6U, 1U, 0U
};
static const uint32_t kPreemptControlOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 6U, 2U, 1U, 2U, 1U
};
static const uint32_t kPreemptLinkOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 6U, 2U, 1U, 3U, 1U
};
static const uint32_t kPreemptMinimumDurationOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 6U, 2U, 1U, 5U, 1U
};
static const uint32_t kPreemptMinimumGreenOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 6U, 2U, 1U, 6U, 1U
};
static const uint32_t kPreemptTrackGreenOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 6U, 2U, 1U, 9U, 1U
};
static const uint32_t kPreemptTrackPhaseOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 6U, 2U, 1U, 12U, 1U
};
static const uint32_t kPreemptStateOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 6U, 2U, 1U, 16U, 1U
};
static const uint32_t kPreemptEnterYellowChangeOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 6U, 2U, 1U, 22U, 1U
};
static const uint32_t kPreemptEnterRedClearOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 6U, 2U, 1U, 23U, 1U
};
static const uint32_t kPreemptSequenceNumberOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 6U, 2U, 1U, 26U, 1U
};
static const uint32_t kPreemptExitTypeOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 6U, 2U, 1U, 28U, 1U
};
static const uint32_t kPreemptControlStateOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 6U, 3U, 1U, 2U, 1U
};
static const uint32_t kPreemptStatusOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 6U, 4U, 0U
};
static const uint32_t kPreemptStatusGroupOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 6U, 6U, 1U, 2U, 1U
};
static const uint32_t kPreemptDetectorWeightOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 6U, 7U, 1U, 1U, 1U, 1U
};
static const uint32_t kMaxPreemptGatesOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 6U, 8U, 0U
};
static const uint32_t kPreemptGateStatusOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 6U, 9U, 1U, 2U, 1U
};
static const uint32_t kPreemptGateDescriptionOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 6U, 9U, 1U, 3U, 1U
};
static const uint32_t kCpMpDiagPeerHealthyOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 59748U, 1U, 2U, 0U
};
static const uint32_t kCpMpDiagSafetyActionOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 59748U, 1U, 5U, 0U
};
static const uint32_t kCpMpDiagSafetyReasonCodeOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 59748U, 1U, 6U, 0U
};
static const uint32_t kCpMpDiagGlobalFlagsOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 59748U, 1U, 8U, 0U
};
static const uint32_t kCpMpDiagChannelFlagsOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 59748U, 1U, 10U, 1U, 2U, 1U
};

enum
{
  TEST_CPMP_REASON_CONFLICT = 1U,
  TEST_CPMP_REASON_CONFIG_INVALID = 53U
};

static MockConfigRepositoryAdapterCtx_t s_repoCtx;
static IConfigRepositoryPort_t s_repoPort;
static ConfigurationService_t s_configurationService;
static IntersectionEngine_t s_engine;
static IntersectionController_t s_controller;
static CpMpLinkService_t s_cpMpLinkService;
static IModuleBusPort_t s_moduleBusPort;
static NtcipDbTransactionService_t s_dbTransactionService;
static NtcipObjectDirectory_t s_directory;
static NtcipContext_t s_ntcipContext;
static MockUnitAlarmAdapterCtx_t s_unitAlarmCtx;
static IUnitAlarmPort_t s_unitAlarmPort;

typedef struct
{
  uint8_t resetCount;
  ModuleBusDetectorClass_t lastResetClass;
  uint8_t lastResetDetectorNumber;
  uint8_t resetResult;
} FakeModuleBusCommandCtx_t;

static FakeModuleBusCommandCtx_t s_moduleBusCommandCtx;

static void ReplicateSequencePlansFromBase(IntersectionConfig_t *config)
{
  uint8_t sequenceIndex;
  uint8_t ringIndex;

  if (config == NULL)
  {
    return;
  }

  for (sequenceIndex = 1U;
       sequenceIndex < INTERSECTION_SEQUENCE_COUNT_MAX;
       sequenceIndex++)
  {
    for (ringIndex = 0U; ringIndex < INTERSECTION_RING_COUNT_MAX; ringIndex++)
    {
      config->sequencePlans[sequenceIndex][ringIndex] = config->rings[ringIndex];
    }
  }
}

static IntersectionConfig_t MakeTwoPhasePerRingConfig(void)
{
  IntersectionConfig_t config;
  const uint8_t overlapIncluded[] = { 1U, 2U };
  uint8_t phaseIndex;
  uint8_t detectorIndex;

  IntersectionConfigInitDefaults(&config);
  config.phaseCount = 4U;
  config.ringCount = 2U;
  config.barrierCount = 2U;

  for (phaseIndex = 0U; phaseIndex < config.phaseCount; phaseIndex++)
  {
    config.phases[phaseIndex].phaseOptions = PHASE_OPTIONS_ENABLED;
    config.phases[phaseIndex].ring = (phaseIndex < 2U) ? 0U : 1U;
  }

  config.rings[0].phaseCount = 2U;
  config.rings[0].barrierPhaseCount = 1U;
  config.rings[0].phaseOrder[0] = 0U;
  config.rings[0].phaseOrder[1] = 1U;
  config.rings[1].phaseCount = 2U;
  config.rings[1].barrierPhaseCount = 1U;
  config.rings[1].phaseOrder[0] = 2U;
  config.rings[1].phaseOrder[1] = 3U;
  ReplicateSequencePlansFromBase(&config);

  config.channels[0].controlSource = 1U;
  config.channels[0].controlType =
    INTERSECTION_CHANNEL_CONTROL_TYPE_PHASE_VEHICLE;
  config.channels[1].controlSource = 1U;
  config.channels[1].controlType =
    INTERSECTION_CHANNEL_CONTROL_TYPE_PHASE_PEDESTRIAN;
  config.channels[2].controlSource = 1U;
  config.channels[2].controlType = INTERSECTION_CHANNEL_CONTROL_TYPE_OVERLAP;
  config.overlaps[0].type = INTERSECTION_OVERLAP_TYPE_NORMAL;
  config.overlaps[0].includedPhases.length = 2U;
  config.overlaps[0].includedPhases.values[0] = overlapIncluded[0];
  config.overlaps[0].includedPhases.values[1] = overlapIncluded[1];

  for (detectorIndex = config.phaseCount;
       detectorIndex < INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX;
       detectorIndex++)
  {
    config.vehicleDetectors[detectorIndex].callPhase = 0U;
  }

  for (detectorIndex = config.phaseCount;
       detectorIndex < INTERSECTION_PED_INPUT_COUNT_MAX;
       detectorIndex++)
  {
    config.pedestrianDetectors[detectorIndex].callPhase = 0U;
  }

  return config;
}

static uint8_t FakeModuleBusRead(void *ctx, ModuleBusSnapshot_t *snapshot)
{
  (void) ctx;
  (void) snapshot;

  return 0U;
}

static uint8_t FakeModuleBusCommandDetectorReset(
  void *ctx,
  ModuleBusDetectorClass_t detectorClass,
  uint8_t detectorNumber)
{
  FakeModuleBusCommandCtx_t *commandCtx = (FakeModuleBusCommandCtx_t *) ctx;

  if (commandCtx == NULL)
  {
    return 0U;
  }

  commandCtx->resetCount++;
  commandCtx->lastResetClass = detectorClass;
  commandCtx->lastResetDetectorNumber = detectorNumber;

  return commandCtx->resetResult;
}

static void ReloadEngine(void)
{
  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(
                     &s_engine,
                     ConfigurationServiceGetActiveConfig(
                       &s_configurationService)));
  IntersectionEngineTick(&s_engine);
}

static void BeginTransaction(NtcipRequestContext_t *request,
                             uint8_t transactionId)
{
  NtcipValue_t value;

  NtcipValueSetUnsigned32(&value, (uint32_t) NTCIP_DB_CREATE_STATE_TRANSACTION);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kDbCreateTransactionOid,
                                                     13U,
                                                     request,
                                                     &value));
  NtcipValueSetUnsigned32(&value, transactionId);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kDbTransactionIdOid,
                                                     13U,
                                                     request,
                                                     &value));
  request->transactionIdValid = 1U;
  request->transactionId = transactionId;
}

static void VerifyAndCommitTransaction(NtcipRequestContext_t *request)
{
  NtcipValue_t value;

  NtcipValueSetUnsigned32(&value, (uint32_t) NTCIP_DB_CREATE_STATE_VERIFY);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kDbCreateTransactionOid,
                                                     13U,
                                                     request,
                                                     &value));
  NtcipValueSetUnsigned32(&value, (uint32_t) NTCIP_DB_CREATE_STATE_NORMAL);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kDbCreateTransactionOid,
                                                     13U,
                                                     request,
                                                     &value));
  ReloadEngine();
}

void setUp(void)
{
  MockConfigRepositoryAdapterInit(&s_repoCtx);
  s_repoPort = MockConfigRepositoryAdapterCreatePort(&s_repoCtx);
  ConfigurationServiceInit(&s_configurationService, &s_repoPort);
  IntersectionEngineInit(&s_engine);
  IntersectionControllerInit(&s_controller);
  memset(&s_moduleBusCommandCtx, 0, sizeof(s_moduleBusCommandCtx));
  s_moduleBusCommandCtx.resetResult = 1U;
  s_moduleBusPort.ctx = &s_moduleBusCommandCtx;
  s_moduleBusPort.ReadSnapshot = FakeModuleBusRead;
  s_moduleBusPort.CommandDetectorReset = FakeModuleBusCommandDetectorReset;
  s_controller.moduleBusPort = &s_moduleBusPort;
  NtcipDbTransactionServiceInit(&s_dbTransactionService,
                                &s_configurationService);
  NtcipContextInit(&s_ntcipContext,
                   &s_configurationService,
                   &s_engine,
                   &s_controller,
                   &s_dbTransactionService);
  MockUnitAlarmAdapterInit(&s_unitAlarmCtx);
  s_unitAlarmPort = MockUnitAlarmAdapterCreatePort(&s_unitAlarmCtx);
  NtcipContextBindUnitAlarmPort(&s_ntcipContext, &s_unitAlarmPort);
  NtcipObjectDirectoryInit(&s_directory);
  Ntcip1201RegisterObjects(&s_directory, &s_ntcipContext);
  Ntcip1202RegisterObjects(&s_directory, &s_ntcipContext);
  CpMpDiagnosticsObjectsRegister(&s_directory, &s_ntcipContext);
  ReloadEngine();
}

static void PrimeCpMpLinkService(CpMpSafetyAction_t safetyAction,
                                 uint8_t safetyReasonCode,
                                 uint32_t globalFlags,
                                 uint16_t channel0Flags)
{
  (void) memset(&s_cpMpLinkService, 0, sizeof(s_cpMpLinkService));
  s_cpMpLinkService.configurationService = &s_configurationService;
  s_cpMpLinkService.controller = &s_controller;
  s_cpMpLinkService.tickCount = 10U;
  s_cpMpLinkService.lastMpHeartbeatTick = 10U;
  s_cpMpLinkService.lastMpHeartbeatSeen = 1U;
  s_cpMpLinkService.configGeneration =
    ConfigurationServiceGetActiveGeneration(&s_configurationService);
  s_cpMpLinkService.configSetId =
    ConfigurationServiceGetActiveSetId(&s_configurationService);
  s_cpMpLinkService.lastMpConfigGeneration = s_cpMpLinkService.configGeneration;
  s_cpMpLinkService.lastMpConfigSetId = s_cpMpLinkService.configSetId;
  s_cpMpLinkService.lastMpConfigState = CPMP_CONFIG_STATE_APPLIED;
  s_cpMpLinkService.lastSafetyAction = safetyAction;
  s_cpMpLinkService.lastSafetyReasonCode = safetyReasonCode;
  s_cpMpLinkService.lastFaultStatusValid = 1U;
  s_cpMpLinkService.lastFaultStatus.sequence = 0x11223344UL;
  s_cpMpLinkService.lastFaultStatus.globalFlags = globalFlags;
  s_cpMpLinkService.lastFaultStatus.channelFlags[0] = channel0Flags;
  s_cpMpLinkService.lastFaultStatus.safetyAction = (uint8_t) safetyAction;
  s_cpMpLinkService.lastFaultStatus.safetyReasonCode = safetyReasonCode;
  s_cpMpLinkService.lastFaultStatus.configState = CPMP_CONFIG_STATE_APPLIED;
  NtcipContextBindCpMpLinkService(&s_ntcipContext, &s_cpMpLinkService);
}

void tearDown(void)
{
}

void test_directory_routes_phase_minimum_green_oid_in_mib_units(void)
{
  NtcipValue_t value;

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kPhaseMinimumGreenOid,
                                                15U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_INT(NTCIP_VALUE_TYPE_UNSIGNED32, value.type);
  TEST_ASSERT_EQUAL_UINT32(5U, value.data.unsigned32);
}

void test_candidate_changes_stay_hidden_until_transaction_commits(void)
{
  NtcipRequestContext_t request = { 0x1111U, 0U, 0U };
  NtcipValue_t value;

  BeginTransaction(&request, 9U);

  NtcipValueSetUnsigned32(&value, 13U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kPhaseMinimumGreenOid,
                                                     15U,
                                                     &request,
                                                     &value));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kPhaseMaximumInitialOid,
                                                     15U,
                                                     &request,
                                                     &value));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kPhaseMinimumGreenOid,
                                                15U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(5U, value.data.unsigned32);

  VerifyAndCommitTransaction(&request);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kPhaseMinimumGreenOid,
                                                15U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(13U, value.data.unsigned32);
}

void test_phase_walk_is_transactional_and_committed_in_seconds(void)
{
  NtcipRequestContext_t request = { 0x1212U, 0U, 0U };
  NtcipValue_t value;

  BeginTransaction(&request, 12U);
  NtcipValueSetUnsigned32(&value, 9U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kPhaseWalkOid,
                                                     15U,
                                                     &request,
                                                     &value));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kPhaseWalkOid,
                                                15U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(7U, value.data.unsigned32);

  VerifyAndCommitTransaction(&request);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kPhaseWalkOid,
                                                15U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(9U, value.data.unsigned32);
}

void test_database_writes_require_owner_and_matching_transaction_id(void)
{
  NtcipRequestContext_t ownerRequest = { 0x2222U, 0U, 0U };
  NtcipRequestContext_t otherRequest = { 0x3333U, 1U, 7U };
  NtcipValue_t value;

  BeginTransaction(&ownerRequest, 7U);

  NtcipValueSetUnsigned32(&value, 14U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OWNER_MISMATCH,
                        NtcipObjectDirectorySetTest(&s_directory,
                                                    kPhaseMinimumGreenOid,
                                                    15U,
                                                    &otherRequest,
                                                    &value));
  ownerRequest.transactionIdValid = 0U;
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_TRANSACTION_ID_MISMATCH,
                        NtcipObjectDirectorySetTest(&s_directory,
                                                    kPhaseMinimumGreenOid,
                                                    15U,
                                                    &ownerRequest,
                                                    &value));
}

void test_phase_ring_zero_disables_phase_and_auxio_group_is_present(void)
{
  NtcipRequestContext_t request = { 0x4444U, 0U, 0U };
  NtcipValue_t value;

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kMaxAuxIoDigitalOid,
                                                13U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(0U, value.data.unsigned32);

  BeginTransaction(&request, 21U);

  NtcipValueSetUnsigned32(&value, 0U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kPhaseRingOid,
                                                     15U,
                                                     &request,
                                                     &value));

  VerifyAndCommitTransaction(&request);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kPhaseRingOid,
                                                15U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(0U, value.data.unsigned32);
}

void test_global_set_id_changes_after_committed_configuration_change(void)
{
  NtcipRequestContext_t request = { 0x5555U, 0U, 0U };
  NtcipValue_t before;
  NtcipValue_t after;

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kGlobalSetIdParameterOid,
                                                13U,
                                                NULL,
                                                &before));

  BeginTransaction(&request, 33U);

  NtcipValueSetUnsigned32(&after, 31U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kPhasePassageOid,
                                                     15U,
                                                     &request,
                                                     &after));

  VerifyAndCommitTransaction(&request);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kGlobalSetIdParameterOid,
                                                13U,
                                                NULL,
                                                &after));

  TEST_ASSERT_NOT_EQUAL(before.data.unsigned32, after.data.unsigned32);
}

void test_phase_status_group_reads_runtime_from_engine(void)
{
  NtcipValue_t value;

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kPhaseStatusGroupGreensOid,
                                                14U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(0x11U, value.data.unsigned32);
}

void test_channel_and_overlap_runtime_status_follow_committed_configuration(
  void)
{
  NtcipRequestContext_t request = { 0x6666U, 0U, 0U };
  NtcipValue_t value;
  const uint8_t overlapIncluded[] = { 1U, 2U };

  BeginTransaction(&request, 41U);

  NtcipValueSetUnsigned32(&value,
                          INTERSECTION_CHANNEL_CONTROL_TYPE_PHASE_VEHICLE);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kChannelControlTypeOid,
                                                     15U,
                                                     &request,
                                                     &value));
  NtcipValueSetUnsigned32(&value, 1U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kChannelControlSourceOid,
                                                     15U,
                                                     &request,
                                                     &value));

  NtcipValueSetUnsigned32(&value, INTERSECTION_OVERLAP_TYPE_NORMAL);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kOverlapTypeOid,
                                                     15U,
                                                     &request,
                                                     &value));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipValueSetOctetString(&value,
                                                 overlapIncluded,
                                                 sizeof(overlapIncluded)));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kOverlapIncludedPhasesOid,
                                                     15U,
                                                     &request,
                                                     &value));

  VerifyAndCommitTransaction(&request);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kChannelStatusGroupGreensOid,
                                                15U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(0x01U, value.data.unsigned32);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kOverlapStatusGroupGreensOid,
                                                15U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(0x01U, value.data.unsigned32);
} /* test_channel_and_overlap_runtime_status_follow_committed_configuration */

void test_ped_status_and_phase_pedestrian_channel_follow_runtime_service(void)
{
  NtcipRequestContext_t request = { 0x7777U, 0U, 0U };
  NtcipValue_t value;

  BeginTransaction(&request, 51U);
  NtcipValueSetUnsigned32(&value,
                          INTERSECTION_CHANNEL_CONTROL_TYPE_PHASE_PEDESTRIAN);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kChannelControlTypeOid,
                                                     15U,
                                                     &request,
                                                     &value));
  NtcipValueSetUnsigned32(&value, 1U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kChannelControlSourceOid,
                                                     15U,
                                                     &request,
                                                     &value));
  NtcipValueSetUnsigned32(&value, 1U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kPhaseWalkOid,
                                                     15U,
                                                     &request,
                                                     &value));

  VerifyAndCommitTransaction(&request);

  TEST_ASSERT_TRUE(IntersectionEngineSetPedCall(&s_engine, 1U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetPedCall(&s_engine, 1U, 0U));
  IntersectionEngineTick(&s_engine);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kPhaseStatusGroupWalksOid,
                                                14U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(0x01U, value.data.unsigned32);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kPhaseStatusGroupPedCallsOid,
                                                14U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(0x00U, value.data.unsigned32);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kChannelStatusGroupGreensOid,
                                                15U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(0x01U, value.data.unsigned32);

  {
    uint32_t tickIndex;

    for (tickIndex = 0U; tickIndex < 100U; tickIndex++)
    {
      IntersectionEngineTick(&s_engine);
    }
  }

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kChannelStatusGroupYellowsOid,
                                                15U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(0x01U, value.data.unsigned32);
} /* test_ped_status_and_phase_pedestrian_channel_follow_runtime_service */

void test_phase_control_group_objects_drive_runtime_controls_and_force_off_clear(
  void)
{
  NtcipValue_t value;

  NtcipValueSetUnsigned32(&value, 0x02U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kPhaseControlGroupVehCallOid,
                                                     15U,
                                                     NULL,
                                                     &value));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kPhaseStatusGroupVehCallsOid,
                                                14U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(0x02U, value.data.unsigned32);

  NtcipValueSetUnsigned32(&value, 0x01U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kPhaseControlGroupPedCallOid,
                                                     15U,
                                                     NULL,
                                                     &value));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kPhaseStatusGroupPedCallsOid,
                                                14U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(0x01U, value.data.unsigned32);
  NtcipValueSetUnsigned32(&value, 0x00U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kPhaseControlGroupPedCallOid,
                                                     15U,
                                                     NULL,
                                                     &value));

  NtcipValueSetUnsigned32(&value, 0x02U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kPhaseControlGroupPhaseOmitOid,
                                                     15U,
                                                     NULL,
                                                     &value));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kPhaseControlGroupPhaseOmitOid,
                                                15U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(0x02U, value.data.unsigned32);

  NtcipValueSetUnsigned32(&value, 0x01U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kPhaseControlGroupHoldOid,
                                                     15U,
                                                     NULL,
                                                     &value));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kPhaseControlGroupHoldOid,
                                                15U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(0x01U, value.data.unsigned32);

  NtcipValueSetUnsigned32(&value, 0x00U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kPhaseControlGroupHoldOid,
                                                     15U,
                                                     NULL,
                                                     &value));

  NtcipValueSetUnsigned32(&value, 0x01U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kPhaseControlGroupForceOffOid,
                                                     15U,
                                                     NULL,
                                                     &value));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kPhaseControlGroupForceOffOid,
                                                15U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(0x01U, value.data.unsigned32);

  {
    uint32_t tickIndex;

    for (tickIndex = 0U; tickIndex < 510U; tickIndex++)
    {
      IntersectionEngineTick(&s_engine);
    }
  }

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kPhaseControlGroupForceOffOid,
                                                15U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(0x00U, value.data.unsigned32);
} /* test_phase_control_group_objects_drive_runtime_controls_and_force_off_clear */

void test_ring_control_group_objects_route_runtime_masks_and_validate_range(
  void)
{
  NtcipValue_t value;

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kMaxRingControlGroupsOid,
                                                13U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(1U, value.data.unsigned32);

  NtcipValueSetUnsigned32(&value, 0x01U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kRingControlGroupStopTimeOid,
                                                     15U,
                                                     NULL,
                                                     &value));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kRingControlGroupStopTimeOid,
                                                15U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(0x01U, value.data.unsigned32);

  NtcipValueSetUnsigned32(&value, 0x01U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kRingControlGroupMax2Oid,
                                                     15U,
                                                     NULL,
                                                     &value));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kRingControlGroupMax2Oid,
                                                15U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(0x01U, value.data.unsigned32);

  NtcipValueSetUnsigned32(&value, 0x03U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kRingControlGroupRedRestOid,
                                                     15U,
                                                     NULL,
                                                     &value));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kRingControlGroupRedRestOid,
                                                15U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(0x03U, value.data.unsigned32);

  NtcipValueSetUnsigned32(&value, 0x04U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_RANGE_ERROR,
                        NtcipObjectDirectorySetTest(&s_directory,
                                                    kRingControlGroupStopTimeOid,
                                                    15U,
                                                    NULL,
                                                    &value));
}

void test_ring_control_group_force_off_mask_drives_engine_runtime(void)
{
  NtcipValue_t value;
  uint32_t tickIndex;

  NtcipValueSetUnsigned32(&value, 0x01U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kRingControlGroupForceOffOid,
                                                     15U,
                                                     NULL,
                                                     &value));

  for (tickIndex = 0U; tickIndex < 700U; tickIndex++)
  {
    if (IntersectionEngineGetRuntime(&s_engine)->phases[0].interval
        == INTERSECTION_PHASE_INTERVAL_YELLOW)
    {
      break;
    }

    IntersectionEngineTick(&s_engine);
  }

  TEST_ASSERT_EQUAL_INT(INTERSECTION_PHASE_INTERVAL_YELLOW,
                        IntersectionEngineGetRuntime(&s_engine)->phases[0].interval);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kRingControlGroupForceOffOid,
                                                15U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(0x01U, value.data.unsigned32);
}

void test_sequence_table_reports_supported_sequence_count_and_commits_reordered_ring(
  void)
{
  NtcipRequestContext_t request = { 0x7272U, 0U, 0U };
  NtcipValue_t value;
  static const uint8_t defaultSequence[] = { 1U, 2U, 3U, 4U };
  static const uint8_t reorderedSequence[] = { 2U, 1U, 3U, 4U };
  static const uint8_t duplicateSequence[] = { 2U, 2U, 3U, 4U };

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kMaxSequencesOid,
                                                13U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(INTERSECTION_SEQUENCE_COUNT_MAX,
                           value.data.unsigned32);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kSequenceNumberOid,
                                                16U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(1U, value.data.unsigned32);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kSequenceRingNumberOid,
                                                16U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(1U, value.data.unsigned32);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kSequenceDataOid,
                                                16U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_INT(NTCIP_VALUE_TYPE_OCTET_STRING, value.type);
  TEST_ASSERT_EQUAL_UINT16(sizeof(defaultSequence),
                           value.data.octetString.length);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(defaultSequence,
                                value.data.octetString.bytes,
                                value.data.octetString.length);

  BeginTransaction(&request, 54U);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipValueSetOctetString(&value,
                                                 duplicateSequence,
                                                 sizeof(duplicateSequence)));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_RANGE_ERROR,
                        NtcipObjectDirectorySetTest(&s_directory,
                                                    kSequenceDataOid,
                                                    16U,
                                                    &request,
                                                    &value));

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipValueSetOctetString(&value,
                                                 reorderedSequence,
                                                 sizeof(reorderedSequence)));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetTest(&s_directory,
                                                    kSequenceDataOid,
                                                    16U,
                                                    &request,
                                                    &value));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kSequenceDataOid,
                                                     16U,
                                                     &request,
                                                     &value));

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kSequenceDataOid,
                                                16U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT8_ARRAY(defaultSequence,
                                value.data.octetString.bytes,
                                value.data.octetString.length);

  VerifyAndCommitTransaction(&request);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kSequenceDataOid,
                                                16U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT8_ARRAY(reorderedSequence,
                                value.data.octetString.bytes,
                                value.data.octetString.length);
  TEST_ASSERT_EQUAL_UINT8(1U,
                          IntersectionEngineGetRuntime(&s_engine)->rings[0].
                          activePhaseIndex);
}

void test_detector_objects_route_transactional_call_phase_and_runtime_status(
  void)
{
  NtcipRequestContext_t request = { 0x7878U, 0U, 0U };
  NtcipValue_t value;

  BeginTransaction(&request, 52U);

  NtcipValueSetUnsigned32(&value, 2U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kVehicleDetectorCallPhaseOid,
                                                     15U,
                                                     &request,
                                                     &value));
  NtcipValueSetUnsigned32(&value, 3U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(
                          &s_directory,
                          kPedestrianDetectorCallPhaseOid,
                          15U,
                          &request,
                          &value));

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kVehicleDetectorCallPhaseOid,
                                                15U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(1U, value.data.unsigned32);

  VerifyAndCommitTransaction(&request);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kVehicleDetectorCallPhaseOid,
                                                15U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(2U, value.data.unsigned32);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kPedestrianDetectorCallPhaseOid,
                                                15U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(3U, value.data.unsigned32);

  s_controller.lastSnapshotValid = 1U;
  s_controller.lastSnapshot = (ModuleBusSnapshot_t) {
    MODULE_BUS_PROTOCOL_VERSION,
    (uint8_t) (MODULE_BUS_SNAPSHOT_VALID_DETECTORS
               | MODULE_BUS_SNAPSHOT_VALID_PEDS),
    (uint8_t) (MODULE_BUS_SNAPSHOT_VALID_DETECTORS
               | MODULE_BUS_SNAPSHOT_VALID_PEDS),
    0U,
    0x0081U,
    0x81U,
    0U,
    0U,
    0U,
    0U,
    0U,
    0U,
    0U,
    0U
  };

  TEST_ASSERT_TRUE(IntersectionEngineSetVehicleDetectorInput(&s_engine, 1U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetVehicleDetectorInput(&s_engine, 8U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetPedestrianDetectorInput(&s_engine,
                                                                1U,
                                                                1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetPedestrianDetectorInput(&s_engine,
                                                                8U,
                                                                1U));
  {
    uint8_t statusGroup = 0U;

    TEST_ASSERT_TRUE(IntersectionEngineGetVehicleDetectorStatusGroup(&s_engine,
                                                                     1U,
                                                                     &statusGroup));
    TEST_ASSERT_EQUAL_UINT8(0x81U, statusGroup);
    TEST_ASSERT_TRUE(IntersectionEngineGetPedestrianDetectorStatusGroup(
                       &s_engine,
                       1U,
                       &statusGroup));
    TEST_ASSERT_EQUAL_UINT8(0x81U, statusGroup);
  }

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(
                          &s_directory,
                          kVehicleDetectorStatusGroupActiveOid,
                          15U,
                          NULL,
                          &value));
  TEST_ASSERT_EQUAL_UINT32(0x81U, value.data.unsigned32);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(
                          &s_directory,
                          kPedestrianDetectorStatusGroupActiveOid,
                          15U,
                          NULL,
                          &value));
  TEST_ASSERT_EQUAL_UINT32(0x81U, value.data.unsigned32);

  s_controller.lastSnapshot.healthMask = 0U;
  s_controller.lastSnapshot.staleMask =
    (uint8_t) (MODULE_BUS_SNAPSHOT_VALID_DETECTORS
               | MODULE_BUS_SNAPSHOT_VALID_PEDS);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kVehicleDetectorAlarmsOid,
                                                15U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(0x08U, value.data.unsigned32);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kPedestrianDetectorAlarmsOid,
                                                15U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(0x08U, value.data.unsigned32);
} /* test_detector_objects_route_transactional_call_phase_and_runtime_status */

void test_detector_alarm_objects_include_module_bus_diagnostics_and_reported_alarm_bytes(
  void)
{
  NtcipValue_t value;

  s_controller.lastSnapshotValid = 1U;
  s_controller.lastSnapshot = (ModuleBusSnapshot_t) {
    MODULE_BUS_PROTOCOL_VERSION,
    (uint8_t) (MODULE_BUS_SNAPSHOT_VALID_DETECTORS
               | MODULE_BUS_SNAPSHOT_VALID_PEDS
               | MODULE_BUS_SNAPSHOT_VALID_DETECTOR_DIAGNOSTICS),
    (uint8_t) (MODULE_BUS_SNAPSHOT_VALID_DETECTORS
               | MODULE_BUS_SNAPSHOT_VALID_PEDS
               | MODULE_BUS_SNAPSHOT_VALID_DETECTOR_DIAGNOSTICS),
    0U,
    0U,
    0U,
    0U,
    0U,
    0U,
    0U,
    0U,
    0U,
    0U
  };
  s_controller.lastSnapshot.vehicleDetectorAlarms[0] = 0x04U;
  s_controller.lastSnapshot.vehicleDetectorReportedAlarms[0] = 0x05U;
  s_controller.lastSnapshot.pedestrianDetectorAlarms[0] = 0x02U;

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kVehicleDetectorAlarmsOid,
                                                15U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(0x04U, value.data.unsigned32);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kVehicleDetectorReportedAlarmsOid,
                                                15U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(0x05U, value.data.unsigned32);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kPedestrianDetectorAlarmsOid,
                                                15U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(0x02U, value.data.unsigned32);

  s_controller.lastSnapshot.healthMask =
    MODULE_BUS_SNAPSHOT_VALID_DETECTOR_DIAGNOSTICS;

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kVehicleDetectorAlarmsOid,
                                                15U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(0x0CU, value.data.unsigned32);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kPedestrianDetectorAlarmsOid,
                                                15U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(0x0AU, value.data.unsigned32);
}

void test_detector_objects_expose_full_committed_config_and_pair_reciprocity(
  void)
{
  NtcipRequestContext_t request = { 0x7979U, 0U, 0U };
  NtcipValue_t value;

  BeginTransaction(&request, 53U);

  NtcipValueSetUnsigned32(&value, 0x8CU);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kVehicleDetectorOptionsOid,
                                                     15U,
                                                     &request,
                                                     &value));
  NtcipValueSetUnsigned32(&value, 2U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(
                          &s_directory,
                          kVehicleDetectorSwitchPhaseOid,
                          15U,
                          &request,
                          &value));
  NtcipValueSetUnsigned32(&value, 2500U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kVehicleDetectorDelayOid,
                                                     15U,
                                                     &request,
                                                     &value));
  NtcipValueSetUnsigned32(&value, 7U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kVehicleDetectorExtendOid,
                                                     15U,
                                                     &request,
                                                     &value));
  NtcipValueSetUnsigned32(&value, 12U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(
                          &s_directory,
                          kVehicleDetectorQueueLimitOid,
                          15U,
                          &request,
                          &value));
  NtcipValueSetUnsigned32(&value, 13U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(
                          &s_directory,
                          kVehicleDetectorNoActivityOid,
                          15U,
                          &request,
                          &value));
  NtcipValueSetUnsigned32(&value, 14U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(
                          &s_directory,
                          kVehicleDetectorMaxPresenceOid,
                          15U,
                          &request,
                          &value));
  NtcipValueSetUnsigned32(&value, 15U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(
                          &s_directory,
                          kVehicleDetectorErraticCountsOid,
                          15U,
                          &request,
                          &value));
  NtcipValueSetUnsigned32(&value, 16U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kVehicleDetectorFailTimeOid,
                                                     15U,
                                                     &request,
                                                     &value));
  NtcipValueSetUnsigned32(&value,
                          (uint32_t) (VEHICLE_DETECTOR_OPTIONS2_SPEED_ENABLED
                                      | VEHICLE_DETECTOR_OPTIONS2_PLACEMENT_LEAD
                                      | VEHICLE_DETECTOR_OPTIONS2_SPEED_NTCIP));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kVehicleDetectorOptions2Oid,
                                                     15U,
                                                     &request,
                                                     &value));
  NtcipValueSetUnsigned32(&value, 2U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(
                          &s_directory,
                          kVehicleDetectorPairedDetectorOid,
                          15U,
                          &request,
                          &value));
  NtcipValueSetUnsigned32(&value, 321U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(
                          &s_directory,
                          kVehicleDetectorPairedDetectorSpacingOid,
                          15U,
                          &request,
                          &value));
  NtcipValueSetUnsigned32(&value, 654U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(
                          &s_directory,
                          kVehicleDetectorAvgVehicleLengthOid,
                          15U,
                          &request,
                          &value));
  NtcipValueSetUnsigned32(&value, 65535U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kVehicleDetectorLengthOid,
                                                     15U,
                                                     &request,
                                                     &value));
  NtcipValueSetUnsigned32(&value, 4U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(
                          &s_directory,
                          kVehicleDetectorTravelModeOid,
                          15U,
                          &request,
                          &value));

  NtcipValueSetUnsigned32(&value, 3U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(
                          &s_directory,
                          kPedestrianDetectorCallPhaseOid,
                          15U,
                          &request,
                          &value));
  NtcipValueSetUnsigned32(&value, 17U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(
                          &s_directory,
                          kPedestrianDetectorNoActivityOid,
                          15U,
                          &request,
                          &value));
  NtcipValueSetUnsigned32(&value, 18U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(
                          &s_directory,
                          kPedestrianDetectorMaxPresenceOid,
                          15U,
                          &request,
                          &value));
  NtcipValueSetUnsigned32(&value, 19U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(
                          &s_directory,
                          kPedestrianDetectorErraticCountsOid,
                          15U,
                          &request,
                          &value));
  NtcipValueSetUnsigned32(&value, 9U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(
                          &s_directory,
                          kPedestrianButtonPushTimeOid,
                          15U,
                          &request,
                          &value));
  NtcipValueSetUnsigned32(&value,
                          (uint32_t) (PED_DETECTOR_OPTIONS_ALT_TIMING
                                      | PED_DETECTOR_OPTIONS_NON_LOCKING));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(
                          &s_directory,
                          kPedestrianDetectorOptionsOid,
                          15U,
                          &request,
                          &value));

  VerifyAndCommitTransaction(&request);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kVehicleDetectorOptionsOid,
                                                15U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(0x84U, value.data.unsigned32);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kVehicleDetectorSwitchPhaseOid,
                                                15U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(2U, value.data.unsigned32);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kVehicleDetectorDelayOid,
                                                15U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(2500U, value.data.unsigned32);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kVehicleDetectorExtendOid,
                                                15U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(7U, value.data.unsigned32);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kVehicleDetectorQueueLimitOid,
                                                15U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(12U, value.data.unsigned32);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kVehicleDetectorNoActivityOid,
                                                15U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(13U, value.data.unsigned32);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kVehicleDetectorMaxPresenceOid,
                                                15U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(14U, value.data.unsigned32);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kVehicleDetectorErraticCountsOid,
                                                15U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(15U, value.data.unsigned32);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kVehicleDetectorFailTimeOid,
                                                15U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(16U, value.data.unsigned32);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kVehicleDetectorOptions2Oid,
                                                15U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(0x07U, value.data.unsigned32);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(
                          &s_directory,
                          kVehicleDetectorPairedDetectorOid,
                          15U,
                          NULL,
                          &value));
  TEST_ASSERT_EQUAL_UINT32(2U, value.data.unsigned32);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kVehicleDetectorPairedDetectorSpacingOid,
                                                15U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(321U, value.data.unsigned32);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kVehicleDetectorAvgVehicleLengthOid,
                                                15U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(654U, value.data.unsigned32);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kVehicleDetectorLengthOid,
                                                15U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(65535U, value.data.unsigned32);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kVehicleDetectorTravelModeOid,
                                                15U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(4U, value.data.unsigned32);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(
                          &s_directory,
                          kVehicleDetectorReportedAlarmsOid,
                          15U,
                          NULL,
                          &value));
  TEST_ASSERT_EQUAL_UINT32(0U, value.data.unsigned32);

  {
    static const uint32_t kVehicleDetectorTwoPairedOid[] =
    {
      1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 2U, 1U, 17U, 2U
    };

    TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                          NtcipObjectDirectoryGet(&s_directory,
                                                  kVehicleDetectorTwoPairedOid,
                                                  15U,
                                                  NULL,
                                                  &value));
    TEST_ASSERT_EQUAL_UINT32(1U, value.data.unsigned32);
  }

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(
                          &s_directory,
                          kPedestrianDetectorCallPhaseOid,
                          15U,
                          NULL,
                          &value));
  TEST_ASSERT_EQUAL_UINT32(3U, value.data.unsigned32);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(
                          &s_directory,
                          kPedestrianDetectorNoActivityOid,
                          15U,
                          NULL,
                          &value));
  TEST_ASSERT_EQUAL_UINT32(17U, value.data.unsigned32);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(
                          &s_directory,
                          kPedestrianDetectorMaxPresenceOid,
                          15U,
                          NULL,
                          &value));
  TEST_ASSERT_EQUAL_UINT32(18U, value.data.unsigned32);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(
                          &s_directory,
                          kPedestrianDetectorErraticCountsOid,
                          15U,
                          NULL,
                          &value));
  TEST_ASSERT_EQUAL_UINT32(19U, value.data.unsigned32);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kPedestrianButtonPushTimeOid,
                                                15U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(9U, value.data.unsigned32);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kPedestrianDetectorOptionsOid,
                                                15U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(0x06U, value.data.unsigned32);
} /* test_detector_objects_expose_full_committed_config_and_pair_reciprocity */

void test_detector_control_group_actuation_drives_runtime_detector_status(void)
{
  NtcipValue_t value;

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(
                          &s_directory,
                          kMaxVehicleDetectorControlGroupsOid,
                          13U,
                          NULL,
                          &value));
  TEST_ASSERT_EQUAL_UINT32(4U, value.data.unsigned32);

  NtcipValueSetUnsigned32(&value, 0x01U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(
                          &s_directory,
                          kVehicleDetectorControlGroupActuationOid,
                          15U,
                          NULL,
                          &value));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(
                          &s_directory,
                          kPedestrianDetectorControlGroupActuationOid,
                          15U,
                          NULL,
                          &value));

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(
                          &s_directory,
                          kVehicleDetectorControlGroupActuationOid,
                          15U,
                          NULL,
                          &value));
  TEST_ASSERT_EQUAL_UINT32(0x01U, value.data.unsigned32);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(
                          &s_directory,
                          kVehicleDetectorStatusGroupActiveOid,
                          15U,
                          NULL,
                          &value));
  TEST_ASSERT_EQUAL_UINT32(0x01U, value.data.unsigned32);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(
                          &s_directory,
                          kPedestrianDetectorControlGroupActuationOid,
                          15U,
                          NULL,
                          &value));
  TEST_ASSERT_EQUAL_UINT32(0x01U, value.data.unsigned32);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(
                          &s_directory,
                          kPedestrianDetectorStatusGroupActiveOid,
                          15U,
                          NULL,
                          &value));
  TEST_ASSERT_EQUAL_UINT32(0x01U, value.data.unsigned32);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kPhaseStatusGroupPedCallsOid,
                                                14U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(0x01U, value.data.unsigned32);
} /* test_detector_control_group_actuation_drives_runtime_detector_status */

void test_detector_object_validation_rejects_invalid_ranges_and_reset_is_ephemeral(
  void)
{
  NtcipRequestContext_t request = { 0x7A7AU, 0U, 0U };
  NtcipValue_t value;

  BeginTransaction(&request, 54U);

  NtcipValueSetUnsigned32(&value, 3000U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_RANGE_ERROR,
                        NtcipObjectDirectorySetTest(&s_directory,
                                                    kVehicleDetectorDelayOid,
                                                    15U,
                                                    &request,
                                                    &value));

  NtcipValueSetUnsigned32(&value, 0x08U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_RANGE_ERROR,
                        NtcipObjectDirectorySetTest(&s_directory,
                                                    kVehicleDetectorOptions2Oid,
                                                    15U,
                                                    &request,
                                                    &value));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_RANGE_ERROR,
                        NtcipObjectDirectorySetTest(&s_directory,
                                                    kPedestrianDetectorOptionsOid,
                                                    15U,
                                                    &request,
                                                    &value));

  NtcipValueSetUnsigned32(&value, 1U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_RANGE_ERROR,
                        NtcipObjectDirectorySetTest(
                          &s_directory,
                          kVehicleDetectorPairedDetectorOid,
                          15U,
                          &request,
                          &value));

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kVehicleDetectorResetOid,
                                                     15U,
                                                     NULL,
                                                     &value));
  TEST_ASSERT_EQUAL_UINT8(1U, s_moduleBusCommandCtx.resetCount);
  TEST_ASSERT_EQUAL_INT(MODULE_BUS_DETECTOR_CLASS_VEHICLE,
                        s_moduleBusCommandCtx.lastResetClass);
  TEST_ASSERT_EQUAL_UINT8(1U, s_moduleBusCommandCtx.lastResetDetectorNumber);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kVehicleDetectorResetOid,
                                                15U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(0U, value.data.unsigned32);

  NtcipValueSetUnsigned32(&value, 1U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kPedestrianDetectorResetOid,
                                                     15U,
                                                     NULL,
                                                     &value));
  TEST_ASSERT_EQUAL_UINT8(2U, s_moduleBusCommandCtx.resetCount);
  TEST_ASSERT_EQUAL_INT(MODULE_BUS_DETECTOR_CLASS_PEDESTRIAN,
                        s_moduleBusCommandCtx.lastResetClass);
  TEST_ASSERT_EQUAL_UINT8(1U, s_moduleBusCommandCtx.lastResetDetectorNumber);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kPedestrianDetectorResetOid,
                                                15U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(0U, value.data.unsigned32);
} /* test_detector_object_validation_rejects_invalid_ranges_and_reset_is_ephemeral */

void test_preempt_objects_route_transactional_config_and_runtime_status(void)
{
  NtcipRequestContext_t request = { 0x9999U, 0U, 0U };
  NtcipValue_t value;
  const uint8_t trackPhase[] = { 2U };
  static const uint8_t gateDescription[] = "Gate 1";

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kMaxPreemptsOid,
                                                13U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(8U, value.data.unsigned32);

  BeginTransaction(&request, 71U);

  NtcipValueSetUnsigned32(&value, 0x10U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kPreemptControlOid,
                                                     15U,
                                                     &request,
                                                     &value));
  NtcipValueSetUnsigned32(&value, 1U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kPreemptLinkOid,
                                                     15U,
                                                     &request,
                                                     &value));
  NtcipValueSetUnsigned32(&value, 1U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kPreemptMinimumDurationOid,
                                                     15U,
                                                     &request,
                                                     &value));
  NtcipValueSetUnsigned32(&value, 0U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kPreemptMinimumGreenOid,
                                                     15U,
                                                     &request,
                                                     &value));
  NtcipValueSetUnsigned32(&value, 1U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kPreemptTrackGreenOid,
                                                     15U,
                                                     &request,
                                                     &value));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipValueSetOctetString(&value,
                                                 trackPhase,
                                                 sizeof(trackPhase)));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kPreemptTrackPhaseOid,
                                                     15U,
                                                     &request,
                                                     &value));
  NtcipValueSetUnsigned32(&value, 1U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kPreemptSequenceNumberOid,
                                                     15U,
                                                     &request,
                                                     &value));
  NtcipValueSetUnsigned32(&value, 0U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kPreemptEnterYellowChangeOid,
                                                     15U,
                                                     &request,
                                                     &value));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kPreemptEnterRedClearOid,
                                                     15U,
                                                     &request,
                                                     &value));
  NtcipValueSetUnsigned32(&value,
                          INTERSECTION_PREEMPT_EXIT_TYPE_EXIT_PHASES);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kPreemptExitTypeOid,
                                                     15U,
                                                     &request,
                                                     &value));
  NtcipValueSetUnsigned32(&value, 450U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kPreemptDetectorWeightOid,
                                                     16U,
                                                     &request,
                                                     &value));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipValueSetOctetString(&value,
                                                 gateDescription,
                                                 sizeof(gateDescription) - 1U));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kPreemptGateDescriptionOid,
                                                     15U,
                                                     &request,
                                                     &value));

  VerifyAndCommitTransaction(&request);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kPreemptDetectorWeightOid,
                                                16U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(450U, value.data.unsigned32);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kMaxPreemptGatesOid,
                                                13U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(8U, value.data.unsigned32);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kPreemptGateDescriptionOid,
                                                15U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT16(sizeof(gateDescription) - 1U,
                           value.data.octetString.length);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(gateDescription,
                                value.data.octetString.bytes,
                                value.data.octetString.length);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kPreemptLinkOid,
                                                15U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(1U, value.data.unsigned32);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kPreemptSequenceNumberOid,
                                                15U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(1U, value.data.unsigned32);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kPreemptGateStatusOid,
                                                15U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(1U, value.data.unsigned32);

  NtcipValueSetUnsigned32(&value, 1U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kPreemptControlStateOid,
                                                     15U,
                                                     NULL,
                                                     &value));
  IntersectionEngineTick(&s_engine);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kPreemptControlStateOid,
                                                15U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(1U, value.data.unsigned32);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kPreemptStateOid,
                                                15U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(INTERSECTION_PREEMPT_STATE_TRACK_SERVICE,
                           value.data.unsigned32);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kPreemptStatusOid,
                                                13U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(1U, value.data.unsigned32);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kPreemptStatusGroupOid,
                                                15U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(0x01U, value.data.unsigned32);
} /* test_preempt_objects_route_transactional_config_and_runtime_status */

void test_coordination_pattern_cycle_time_is_transactional_and_committed(void)
{
  NtcipRequestContext_t request = { 0x8888U, 0U, 0U };
  NtcipValue_t value;

  BeginTransaction(&request, 61U);

  NtcipValueSetUnsigned32(&value, 90U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kPatternCycleTimeOid,
                                                     15U,
                                                     &request,
                                                     &value));

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kPatternCycleTimeOid,
                                                15U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(120U, value.data.unsigned32);

  VerifyAndCommitTransaction(&request);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kPatternCycleTimeOid,
                                                15U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(90U, value.data.unsigned32);
}

void test_system_pattern_and_sync_control_drive_runtime_coord_status(void)
{
  NtcipValue_t value;

  NtcipValueSetUnsigned32(&value, 15U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kSystemSyncControlOid,
                                                     13U,
                                                     NULL,
                                                     &value));
  NtcipValueSetUnsigned32(&value, 1U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kSystemPatternControlOid,
                                                     13U,
                                                     NULL,
                                                     &value));

  IntersectionEngineTick(&s_engine);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kCoordPatternStatusOid,
                                                13U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(1U, value.data.unsigned32);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kLocalFreeStatusOid,
                                                13U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(INTERSECTION_LOCAL_FREE_STATUS_NOT_FREE,
                           value.data.unsigned32);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kCoordSyncStatusOid,
                                                13U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(15U, value.data.unsigned32);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kCoordCycleStatusOid,
                                                13U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(104U, value.data.unsigned32);
} /* test_system_pattern_and_sync_control_drive_runtime_coord_status */

void test_unit_status_objects_project_runtime_and_module_bus_health(void)
{
  NtcipValue_t value;

  NtcipValueSetUnsigned32(&value, 1U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kSystemPatternControlOid,
                                                     13U,
                                                     NULL,
                                                     &value));
  IntersectionEngineTick(&s_engine);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kUnitControlStatusOid,
                                                13U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(2U, value.data.unsigned32);

  s_controller.lastSnapshotValid = 1U;
  s_controller.lastSnapshot = (ModuleBusSnapshot_t) {
    MODULE_BUS_PROTOCOL_VERSION,
    (uint8_t) (MODULE_BUS_SNAPSHOT_VALID_DETECTORS
               | MODULE_BUS_SNAPSHOT_VALID_MMU_STATUS),
    0U,
    (uint8_t) (MODULE_BUS_SNAPSHOT_VALID_DETECTORS
               | MODULE_BUS_SNAPSHOT_VALID_MMU_STATUS),
    0U,
    0U,
    0U,
    0U,
    0U,
    0U,
    0U,
    0U,
    0U,
    0U
  };

  TEST_ASSERT_TRUE(UnitAlarmPortSetUnitAlarmStatus1(&s_unitAlarmPort, 0x04U));
  TEST_ASSERT_TRUE(UnitAlarmPortSetUnitAlarmStatus3(&s_unitAlarmPort, 0x08U));
  TEST_ASSERT_TRUE(IntersectionEngineSetMmuFlashControl(&s_engine, 1U));
  IntersectionEngineTick(&s_engine);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kUnitFlashStatusOid,
                                                13U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(6U, value.data.unsigned32);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kUnitAlarmStatus1Oid,
                                                13U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(0x14U, value.data.unsigned32);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kShortAlarmStatusOid,
                                                13U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(0x22U, value.data.unsigned32 & 0x22U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kUnitAlarmStatus3Oid,
                                                13U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(0x08U, value.data.unsigned32);
} /* test_unit_status_objects_project_runtime_and_module_bus_health */

void test_unit_alarm_status2_and_short_alarm_status_follow_runtime_and_alarm_inputs(
  void)
{
  NtcipValue_t value;

  TEST_ASSERT_TRUE(UnitAlarmPortSetUnitAlarmStatus2(&s_unitAlarmPort, 0x11U));
  TEST_ASSERT_TRUE(UnitAlarmPortSetAlarmGroupState(&s_unitAlarmPort, 0U, 0x01U));

  NtcipValueSetUnsigned32(&value, 15U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kSystemSyncControlOid,
                                                     13U,
                                                     NULL,
                                                     &value));
  NtcipValueSetUnsigned32(&value, 1U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kSystemPatternControlOid,
                                                     13U,
                                                     NULL,
                                                     &value));

  IntersectionEngineTick(&s_engine);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kUnitAlarmStatus2Oid,
                                                13U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(0x31U, value.data.unsigned32);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kUnitAlarmStatus2Oid,
                                                13U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(0x30U, value.data.unsigned32);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kShortAlarmStatusOid,
                                                13U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(0xC0U, value.data.unsigned32 & 0xC0U);
}

void test_unit_alarm_status1_sets_cycle_fault_after_two_coordinated_cycles(void)
{
  IntersectionConfig_t config;
  NtcipValue_t value;

  config = MakeTwoPhasePerRingConfig();
  config.coordination.patterns[0].cycleTimeSeconds = 80U;
  config.coordination.patterns[0].offsetTimeSeconds = 0U;
  config.coordination.patterns[0].splitNumber = 1U;
  config.coordination.splits[0][0].timeSeconds = 20U;
  config.coordination.splits[0][1].timeSeconds = 20U;
  config.coordination.splits[0][2].timeSeconds = 20U;
  config.coordination.splits[0][3].timeSeconds = 20U;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  TEST_ASSERT_TRUE(IntersectionEngineSetSystemPatternControl(&s_engine, 1U));

  IntersectionEngineTick(&s_engine);
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 2U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 4U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetPhaseHoldControl(&s_engine, 1U, 1U));

  for (uint32_t tick = 0U; tick < 16100U; tick++)
  {
    IntersectionEngineTick(&s_engine);
  }

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kUnitAlarmStatus1Oid,
                                                13U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(0x41U, value.data.unsigned32 & 0x41U);
}

void test_unit_alarm_status1_sets_coord_fault_after_service_within_retry_window(
  void)
{
  IntersectionConfig_t config;
  NtcipValue_t value;

  config = MakeTwoPhasePerRingConfig();
  config.coordination.patterns[0].cycleTimeSeconds = 80U;
  config.coordination.patterns[0].offsetTimeSeconds = 0U;
  config.coordination.patterns[0].splitNumber = 1U;
  config.coordination.splits[0][0].timeSeconds = 20U;
  config.coordination.splits[0][1].timeSeconds = 20U;
  config.coordination.splits[0][2].timeSeconds = 20U;
  config.coordination.splits[0][3].timeSeconds = 20U;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  TEST_ASSERT_TRUE(IntersectionEngineSetSystemPatternControl(&s_engine, 1U));

  IntersectionEngineTick(&s_engine);
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 2U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 4U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetPhaseHoldControl(&s_engine, 1U, 1U));

  for (uint32_t tick = 0U; tick < 16100U; tick++)
  {
    IntersectionEngineTick(&s_engine);
  }

  TEST_ASSERT_TRUE(IntersectionEngineSetPhaseHoldControl(&s_engine, 1U, 0U));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 2U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 4U, 1U));

  {
    uint8_t reachedPhase2 = 0U;

    for (uint32_t tick = 0U; tick < 10000U; tick++)
    {
      uint8_t activePhaseNumber = 0U;

      TEST_ASSERT_TRUE(IntersectionEngineGetActivePhaseForRing(&s_engine,
                                                               1U,
                                                               &activePhaseNumber));
      if (activePhaseNumber == 2U)
      {
        reachedPhase2 = 1U;
        break;
      }

      IntersectionEngineTick(&s_engine);
    }

    TEST_ASSERT_EQUAL_UINT8(1U, reachedPhase2);
  }

  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 2U, 0U));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 4U, 0U));
  IntersectionEngineTick(&s_engine);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kUnitAlarmStatus1Oid,
                                                13U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(0x82U, value.data.unsigned32 & 0xC3U);
}

void test_unit_alarm_status1_sets_coord_fail_and_cycle_fail_bits_from_runtime(
  void)
{
  IntersectionConfig_t config;
  NtcipValue_t value;

  config = MakeTwoPhasePerRingConfig();
  config.coordination.patterns[0].cycleTimeSeconds = 80U;
  config.coordination.patterns[0].offsetTimeSeconds = 0U;
  config.coordination.patterns[0].splitNumber = 1U;
  config.coordination.splits[0][0].timeSeconds = 20U;
  config.coordination.splits[0][1].timeSeconds = 20U;
  config.coordination.splits[0][2].timeSeconds = 20U;
  config.coordination.splits[0][3].timeSeconds = 20U;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  TEST_ASSERT_TRUE(IntersectionEngineSetSystemPatternControl(&s_engine, 1U));

  IntersectionEngineTick(&s_engine);
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 2U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 4U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetPhaseHoldControl(&s_engine, 1U, 1U));

  for (uint32_t tick = 0U; tick < 16100U; tick++)
  {
    IntersectionEngineTick(&s_engine);
  }

  TEST_ASSERT_TRUE(IntersectionEngineSetPhaseHoldControl(&s_engine, 1U, 0U));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 2U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 4U, 1U));

  {
    uint8_t reachedPhase2 = 0U;

    for (uint32_t tick = 0U; tick < 10000U; tick++)
    {
      uint8_t activePhaseNumber = 0U;

      TEST_ASSERT_TRUE(IntersectionEngineGetActivePhaseForRing(
        &s_engine,
        1U,
        &activePhaseNumber));
      if (activePhaseNumber == 2U)
      {
        reachedPhase2 = 1U;
        break;
      }

      IntersectionEngineTick(&s_engine);
    }

    TEST_ASSERT_EQUAL_UINT8(1U, reachedPhase2);
  }

  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 2U, 0U));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 4U, 0U));
  IntersectionEngineTick(&s_engine);
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 1U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 3U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetRingForceOffControl(&s_engine, 1U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetRingForceOffControl(&s_engine, 2U, 1U));

  {
    uint8_t reachedPhase1 = 0U;

    for (uint32_t tick = 0U; tick < 10000U; tick++)
    {
      uint8_t activePhaseNumber = 0U;

      TEST_ASSERT_TRUE(IntersectionEngineGetActivePhaseForRing(
        &s_engine,
        1U,
        &activePhaseNumber));
      if (activePhaseNumber == 1U)
      {
        reachedPhase1 = 1U;
        break;
      }

      IntersectionEngineTick(&s_engine);
    }

    TEST_ASSERT_EQUAL_UINT8(1U, reachedPhase1);
  }

  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 1U, 0U));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 3U, 0U));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 2U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 4U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetPhaseHoldControl(&s_engine, 1U, 1U));

  for (uint32_t tick = 0U; tick < 50000U; tick++)
  {
    IntersectionEngineTick(&s_engine);

    TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                          NtcipObjectDirectoryGet(&s_directory,
                                                  kUnitAlarmStatus1Oid,
                                                  13U,
                                                  NULL,
                                                  &value));

    if ((value.data.unsigned32 & 0x4FU) == 0x4DU)
    {
      break;
    }
  }

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kUnitAlarmStatus1Oid,
                                                13U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(0x4DU, value.data.unsigned32 & 0x4FU);
}

void test_short_alarm_status_sets_coordination_alarm_after_three_failed_cycles(
  void)
{
  IntersectionConfig_t config;
  NtcipValue_t value;

  config = MakeTwoPhasePerRingConfig();
  config.coordination.patterns[0].cycleTimeSeconds = 80U;
  config.coordination.patterns[0].offsetTimeSeconds = 0U;
  config.coordination.patterns[0].splitNumber = 1U;
  config.coordination.splits[0][0].timeSeconds = 20U;
  config.coordination.splits[0][1].timeSeconds = 20U;
  config.coordination.splits[0][2].timeSeconds = 20U;
  config.coordination.splits[0][3].timeSeconds = 20U;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));

  NtcipValueSetUnsigned32(&value, 1U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kSystemPatternControlOid,
                                                     13U,
                                                     NULL,
                                                     &value));

  IntersectionEngineTick(&s_engine);
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 2U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 4U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetPhaseHoldControl(&s_engine, 1U, 1U));
  for (uint32_t tick = 0U; tick < 16100U; tick++)
  {
    IntersectionEngineTick(&s_engine);
  }

  for (uint32_t tick = 0U; tick < 25000U; tick++)
  {
    IntersectionEngineTick(&s_engine);
    TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                          NtcipObjectDirectoryGet(&s_directory,
                                                  kShortAlarmStatusOid,
                                                  13U,
                                                  NULL,
                                                  &value));

    if ((value.data.unsigned32 & 0x10U) != 0U)
    {
      break;
    }
  }

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kShortAlarmStatusOid,
                                                13U,
                                                NULL,
                                                &value));
  TEST_ASSERT_BITS_HIGH(0x10U, value.data.unsigned32);
}

void test_unit_flash_status_reports_startup_and_defers_mmu_until_expiry(void)
{
  IntersectionConfig_t config;
  NtcipValue_t value;

  config = MakeTwoPhasePerRingConfig();
  config.unit.startUpFlashSeconds = 1U;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  TEST_ASSERT_TRUE(IntersectionEngineSetMmuFlashControl(&s_engine, 1U));

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kUnitFlashStatusOid,
                                                13U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(7U, value.data.unsigned32);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kUnitAlarmStatus1Oid,
                                                13U,
                                                NULL,
                                                &value));
  TEST_ASSERT_BITS_LOW(0x10U, value.data.unsigned32);

  for (uint32_t tick = 0U; tick < 100U; tick++)
  {
    IntersectionEngineTick(&s_engine);
  }

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kUnitFlashStatusOid,
                                                13U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(6U, value.data.unsigned32);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kUnitAlarmStatus1Oid,
                                                13U,
                                                NULL,
                                                &value));
  TEST_ASSERT_BITS_HIGH(0x10U, value.data.unsigned32);
}

void test_unit_flash_status_reports_fault_monitor_for_cp_mp_safety_action(void)
{
  NtcipValue_t value;

  PrimeCpMpLinkService(CPMP_SAFETY_ACTION_FLASH,
                       TEST_CPMP_REASON_CONFIG_INVALID,
                       CPMP_FAULT_GLOBAL_FLAG_CONFIG_INVALID,
                       0U);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kUnitFlashStatusOid,
                                                13U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(5U, value.data.unsigned32);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kUnitAlarmStatus1Oid,
                                                13U,
                                                NULL,
                                                &value));
  TEST_ASSERT_BITS_LOW(0x30U, value.data.unsigned32);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kUnitAlarmStatus4Oid,
                                                13U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(0U, value.data.unsigned32);
}

void test_unit_flash_status_does_not_report_fault_monitor_for_cp_mp_dark_action(void)
{
  NtcipValue_t value;

  PrimeCpMpLinkService(CPMP_SAFETY_ACTION_DARK,
                       TEST_CPMP_REASON_CONFLICT,
                       0U,
                       CPMP_FAULT_CHANNEL_FLAG_CONFLICT);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kUnitFlashStatusOid,
                                                13U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(1U, value.data.unsigned32);
}

void test_unit_alarm_statuses_map_cp_mp_fault_flags_to_standard_bits(void)
{
  NtcipValue_t value;

  PrimeCpMpLinkService(CPMP_SAFETY_ACTION_FLASH,
                       TEST_CPMP_REASON_CONFLICT,
                       CPMP_FAULT_GLOBAL_FLAG_LOCAL_FLASH_ACTIVE
                         | CPMP_FAULT_GLOBAL_FLAG_MMU_FLASH_ACTIVE
                         | CPMP_FAULT_GLOBAL_FLAG_WATCHDOG
                         | CPMP_FAULT_GLOBAL_FLAG_AC_LINE
                         | CPMP_FAULT_GLOBAL_FLAG_PSM_MISSING,
                       0U);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kUnitAlarmStatus1Oid,
                                                13U,
                                                NULL,
                                                &value));
  TEST_ASSERT_BITS_HIGH(0x30U, value.data.unsigned32);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kUnitAlarmStatus2Oid,
                                                13U,
                                                NULL,
                                                &value));
  TEST_ASSERT_BITS_HIGH(0x40U, value.data.unsigned32);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kUnitAlarmStatus3Oid,
                                                13U,
                                                NULL,
                                                &value));
  TEST_ASSERT_BITS_HIGH(0x11U, value.data.unsigned32);
}

void test_vendor_cp_mp_diagnostics_objects_expose_cached_fault_status(void)
{
  NtcipValue_t value;

  PrimeCpMpLinkService(CPMP_SAFETY_ACTION_DARK,
                       TEST_CPMP_REASON_CONFLICT,
                       CPMP_FAULT_GLOBAL_FLAG_CP_MISSING
                         | CPMP_FAULT_GLOBAL_FLAG_CONFIG_INVALID,
                       CPMP_FAULT_CHANNEL_FLAG_CONFLICT
                         | CPMP_FAULT_CHANNEL_FLAG_RED_FAIL);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kCpMpDiagPeerHealthyOid,
                                                10U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(1U, value.data.unsigned32);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kCpMpDiagSafetyActionOid,
                                                10U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(CPMP_SAFETY_ACTION_DARK, value.data.unsigned32);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kCpMpDiagSafetyReasonCodeOid,
                                                10U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(TEST_CPMP_REASON_CONFLICT,
                           value.data.unsigned32);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kCpMpDiagGlobalFlagsOid,
                                                10U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(CPMP_FAULT_GLOBAL_FLAG_CP_MISSING
                             | CPMP_FAULT_GLOBAL_FLAG_CONFIG_INVALID,
                           value.data.unsigned32);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kCpMpDiagChannelFlagsOid,
                                                12U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(CPMP_FAULT_CHANNEL_FLAG_CONFLICT
                             | CPMP_FAULT_CHANNEL_FLAG_RED_FAIL,
                           value.data.unsigned32);
}

void test_short_alarm_status_clears_local_cycle_zero_bit_after_read(void)
{
  NtcipValue_t value;
  uint8_t latched = 0U;
  uint32_t tick = 0U;

  NtcipValueSetUnsigned32(&value, 0U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kSystemSyncControlOid,
                                                     13U,
                                                     NULL,
                                                     &value));
  NtcipValueSetUnsigned32(&value, 1U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kSystemPatternControlOid,
                                                     13U,
                                                     NULL,
                                                     &value));

  for (tick = 0U; tick < 20000U; tick++)
  {
    IntersectionEngineTick(&s_engine);
    TEST_ASSERT_TRUE(IntersectionEngineGetShortAlarmCycleZeroLatched(&s_engine,
                                                                     &latched));
    if (latched != 0U)
    {
      break;
    }
  }

  TEST_ASSERT_EQUAL_UINT8(1U, latched);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kShortAlarmStatusOid,
                                                13U,
                                                NULL,
                                                &value));
  TEST_ASSERT_NOT_EQUAL(0U, value.data.unsigned32 & 0x04U);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kShortAlarmStatusOid,
                                                13U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(0U, value.data.unsigned32 & 0x04U);
}

void test_unit_control_object_rejects_reserved_bit_and_drives_runtime_demands(
  void)
{
  NtcipValue_t value;

  NtcipValueSetUnsigned32(&value, 0x01U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_BAD_VALUE,
                        NtcipObjectDirectorySetTest(&s_directory,
                                                    kUnitControlOid,
                                                    13U,
                                                    NULL,
                                                    &value));

  NtcipValueSetUnsigned32(&value, 0x04U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetTest(&s_directory,
                                                    kUnitControlOid,
                                                    13U,
                                                    NULL,
                                                    &value));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kUnitControlOid,
                                                     13U,
                                                     NULL,
                                                     &value));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kUnitControlOid,
                                                13U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(0x04U, value.data.unsigned32);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kUnitControlStatusOid,
                                                13U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(1U, value.data.unsigned32);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kPhaseStatusGroupVehCallsOid,
                                                14U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(0xFFU, value.data.unsigned32);
}

void test_preempt_verify_accepts_short_service_exit_type(void)
{
  NtcipRequestContext_t request = { 0x9999U, 0U, 0U };
  NtcipValue_t value;
  const uint8_t trackPhase[] = { 2U };

  BeginTransaction(&request, 72U);

  NtcipValueSetUnsigned32(&value, 0x10U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kPreemptControlOid,
                                                     15U,
                                                     &request,
                                                     &value));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipValueSetOctetString(&value,
                                                 trackPhase,
                                                 sizeof(trackPhase)));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kPreemptTrackPhaseOid,
                                                     15U,
                                                     &request,
                                                     &value));
  NtcipValueSetUnsigned32(&value,
                          INTERSECTION_PREEMPT_EXIT_TYPE_SHORT_SERVICE);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kPreemptExitTypeOid,
                                                     15U,
                                                     &request,
                                                     &value));
  NtcipValueSetUnsigned32(&value, (uint32_t) NTCIP_DB_CREATE_STATE_VERIFY);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kDbCreateTransactionOid,
                                                     13U,
                                                     &request,
                                                     &value));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kDbVerifyStatusOid,
                                                13U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(NTCIP_DB_VERIFY_STATUS_DONE_WITH_NO_ERROR,
                           value.data.unsigned32);
}

void test_unit_alarm_group_and_alarm_status4_objects_follow_bound_port(void)
{
  NtcipValue_t value;

  TEST_ASSERT_TRUE(UnitAlarmPortSetMaxAlarmGroups(&s_unitAlarmPort, 1U));
  TEST_ASSERT_TRUE(UnitAlarmPortSetAlarmGroupState(&s_unitAlarmPort, 0U, 0xA5U));
  TEST_ASSERT_TRUE(UnitAlarmPortSetUnitAlarmStatus4(&s_unitAlarmPort, 0x24U));

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kMaxAlarmGroupsOid,
                                                13U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(1U, value.data.unsigned32);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kAlarmGroupNumberOid,
                                                15U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(1U, value.data.unsigned32);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kAlarmGroupStateOid,
                                                15U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(0xA5U, value.data.unsigned32);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kUnitAlarmStatus4Oid,
                                                13U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(0x24U, value.data.unsigned32);
}

void test_unit_alarm_status2_clears_power_restart_bit_after_read(void)
{
  NtcipValue_t value;

  TEST_ASSERT_TRUE(UnitAlarmPortSetUnitAlarmStatus2(&s_unitAlarmPort, 0x83U));

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kUnitAlarmStatus2Oid,
                                                13U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(0x83U, value.data.unsigned32);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kUnitAlarmStatus2Oid,
                                                13U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(0x82U, value.data.unsigned32);
}

void test_alarm_group_table_rejects_rows_above_max_alarm_groups(void)
{
  NtcipValue_t value;
  static const uint32_t kAlarmGroupStateRow2Oid[] =
  {
    1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 3U, 12U, 1U, 2U, 2U
  };

  TEST_ASSERT_TRUE(UnitAlarmPortSetMaxAlarmGroups(&s_unitAlarmPort, 1U));

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_RANGE_ERROR,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kAlarmGroupStateRow2Oid,
                                                15U,
                                                NULL,
                                                &value));
}

void test_global_set_id_table_reports_zero_supported_rows(void)
{
  NtcipValue_t value;

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kMaxGlobalSetIdsOid,
                                                13U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(0U, value.data.unsigned32);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_RANGE_ERROR,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kGlobalSetIdNumberOid,
                                                15U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_RANGE_ERROR,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kGlobalSetIdOidOid,
                                                15U,
                                                NULL,
                                                &value));
}

void test_unit_config_objects_are_transactional_and_committed(void)
{
  NtcipRequestContext_t request = { 0x6111U, 0U, 0U };
  NtcipValue_t value;

  NtcipValueSetUnsigned32(&value, (uint32_t) NTCIP_DB_CREATE_STATE_TRANSACTION);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kDbCreateTransactionOid,
                                                     13U,
                                                     &request,
                                                     &value));
  NtcipValueSetUnsigned32(&value, 7U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kDbTransactionIdOid,
                                                     13U,
                                                     &request,
                                                     &value));
  request.transactionIdValid = 1U;
  request.transactionId = 7U;

  NtcipValueSetUnsigned32(&value, 9U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kUnitStartUpFlashOid,
                                                     13U,
                                                     &request,
                                                     &value));
  NtcipValueSetUnsigned32(
    &value,
    (uint32_t) INTERSECTION_UNIT_AUTO_PEDESTRIAN_CLEAR_ENABLE);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kUnitAutoPedestrianClearOid,
                                                     13U,
                                                     &request,
                                                     &value));
  NtcipValueSetUnsigned32(&value, 123U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kUnitBackupTimeOid,
                                                     13U,
                                                     &request,
                                                     &value));
  NtcipValueSetUnsigned32(&value, 7U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kUnitRedRevertOid,
                                                     13U,
                                                     &request,
                                                     &value));
  NtcipValueSetUnsigned32(
    &value,
    (uint32_t) INTERSECTION_UNIT_STARTUP_FLASH_MODE_ALL_RED_FLASH_OVERRIDE);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kUnitStartUpFlashModeOid,
                                                     13U,
                                                     &request,
                                                     &value));
  NtcipValueSetUnsigned32(&value, 22U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kAscElevationOffsetOid,
                                                     13U,
                                                     &request,
                                                     &value));

  NtcipValueSetUnsigned32(&value, (uint32_t) NTCIP_DB_CREATE_STATE_VERIFY);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kDbCreateTransactionOid,
                                                     13U,
                                                     &request,
                                                     &value));
  NtcipValueSetUnsigned32(&value, (uint32_t) NTCIP_DB_CREATE_STATE_NORMAL);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kDbCreateTransactionOid,
                                                     13U,
                                                     &request,
                                                     &value));

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kUnitStartUpFlashOid,
                                                13U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(9U, value.data.unsigned32);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kUnitAutoPedestrianClearOid,
                                                13U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(
    (uint32_t) INTERSECTION_UNIT_AUTO_PEDESTRIAN_CLEAR_ENABLE,
    value.data.unsigned32);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kUnitBackupTimeOid,
                                                13U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(123U, value.data.unsigned32);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kUnitRedRevertOid,
                                                13U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(7U, value.data.unsigned32);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kUnitStartUpFlashModeOid,
                                                13U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(
    (uint32_t) INTERSECTION_UNIT_STARTUP_FLASH_MODE_ALL_RED_FLASH_OVERRIDE,
    value.data.unsigned32);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kAscElevationOffsetOid,
                                                13U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(22U, value.data.unsigned32);
}

void test_unit_asc_elevation_offset_rejects_values_above_mib_limit(void)
{
  NtcipRequestContext_t request = { 0x6112U, 0U, 0U };
  NtcipValue_t value;

  NtcipValueSetUnsigned32(&value, (uint32_t) NTCIP_DB_CREATE_STATE_TRANSACTION);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kDbCreateTransactionOid,
                                                     13U,
                                                     &request,
                                                     &value));
  NtcipValueSetUnsigned32(&value, 8U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kDbTransactionIdOid,
                                                     13U,
                                                     &request,
                                                     &value));
  request.transactionIdValid = 1U;
  request.transactionId = 8U;

  NtcipValueSetUnsigned32(&value, 32U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_RANGE_ERROR,
                        NtcipObjectDirectorySetTest(&s_directory,
                                                    kAscElevationOffsetOid,
                                                    13U,
                                                    &request,
                                                    &value));
}

void test_unit_user_defined_backup_objects_are_transactional_and_committed(void)
{
  NtcipRequestContext_t request = { 0x6161U, 0U, 0U };
  NtcipValue_t value;
  static const uint32_t backupMatchOid[] =
  {
    1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 4U, 14U, 0U
  };
  static const uint8_t backupDescription[] = "systemPatternControl";

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kMaxUserDefinedBackupTimeContentOid,
                                                13U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(INTERSECTION_USER_DEFINED_BACKUP_CONTENT_COUNT_MAX,
                           value.data.unsigned32);

  BeginTransaction(&request, 8U);

  NtcipValueSetUnsigned32(&value, 2U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetTest(
                          &s_directory,
                          kUnitUserDefinedBackupTimeOid,
                          13U,
                          &request,
                          &value));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(
                          &s_directory,
                          kUnitUserDefinedBackupTimeOid,
                          13U,
                          &request,
                          &value));

  NtcipValueSetUnsigned32(&value, 456U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kUnitBackupTimeOid,
                                                     13U,
                                                     &request,
                                                     &value));

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipValueSetObjectId(&value,
                                              backupMatchOid,
                                              (uint8_t) (sizeof(backupMatchOid)
                                                         / sizeof(backupMatchOid[0]))));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetTest(
                          &s_directory,
                          kUnitUserDefinedBackupTimeContentOidOid,
                          15U,
                          &request,
                          &value));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(
                          &s_directory,
                          kUnitUserDefinedBackupTimeContentOidOid,
                          15U,
                          &request,
                          &value));

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipValueSetOctetString(&value,
                                                 backupDescription,
                                                 sizeof(backupDescription) - 1U));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetTest(
                          &s_directory,
                          kUnitUserDefinedBackupTimeContentDescriptionOid,
                          15U,
                          &request,
                          &value));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(
                          &s_directory,
                          kUnitUserDefinedBackupTimeContentDescriptionOid,
                          15U,
                          &request,
                          &value));

  VerifyAndCommitTransaction(&request);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kUnitUserDefinedBackupTimeOid,
                                                13U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(2U, value.data.unsigned32);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kUnitBackupTimeOid,
                                                13U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(0U, value.data.unsigned32);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(
                          &s_directory,
                          kUnitUserDefinedBackupTimeContentNumberOid,
                          15U,
                          NULL,
                          &value));
  TEST_ASSERT_EQUAL_UINT32(1U, value.data.unsigned32);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(
                          &s_directory,
                          kUnitUserDefinedBackupTimeContentOidOid,
                          15U,
                          NULL,
                          &value));
  TEST_ASSERT_EQUAL_INT(NTCIP_VALUE_TYPE_OBJECT_ID, value.type);
  TEST_ASSERT_EQUAL_UINT8(sizeof(backupMatchOid) / sizeof(backupMatchOid[0]),
                          value.data.objectId.length);
  TEST_ASSERT_EQUAL_UINT32_ARRAY(backupMatchOid,
                                 value.data.objectId.elements,
                                 value.data.objectId.length);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(
                          &s_directory,
                          kUnitUserDefinedBackupTimeContentDescriptionOid,
                          15U,
                          NULL,
                          &value));
  TEST_ASSERT_EQUAL_INT(NTCIP_VALUE_TYPE_OCTET_STRING, value.type);
  TEST_ASSERT_EQUAL_UINT16(sizeof(backupDescription) - 1U,
                           value.data.octetString.length);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(backupDescription,
                                value.data.octetString.bytes,
                                value.data.octetString.length);
}

void test_remote_manual_control_objects_require_mode_and_follow_runtime_timer(
  void)
{
  NtcipValue_t value;

  NtcipValueSetUnsigned32(&value, 1U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_BAD_VALUE,
                        NtcipObjectDirectorySetTest(&s_directory,
                                                    kUnitMceIntAdvOid,
                                                    13U,
                                                    NULL,
                                                    &value));

  NtcipValueSetUnsigned32(&value, 2U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kUnitMceTimeoutOid,
                                                     13U,
                                                     NULL,
                                                     &value));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kUnitMceTimeoutOid,
                                                13U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(2U, value.data.unsigned32);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kUnitControlStatusOid,
                                                13U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(9U, value.data.unsigned32);

  NtcipValueSetUnsigned32(&value, 1U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetTest(&s_directory,
                                                    kUnitMceIntAdvOid,
                                                    13U,
                                                    NULL,
                                                    &value));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kUnitMceIntAdvOid,
                                                     13U,
                                                     NULL,
                                                     &value));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kUnitMceIntAdvOid,
                                                13U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(1U, value.data.unsigned32);

  IntersectionEngineTick(&s_engine);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kUnitMceIntAdvOid,
                                                13U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(0U, value.data.unsigned32);

  {
    uint32_t tickIndex;

    for (tickIndex = 0U; tickIndex < 100U; tickIndex++)
    {
      IntersectionEngineTick(&s_engine);
    }
  }

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kUnitMceTimeoutOid,
                                                13U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(1U, value.data.unsigned32);

  NtcipValueSetUnsigned32(&value, 0U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kUnitMceTimeoutOid,
                                                     13U,
                                                     NULL,
                                                     &value));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kUnitMceTimeoutOid,
                                                13U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(0U, value.data.unsigned32);
}

void test_matching_set_oid_resets_user_defined_backup_timer(void)
{
  NtcipRequestContext_t request = { 0xA2A2U, 0U, 0U };
  NtcipValue_t value;
  const IntersectionRuntime_t *runtime;
  static const uint32_t backupMatchOid[] =
  {
    1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 4U, 14U, 0U
  };
  uint32_t tickIndex;

  BeginTransaction(&request, 22U);

  NtcipValueSetUnsigned32(&value, 1U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(
                          &s_directory,
                          kUnitUserDefinedBackupTimeOid,
                          13U,
                          &request,
                          &value));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipValueSetObjectId(&value,
                                              backupMatchOid,
                                              (uint8_t) (sizeof(backupMatchOid)
                                                         / sizeof(backupMatchOid[0]))));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(
                          &s_directory,
                          kUnitUserDefinedBackupTimeContentOidOid,
                          15U,
                          &request,
                          &value));

  VerifyAndCommitTransaction(&request);

  NtcipValueSetUnsigned32(&value, 0U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kActionPlanControlOid,
                                                     13U,
                                                     NULL,
                                                     &value));
  for (tickIndex = 0U; tickIndex < 100U; tickIndex++)
  {
    IntersectionEngineTick(&s_engine);
  }

  runtime = IntersectionEngineGetRuntime(&s_engine);
  TEST_ASSERT_EQUAL_UINT8(0U, runtime->backupModeActive);

  NtcipValueSetUnsigned32(&value, 1U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kSystemPatternControlOid,
                                                     13U,
                                                     NULL,
                                                     &value));
  for (tickIndex = 0U; tickIndex < 99U; tickIndex++)
  {
    IntersectionEngineTick(&s_engine);
  }

  runtime = IntersectionEngineGetRuntime(&s_engine);
  TEST_ASSERT_EQUAL_UINT8(0U, runtime->backupModeActive);

  IntersectionEngineTick(&s_engine);
  runtime = IntersectionEngineGetRuntime(&s_engine);
  TEST_ASSERT_EQUAL_UINT8(1U, runtime->backupModeActive);
  TEST_ASSERT_EQUAL_UINT8(4U, runtime->unitControlStatus);
}

void test_special_function_output_objects_follow_runtime_control_and_timebase(
  void)
{
  NtcipValue_t value;

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kMaxSpecialFunctionOutputsOid,
                                                13U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(INTERSECTION_SPECIAL_FUNCTION_OUTPUT_COUNT,
                           value.data.unsigned32);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kSpecialFunctionOutputNumberOid,
                                                15U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(1U, value.data.unsigned32);

  NtcipValueSetUnsigned32(&value, 1U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(
                          &s_directory,
                          kSpecialFunctionOutputStateOid,
                          15U,
                          NULL,
                          &value));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kSpecialFunctionOutputControlOid,
                                                15U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(1U, value.data.unsigned32);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kSpecialFunctionOutputStatusOid,
                                                15U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(1U, value.data.unsigned32);

  s_engine.config.timebase.actions[0].specialFunction = 0x01U;
  TEST_ASSERT_TRUE(IntersectionEngineSetActionPlanControl(&s_engine, 1U));
  NtcipValueSetUnsigned32(&value, 0U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(
                          &s_directory,
                          kSpecialFunctionOutputControlOid,
                          15U,
                          NULL,
                          &value));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kSpecialFunctionOutputStatusOid,
                                                15U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(1U, value.data.unsigned32);
}

void test_timebase_objects_route_transactional_table_and_runtime_action_control(
  void)
{
  NtcipRequestContext_t request = { 0x5151U, 0U, 0U };
  NtcipValue_t value;

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kMaxTimebaseAscActionsOid,
                                                13U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(INTERSECTION_TIMEBASE_ACTION_COUNT_MAX,
                           value.data.unsigned32);

  BeginTransaction(&request, 17U);

  NtcipValueSetUnsigned32(&value, 255U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetTest(&s_directory,
                                                    kTimebaseAscPatternOid,
                                                    15U,
                                                    &request,
                                                    &value));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kTimebaseAscPatternOid,
                                                     15U,
                                                     &request,
                                                     &value));

  NtcipValueSetUnsigned32(
    &value,
    INTERSECTION_TIMEBASE_AUX_FUNCTION_DIMMING
    | INTERSECTION_TIMEBASE_AUX_FUNCTION_1);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetTest(&s_directory,
                                                    kTimebaseAscAuxFunctionOid,
                                                    15U,
                                                    &request,
                                                    &value));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kTimebaseAscAuxFunctionOid,
                                                     15U,
                                                     &request,
                                                     &value));

  NtcipValueSetUnsigned32(&value, 4321U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetTest(&s_directory,
                                                    kTimebaseAscPatternSyncOid,
                                                    13U,
                                                    &request,
                                                    &value));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kTimebaseAscPatternSyncOid,
                                                     13U,
                                                     &request,
                                                     &value));

  VerifyAndCommitTransaction(&request);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kTimebaseAscPatternOid,
                                                15U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(255U, value.data.unsigned32);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kTimebaseAscPatternSyncOid,
                                                13U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(4321U, value.data.unsigned32);

  NtcipValueSetUnsigned32(&value, 1U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kActionPlanControlOid,
                                                     13U,
                                                     NULL,
                                                     &value));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kActionPlanControlOid,
                                                13U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(1U, value.data.unsigned32);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kTimebaseAscActionStatusOid,
                                                13U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(1U, value.data.unsigned32);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kUnitControlStatusOid,
                                                13U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(6U, value.data.unsigned32);
}

void test_timebase_auxiliary_reserved_bits_are_rejected(void)
{
  NtcipRequestContext_t request = { 0x5252U, 0U, 0U };
  NtcipValue_t value;

  BeginTransaction(&request, 18U);
  NtcipValueSetUnsigned32(&value, 0x10U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_BAD_VALUE,
                        NtcipObjectDirectorySetTest(&s_directory,
                                                    kTimebaseAscAuxFunctionOid,
                                                    15U,
                                                    &request,
                                                    &value));
}

void test_unit_control_status_reports_interconnect_and_backup(void)
{
  NtcipRequestContext_t request = { 0x5353U, 0U, 0U };
  NtcipValue_t value;

  BeginTransaction(&request, 19U);
  NtcipValueSetUnsigned32(&value, 254U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kTimebaseAscPatternOid,
                                                     15U,
                                                     &request,
                                                     &value));
  VerifyAndCommitTransaction(&request);

  NtcipValueSetUnsigned32(&value, 1U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kActionPlanControlOid,
                                                     13U,
                                                     NULL,
                                                     &value));
  NtcipValueSetUnsigned32(&value, 0x40U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kUnitControlOid,
                                                     13U,
                                                     NULL,
                                                     &value));

  TEST_ASSERT_TRUE(
    IntersectionEngineSetLocalInterconnectCommand(&s_engine, 255U));
  TEST_ASSERT_TRUE(
    IntersectionEngineSetLocalInterconnectInputsValid(&s_engine, 1U));

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kUnitControlStatusOid,
                                                13U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(7U, value.data.unsigned32);

  TEST_ASSERT_TRUE(
    IntersectionEngineSetLocalInterconnectInputsValid(&s_engine, 0U));

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kUnitControlStatusOid,
                                                13U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(8U, value.data.unsigned32);
}

void test_remote_command_lockout_denies_all_sets_except_unlock(void)
{
  NtcipRequestContext_t request = { 0xA1A1U, 0U, 0U };
  NtcipValue_t value;

  NtcipValueSetUnsigned32(&value, 0x02U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kUnitControlOid,
                                                     13U,
                                                     NULL,
                                                     &value));

  NtcipValueSetUnsigned32(&value, 1U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_NO_ACCESS,
                        NtcipObjectDirectorySetTest(&s_directory,
                                                    kSystemPatternControlOid,
                                                    13U,
                                                    NULL,
                                                    &value));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_NO_ACCESS,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kSystemPatternControlOid,
                                                     13U,
                                                     NULL,
                                                     &value));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kCoordPatternStatusOid,
                                                13U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(254U, value.data.unsigned32);

  NtcipValueSetUnsigned32(&value, 13U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_NO_ACCESS,
                        NtcipObjectDirectorySetTest(&s_directory,
                                                    kPhaseMinimumGreenOid,
                                                    15U,
                                                    &request,
                                                    &value));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_NO_ACCESS,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kPhaseMinimumGreenOid,
                                                     15U,
                                                     &request,
                                                     &value));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kPhaseMinimumGreenOid,
                                                15U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(5U, value.data.unsigned32);

  NtcipValueSetUnsigned32(&value, 0x06U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_NO_ACCESS,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kUnitControlOid,
                                                     13U,
                                                     NULL,
                                                     &value));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kUnitControlOid,
                                                13U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(0x02U, value.data.unsigned32);

  NtcipValueSetUnsigned32(&value, 0x00U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kUnitControlOid,
                                                     13U,
                                                     NULL,
                                                     &value));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kUnitControlOid,
                                                13U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(0x00U, value.data.unsigned32);

  NtcipValueSetUnsigned32(&value, 1U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kSystemPatternControlOid,
                                                     13U,
                                                     NULL,
                                                     &value));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kCoordPatternStatusOid,
                                                13U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(1U, value.data.unsigned32);
}

void test_coord_operational_mode_write_requires_transaction(void)
{
  NtcipValue_t value;

  NtcipValueSetUnsigned32(&value, 2U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_NO_TRANSACTION,
                        NtcipObjectDirectorySetTest(&s_directory,
                                                    kCoordOperationalModeOid,
                                                    13U,
                                                    NULL,
                                                    &value));
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_directory_routes_phase_minimum_green_oid_in_mib_units);
  RUN_TEST(test_candidate_changes_stay_hidden_until_transaction_commits);
  RUN_TEST(test_phase_walk_is_transactional_and_committed_in_seconds);
  RUN_TEST(test_database_writes_require_owner_and_matching_transaction_id);
  RUN_TEST(test_phase_ring_zero_disables_phase_and_auxio_group_is_present);
  RUN_TEST(test_global_set_id_changes_after_committed_configuration_change);
  RUN_TEST(test_phase_status_group_reads_runtime_from_engine);
  RUN_TEST(
    test_channel_and_overlap_runtime_status_follow_committed_configuration);
  RUN_TEST(test_ped_status_and_phase_pedestrian_channel_follow_runtime_service);
  RUN_TEST(test_phase_control_group_objects_drive_runtime_controls_and_force_off_clear);
  RUN_TEST(test_ring_control_group_objects_route_runtime_masks_and_validate_range);
  RUN_TEST(test_ring_control_group_force_off_mask_drives_engine_runtime);
  RUN_TEST(test_sequence_table_reports_supported_sequence_count_and_commits_reordered_ring);
  RUN_TEST(
    test_detector_objects_route_transactional_call_phase_and_runtime_status);
  RUN_TEST(
    test_detector_alarm_objects_include_module_bus_diagnostics_and_reported_alarm_bytes);
  RUN_TEST(
    test_detector_objects_expose_full_committed_config_and_pair_reciprocity);
  RUN_TEST(test_detector_control_group_actuation_drives_runtime_detector_status);
  RUN_TEST(
    test_detector_object_validation_rejects_invalid_ranges_and_reset_is_ephemeral);
  RUN_TEST(test_coordination_pattern_cycle_time_is_transactional_and_committed);
  RUN_TEST(test_system_pattern_and_sync_control_drive_runtime_coord_status);
  RUN_TEST(
    test_timebase_objects_route_transactional_table_and_runtime_action_control);
  RUN_TEST(test_timebase_auxiliary_reserved_bits_are_rejected);
  RUN_TEST(test_unit_control_status_reports_interconnect_and_backup);
  RUN_TEST(test_unit_status_objects_project_runtime_and_module_bus_health);
  RUN_TEST(
    test_unit_alarm_status2_and_short_alarm_status_follow_runtime_and_alarm_inputs);
  RUN_TEST(test_unit_alarm_status1_sets_cycle_fault_after_two_coordinated_cycles);
  RUN_TEST(
    test_unit_alarm_status1_sets_coord_fault_after_service_within_retry_window);
  RUN_TEST(test_unit_alarm_status1_sets_coord_fail_and_cycle_fail_bits_from_runtime);
  RUN_TEST(
    test_short_alarm_status_sets_coordination_alarm_after_three_failed_cycles);
  RUN_TEST(test_unit_flash_status_reports_startup_and_defers_mmu_until_expiry);
  RUN_TEST(test_unit_flash_status_reports_fault_monitor_for_cp_mp_safety_action);
  RUN_TEST(test_unit_flash_status_does_not_report_fault_monitor_for_cp_mp_dark_action);
  RUN_TEST(test_vendor_cp_mp_diagnostics_objects_expose_cached_fault_status);
  RUN_TEST(test_short_alarm_status_clears_local_cycle_zero_bit_after_read);
  RUN_TEST(test_unit_control_object_rejects_reserved_bit_and_drives_runtime_demands);
  RUN_TEST(test_unit_alarm_group_and_alarm_status4_objects_follow_bound_port);
  RUN_TEST(test_unit_alarm_status2_clears_power_restart_bit_after_read);
  RUN_TEST(test_alarm_group_table_rejects_rows_above_max_alarm_groups);
  RUN_TEST(test_global_set_id_table_reports_zero_supported_rows);
  RUN_TEST(test_unit_config_objects_are_transactional_and_committed);
  RUN_TEST(test_unit_asc_elevation_offset_rejects_values_above_mib_limit);
  RUN_TEST(test_unit_user_defined_backup_objects_are_transactional_and_committed);
  RUN_TEST(test_remote_manual_control_objects_require_mode_and_follow_runtime_timer);
  RUN_TEST(test_matching_set_oid_resets_user_defined_backup_timer);
  RUN_TEST(test_special_function_output_objects_follow_runtime_control_and_timebase);
  RUN_TEST(test_remote_command_lockout_denies_all_sets_except_unlock);
  RUN_TEST(test_coord_operational_mode_write_requires_transaction);
  RUN_TEST(test_preempt_objects_route_transactional_config_and_runtime_status);
  RUN_TEST(test_preempt_verify_accepts_short_service_exit_type);

  return UNITY_END();
}
