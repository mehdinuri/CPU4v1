# SSM unit tests — run on x86-64 via the Host-Test preset.
# Add_Unity_Domain_Test links against ssm_domain; Add_Unity_Test does not.

# Infrastructure smoke test: verifies the host test pipeline is operational.
Add_Unity_Test(Test_Smoke
    ${CMAKE_CURRENT_SOURCE_DIR}/Test_Smoke.c
)

# Domain: SignalOutput.c — on/off-counter majority predicate.
Add_Unity_Domain_Test(Test_SignalOutput
    ${CMAKE_CURRENT_SOURCE_DIR}/Test_SignalOutput.c
)

# Domain + Mock adapter: SignalOutputImageBuilder.c end-to-end via MockSignalOutputAdapter.
Add_Unity_Domain_Test(Test_SignalOutputImageBuilder
    ${CMAKE_CURRENT_SOURCE_DIR}/Test_SignalOutputImageBuilder.c
)
target_link_libraries(Test_SignalOutputImageBuilder PRIVATE ssm_adapters_mock)

# Domain + Mock adapter: SignalFlashConfig persistence round-trip.
Add_Unity_Domain_Test(Test_SignalFlashConfig
    ${CMAKE_CURRENT_SOURCE_DIR}/Test_SignalFlashConfig.c
)
target_link_libraries(Test_SignalFlashConfig PRIVATE ssm_adapters_mock)

# Domain + Mock adapter: CurrentMeasurement pack + port integration.
Add_Unity_Domain_Test(Test_CurrentMeasurement
    ${CMAKE_CURRENT_SOURCE_DIR}/Test_CurrentMeasurement.c
)
target_link_libraries(Test_CurrentMeasurement PRIVATE ssm_adapters_mock)

# CanBus port contract exercised through MockCanBusAdapter.
Add_Unity_Domain_Test(Test_CanBusPort
    ${CMAKE_CURRENT_SOURCE_DIR}/Test_CanBusPort.c
)
target_link_libraries(Test_CanBusPort PRIVATE ssm_adapters_mock)

# Domain: SignalSampleAccumulator + input-port integration.
Add_Unity_Domain_Test(Test_SignalSampleAccumulator
    ${CMAKE_CURRENT_SOURCE_DIR}/Test_SignalSampleAccumulator.c
)
target_link_libraries(Test_SignalSampleAccumulator PRIVATE ssm_adapters_mock)

# Domain: CRC-32/ISO-HDLC table against standard test vectors.
Add_Unity_Domain_Test(Test_Crc32
    ${CMAKE_CURRENT_SOURCE_DIR}/Test_Crc32.c
)

# Domain: FlashSyncWatchdog deadman, including SysTick wraparound.
Add_Unity_Domain_Test(Test_FlashSyncWatchdog
    ${CMAKE_CURRENT_SOURCE_DIR}/Test_FlashSyncWatchdog.c
)
target_link_libraries(Test_FlashSyncWatchdog PRIVATE ssm_adapters_mock)

# Domain: OutputVerify debouncer + sticky fault flag.
Add_Unity_Domain_Test(Test_OutputVerify
    ${CMAKE_CURRENT_SOURCE_DIR}/Test_OutputVerify.c
)

# Domain: byte-explicit encoder for outgoing voltage/current CAN frame.
Add_Unity_Domain_Test(Test_VoltageCurrentFrame
    ${CMAKE_CURRENT_SOURCE_DIR}/Test_VoltageCurrentFrame.c
)
