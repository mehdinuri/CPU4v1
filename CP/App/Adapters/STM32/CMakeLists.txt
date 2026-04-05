# STM32H743 concrete port adapters.
# Each adapter implements exactly one port interface and may use FreeRTOS
# primitives internally, but never exposes them through the port interface.

add_library(Adapters_STM32 STATIC)

target_sources(Adapters_STM32 PRIVATE
    SignalCardAdapter.c
    DetectorInputAdapter.c
    RTCAdapter.c
    LWIPSNMPAdapter.c
    FlashStorageAdapter.c
    LCDAdapter.c
    KeypadAdapter.c
    ModemAdapter.c
)

target_include_directories(Adapters_STM32
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE ${CMAKE_SOURCE_DIR}/App/Ports
            ${CMAKE_SOURCE_DIR}/App/Domain
)

target_link_libraries(Adapters_STM32
    PUBLIC  Domain
    PRIVATE stm32cubemx
)
