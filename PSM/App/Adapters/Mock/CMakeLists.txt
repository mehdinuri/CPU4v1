# PSM_MockAdapters — in-memory port implementations for host unit tests.
# Compiled only when BUILD_TARGET=Host.

add_library(PSM_MockAdapters STATIC)

target_sources(PSM_MockAdapters PRIVATE
    MockIndicatorLEDAdapter.c
    MockSignalInputAdapter.c
    MockEepromAdapter.c
    MockCANTxAdapter.c
    MockVoltageSensorAdapter.c
    MockFrequencySensorAdapter.c
)

target_include_directories(PSM_MockAdapters
    PUBLIC
        ${CMAKE_SOURCE_DIR}/App
        ${CMAKE_SOURCE_DIR}/App/Adapters/Mock
)
