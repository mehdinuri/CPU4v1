/* App/Platform/STM32/Services/UI/mmi.c
 *
 * Legacy producer compatibility shim.
 *
 * The old 0x7A7..0x7FF MMI request/response protocol is no longer active.
 * Residual legacy producers still call these hooks; they now only mark the
 * equivalent MMI v2 runtime topics dirty so subscribed external LCD clients
 * receive the canonical runtime view on their next publish cycle.
 */
#include "mmi.h"

#include "Adapters/STM32/MmiCanAdapter.h"
#include "DomainServices.h"

void OpenMMI(void)
{
}

void CloseMMI(void)
{
}

void StreamPSMMeasurements(void)
{
  MmiCanAdapterNotifyRuntimeTopic(MMI_PROTOCOL_V2_RUNTIME_TOPIC_POWER, 0U);
}

void StreamDateTime(void)
{
  MmiCanAdapterNotifyRuntimeTopic(MMI_PROTOCOL_V2_RUNTIME_TOPIC_CLOCK, 0U);
}

void StreamGPRSImei(void)
{
  MmiCanAdapterNotifyRuntimeTopic(MMI_PROTOCOL_V2_RUNTIME_TOPIC_COMMS, 0U);
}

void StreamUSRMAC(void)
{
  MmiCanAdapterNotifyRuntimeTopic(MMI_PROTOCOL_V2_RUNTIME_TOPIC_COMMS, 0U);
}

void StreamEthernetMAC(void)
{
  MmiCanAdapterNotifyRuntimeTopic(MMI_PROTOCOL_V2_RUNTIME_TOPIC_COMMS, 0U);
}

void StreamGsmOperator(void)
{
  MmiCanAdapterNotifyRuntimeTopic(MMI_PROTOCOL_V2_RUNTIME_TOPIC_COMMS, 0U);
}

void StreamGPRSState(void)
{
  MmiCanAdapterNotifyRuntimeTopic(MMI_PROTOCOL_V2_RUNTIME_TOPIC_COMMS, 0U);
}

void StreamOperationRuntime(void)
{
  MmiCanAdapterNotifyRuntimeTopic(MMI_PROTOCOL_V2_RUNTIME_TOPIC_SUMMARY, 0U);
  MmiCanAdapterNotifyRuntimeTopic(MMI_PROTOCOL_V2_RUNTIME_TOPIC_RINGS, 0U);
  MmiCanAdapterNotifyRuntimeTopic(MMI_PROTOCOL_V2_RUNTIME_TOPIC_PHASES, 0U);
  MmiCanAdapterNotifyRuntimeTopic(MMI_PROTOCOL_V2_RUNTIME_TOPIC_OVERLAPS, 0U);
}

void StreamModuleRuntime(void)
{
  MmiCanAdapterNotifyRuntimeTopic(MMI_PROTOCOL_V2_RUNTIME_TOPIC_MODULE_STATUS,
                                  0U);
  MmiCanAdapterNotifyRuntimeTopic(MMI_PROTOCOL_V2_RUNTIME_TOPIC_RELAY, 0U);
  MmiCanAdapterNotifyRuntimeTopic(MMI_PROTOCOL_V2_RUNTIME_TOPIC_COMMS, 0U);
}

void StreamGateStateChanged(uint8_t fState, tpSLogRecord pSLog)
{
  (void) fState;
  (void) pSLog;

  UiDoorServiceRefreshLatestLogIndices(&g_uiDoorService);
  MmiCanAdapterNotifyRuntimeTopic(MMI_PROTOCOL_V2_RUNTIME_TOPIC_DOOR, 0U);
}

void StreamErrorRuntime(void)
{
  MmiCanAdapterNotifyRuntimeTopic(MMI_PROTOCOL_V2_RUNTIME_TOPIC_SAFETY_SUMMARY,
                                  0U);
  MmiCanAdapterNotifyRuntimeTopic(MMI_PROTOCOL_V2_RUNTIME_TOPIC_SAFETY_CHANNELS,
                                  0U);
}

void StreamSignals(void)
{
  MmiCanAdapterNotifyRuntimeTopic(MMI_PROTOCOL_V2_RUNTIME_TOPIC_CHANNELS, 0U);
}

void StreamInputs(void)
{
  MmiCanAdapterNotifyRuntimeTopic(MMI_PROTOCOL_V2_RUNTIME_TOPIC_RAW_INPUTS, 0U);
  MmiCanAdapterNotifyRuntimeTopic(
    MMI_PROTOCOL_V2_RUNTIME_TOPIC_VEHICLE_DETECTORS,
    0U);
  MmiCanAdapterNotifyRuntimeTopic(
    MMI_PROTOCOL_V2_RUNTIME_TOPIC_PEDESTRIAN_DETECTORS,
    0U);
}

void StreamSOTest(void)
{
  MmiCanAdapterNotifyRuntimeTopic(MMI_PROTOCOL_V2_RUNTIME_TOPIC_OUTPUT_TEST,
                                  0U);
}
