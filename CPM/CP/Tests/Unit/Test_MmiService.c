/*
 * Tests/Unit/Test_MmiService.c
 *
 * Protocol-v2 MMI resource routing and record-count resolution.
 */
#include "unity.h"

#include <string.h>

#include "Domain/Intersection/ConfigurationService.h"
#include "Domain/Services/MmiProtocol.h"
#include "Domain/Services/MmiService.h"

static MmiService_t s_service;
static ConfigurationService_t s_configurationService;

void setUp(void)
{
  IntersectionConfig_t config;

  memset(&s_service, 0, sizeof(s_service));
  memset(&s_configurationService, 0, sizeof(s_configurationService));
  memset(&config, 0, sizeof(config));

  IntersectionConfigInitDefaults(&config);
  config.phaseCount = 8U;
  config.ringCount = 2U;

  s_configurationService.activeConfig = config;

  MmiServiceInit(&s_service);
  MmiServiceBind(&s_service,
                 &s_configurationService,
                 NULL,
                 NULL,
                 NULL,
                 NULL,
                 NULL);
}

void tearDown(void)
{
}

void test_protocol_v2_can_ids_map_to_message_classes(void)
{
  MmiProtocolMessageClass_t messageClass;

  TEST_ASSERT_TRUE(
    MmiProtocolV2CanIdToMessageClass(MMI_PROTOCOL_V2_CAN_ID_COMMAND_SEG,
                                     &messageClass));
  TEST_ASSERT_EQUAL_UINT8(MMI_PROTOCOL_V2_MESSAGE_CLASS_COMMAND, messageClass);

  TEST_ASSERT_TRUE(
    MmiProtocolV2CanIdToMessageClass(MMI_PROTOCOL_V2_CAN_ID_HELLO_REQ,
                                     &messageClass));
  TEST_ASSERT_EQUAL_UINT8(MMI_PROTOCOL_V2_MESSAGE_CLASS_HELLO_REQUEST,
                          messageClass);

  TEST_ASSERT_FALSE(MmiProtocolV2CanIdToMessageClass(0x123U, &messageClass));
}

void test_lookup_runtime_summary_route_uses_engine_runtime_and_subscriptions(void)
{
  MmiResourceDescriptor_t descriptor;

  TEST_ASSERT_TRUE(MmiServiceLookupResource(&s_service,
                                            MMI_PROTOCOL_V2_NAMESPACE_RUNTIME,
                                            MMI_PROTOCOL_V2_RUNTIME_TOPIC_SUMMARY,
                                            &descriptor));
  TEST_ASSERT_EQUAL_UINT8(MMI_RESOURCE_SOURCE_ENGINE_RUNTIME,
                          descriptor.source);
  TEST_ASSERT_EQUAL_UINT8(1U, descriptor.readOnly);
  TEST_ASSERT_EQUAL_UINT8(1U, descriptor.supportsSubscription);
  TEST_ASSERT_EQUAL_UINT8(0U, descriptor.requiresTransaction);
  TEST_ASSERT_EQUAL_UINT16(sizeof(MmiRuntimeSummaryV2_t), descriptor.recordSize);
}

void test_lookup_standard_object_route_requires_transaction(void)
{
  MmiResourceDescriptor_t descriptor;

  TEST_ASSERT_TRUE(MmiServiceLookupResource(
    &s_service,
    MMI_PROTOCOL_V2_NAMESPACE_STANDARD_OBJECT,
    MMI_PROTOCOL_V2_STANDARD_RESOURCE_NTCIP_OBJECT,
    &descriptor));
  TEST_ASSERT_EQUAL_UINT8(MMI_RESOURCE_SOURCE_NTCIP_OBJECT_DIRECTORY,
                          descriptor.source);
  TEST_ASSERT_EQUAL_UINT8(1U, descriptor.requiresTransaction);
  TEST_ASSERT_EQUAL_UINT8(0U, descriptor.supportsSubscription);
  TEST_ASSERT_EQUAL_UINT16(0U, descriptor.recordSize);
}

void test_resolve_runtime_counts_from_configuration_service(void)
{
  MmiResourceDescriptor_t descriptor;
  uint16_t count;

  TEST_ASSERT_TRUE(MmiServiceLookupResource(&s_service,
                                            MMI_PROTOCOL_V2_NAMESPACE_RUNTIME,
                                            MMI_PROTOCOL_V2_RUNTIME_TOPIC_RINGS,
                                            &descriptor));
  TEST_ASSERT_TRUE(MmiServiceResolveRecordCount(&s_service, &descriptor, &count));
  TEST_ASSERT_EQUAL_UINT16(2U, count);

  TEST_ASSERT_TRUE(MmiServiceLookupResource(
    &s_service,
    MMI_PROTOCOL_V2_NAMESPACE_RUNTIME,
    MMI_PROTOCOL_V2_RUNTIME_TOPIC_CHANNELS,
    &descriptor));
  TEST_ASSERT_TRUE(MmiServiceResolveRecordCount(&s_service, &descriptor, &count));
  TEST_ASSERT_EQUAL_UINT16(INTERSECTION_CHANNEL_COUNT_MAX, count);

  TEST_ASSERT_TRUE(MmiServiceLookupResource(
    &s_service,
    MMI_PROTOCOL_V2_NAMESPACE_RUNTIME,
    MMI_PROTOCOL_V2_RUNTIME_TOPIC_VEHICLE_DETECTORS,
    &descriptor));
  TEST_ASSERT_TRUE(MmiServiceResolveRecordCount(&s_service, &descriptor, &count));
  TEST_ASSERT_EQUAL_UINT16(INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX, count);
}

void test_vendor_private_resource_maps_to_cpmp_diagnostics(void)
{
  MmiResourceDescriptor_t descriptor;

  TEST_ASSERT_TRUE(MmiServiceLookupResource(
    &s_service,
    MMI_PROTOCOL_V2_NAMESPACE_VENDOR_PRIVATE,
    MMI_PROTOCOL_V2_VENDOR_RESOURCE_CPMP_FAULT_CHANNELS,
    &descriptor));
  TEST_ASSERT_EQUAL_UINT8(MMI_RESOURCE_SOURCE_VENDOR_DIAGNOSTICS,
                          descriptor.source);
  TEST_ASSERT_EQUAL_UINT8(1U, descriptor.supportsSubscription);
  TEST_ASSERT_EQUAL_UINT16(sizeof(MmiRuntimeSafetyChannelRecordV2_t),
                           descriptor.recordSize);
}

void test_local_settings_resources_include_grouped_clock_and_admin_change(void)
{
  MmiResourceDescriptor_t descriptor;

  TEST_ASSERT_TRUE(MmiServiceLookupResource(
    &s_service,
    MMI_PROTOCOL_V2_NAMESPACE_LOCAL_SETTINGS,
    MMI_PROTOCOL_V2_LOCAL_RESOURCE_CLOCK_SETTINGS,
    &descriptor));
  TEST_ASSERT_EQUAL_UINT8(MMI_RESOURCE_SOURCE_LOCAL_SETTINGS,
                          descriptor.source);
  TEST_ASSERT_EQUAL_UINT8(0U, descriptor.readOnly);
  TEST_ASSERT_EQUAL_UINT16(sizeof(MmiLocalClockSettingsV2_t),
                           descriptor.recordSize);

  TEST_ASSERT_TRUE(MmiServiceLookupResource(
    &s_service,
    MMI_PROTOCOL_V2_NAMESPACE_LOCAL_SETTINGS,
    MMI_PROTOCOL_V2_LOCAL_RESOURCE_ADMIN_PASSWORD_CHANGE,
    &descriptor));
  TEST_ASSERT_EQUAL_UINT8(MMI_RESOURCE_SOURCE_LOCAL_SETTINGS,
                          descriptor.source);
  TEST_ASSERT_EQUAL_UINT8(0U, descriptor.readOnly);
  TEST_ASSERT_EQUAL_UINT16(sizeof(MmiLocalAdminPasswordChangeV2_t),
                           descriptor.recordSize);
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_protocol_v2_can_ids_map_to_message_classes);
  RUN_TEST(test_lookup_runtime_summary_route_uses_engine_runtime_and_subscriptions);
  RUN_TEST(test_lookup_standard_object_route_requires_transaction);
  RUN_TEST(test_resolve_runtime_counts_from_configuration_service);
  RUN_TEST(test_vendor_private_resource_maps_to_cpmp_diagnostics);
  RUN_TEST(test_local_settings_resources_include_grouped_clock_and_admin_change);
  return UNITY_END();
}
