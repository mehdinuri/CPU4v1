/* App/Adapters/STM32/MmiCanAdapter.h
 *
 * Queue-backed FDCAN1 adapter for the new MMI v2 touchscreen protocol.
 * This adapter intentionally coexists with the legacy MMI task while panels
 * migrate to the canonical runtime/NTCIP-backed model.
 */
#ifndef MMI_CAN_ADAPTER_H
#define MMI_CAN_ADAPTER_H

#include <stdint.h>

#include "cmsis_os2.h"
#include "fdcan.h"

#include "Adapters/STM32/LWIPSNMPAdapter.h"
#include "Domain/Services/MmiEventLogService.h"
#include "Domain/Services/MmiLocalSettingsService.h"
#include "Domain/Services/MmiMaintenanceService.h"
#include "Domain/Services/MmiService.h"
#include "Domain/Services/MmiSnapshotCache.h"

#define MMI_CAN_ADAPTER_SUBSCRIPTION_MAX 16U

typedef struct
{
  uint8_t active;
  uint8_t topicId;
  uint8_t recordIndex;
  uint8_t sequence;
  uint8_t dirty;
  uint16_t periodTicks;
  uint16_t ticksUntilDue;
} MmiRuntimeSubscription_t;

typedef struct
{
  FDCAN_HandleTypeDef *hfdcan;
  MmiService_t *service;
  MmiSnapshotCache_t *snapshotCache;
  MmiEventLogService_t *eventLogService;
  MmiLocalSettingsService_t *localSettingsService;
  MmiMaintenanceService_t *maintenanceService;
  osMemoryPoolId_t rxPool;
  osMemoryPoolId_t txPool;
  osMessageQueueId_t rxQueue;
  osMessageQueueId_t txQueue;
  uint8_t sessionId;
  LWIPSNMPAdapterCtx_t ntcipAdapter;
  uint8_t rxActive;
  uint8_t rxMessageClass;
  uint8_t rxSessionId;
  uint8_t rxTransferId;
  uint8_t rxNextSegmentIndex;
  uint16_t rxExpectedLength;
  uint8_t rxBuffer[512];
  uint8_t subscribeActive;
  uint8_t subscribeTransferId;
  uint8_t subscribeNextSegmentIndex;
  uint8_t subscribeBuffer[sizeof(MmiProtocolSubscribeRequestV2_t)];
  uint8_t publishTransferId;
  MmiRuntimeSubscription_t subscriptions[MMI_CAN_ADAPTER_SUBSCRIPTION_MAX];
  uint32_t rxDrops;
  uint32_t txDrops;
  uint32_t txErrors;
} MmiCanAdapterCtx_t;

void MmiCanAdapterInit(MmiCanAdapterCtx_t *ctx,
                       FDCAN_HandleTypeDef *hfdcan,
                       MmiService_t *service,
                       MmiSnapshotCache_t *snapshotCache);
void MmiCanAdapterBindActivationService(
  MmiCanAdapterCtx_t *ctx,
  IntersectionActivationService_t *activationService);
void MmiCanAdapterBindDetectorReportService(
  MmiCanAdapterCtx_t *ctx,
  DetectorReportService_t *detectorReportService);
void MmiCanAdapterBindGlobalTimeManagementService(
  MmiCanAdapterCtx_t *ctx,
  GlobalTimeManagementService_t *globalTimeManagementService);
void MmiCanAdapterBindDoorSensorPort(MmiCanAdapterCtx_t *ctx,
                                     IDoorSensorPort_t *doorSensorPort);
void MmiCanAdapterBindHeaterPort(MmiCanAdapterCtx_t *ctx,
                                 IHeaterPort_t *heaterPort);
void MmiCanAdapterBindPowerMonitorPort(MmiCanAdapterCtx_t *ctx,
                                       IPowerMonitorPort_t *powerMonitorPort);
void MmiCanAdapterBindUnitAlarmPort(MmiCanAdapterCtx_t *ctx,
                                    IUnitAlarmPort_t *unitAlarmPort);
void MmiCanAdapterBindUnitClockPort(MmiCanAdapterCtx_t *ctx,
                                    IUnitClockPort_t *unitClockPort);
void MmiCanAdapterBindCpMpLinkService(MmiCanAdapterCtx_t *ctx,
                                      CpMpLinkService_t *cpMpLinkService);
void MmiCanAdapterBindLocalSettingsService(
  MmiCanAdapterCtx_t *ctx,
  MmiLocalSettingsService_t *localSettingsService);
void MmiCanAdapterBindEventLogService(MmiCanAdapterCtx_t *ctx,
                                      MmiEventLogService_t *eventLogService);
void MmiCanAdapterBindMaintenanceService(MmiCanAdapterCtx_t *ctx,
                                         MmiMaintenanceService_t *service);
void MmiCanAdapterOnRxIsr(const FDCAN_RxHeaderTypeDef *header,
                          const uint8_t *data);
void MmiCanAdapterStep(void);
void MmiCanAdapterNotifyRuntimeTopic(uint8_t topicId, uint8_t recordIndex);

#endif /* MMI_CAN_ADAPTER_H */
