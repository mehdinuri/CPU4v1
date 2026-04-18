# Mock adapters — test doubles for App/Ports/*
# Linked by host unit tests only (Host-Test preset).

add_library(ssm_adapters_mock STATIC)

target_sources(ssm_adapters_mock PRIVATE
    MockSignalOutputAdapter.c
    MockSignalInputAdapter.c
    MockWatchdogAdapter.c
    MockTickAdapter.c
    MockPersistenceAdapter.c
    MockCurrentMeasurementAdapter.c
    MockCanBusAdapter.c
    MockTimerAdapter.c
)

target_include_directories(ssm_adapters_mock PUBLIC
    ${CMAKE_SOURCE_DIR}/App
)

target_link_libraries(ssm_adapters_mock PUBLIC
    ssm_domain
)

target_compile_options(ssm_adapters_mock PRIVATE
    -Wall -Wextra -Werror
)
