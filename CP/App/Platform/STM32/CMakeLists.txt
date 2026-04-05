# STM32 platform layer: thin FreeRTOS task wrappers + adapter wiring.
# Tasks call Domain init and Domain tick functions — no business logic here.

add_library(Platform_STM32 STATIC)

target_sources(Platform_STM32 PRIVATE
    main_stm32.c
    Tasks/ProgramTask.c
    Tasks/CANRxTask.c
    Tasks/CANTxTask.c
    Tasks/NetworkTask.c
    Tasks/GPSTask.c
    Tasks/UITask.c
    Tasks/StorageTask.c
    Tasks/TimeTask.c
    Tasks/MaintenanceTask.c
)

target_include_directories(Platform_STM32
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}
            ${CMAKE_CURRENT_SOURCE_DIR}/Tasks
    PRIVATE ${CMAKE_SOURCE_DIR}/App/Ports
            ${CMAKE_SOURCE_DIR}/App/Domain
            ${CMAKE_SOURCE_DIR}/App/Adapters/STM32
)

target_link_libraries(Platform_STM32
    PRIVATE Domain Adapters_STM32 stm32cubemx
)
