# STM32 platform layer — composition root + FreeRTOS task wrappers.
# Linked by the STM32 firmware executable only.

add_library(ssm_platform_stm32 STATIC)

target_sources(ssm_platform_stm32 PRIVATE
    Bootstrap/main_ssm.c
    Tasks/can_msg_parser.c
    Tasks/can_msg_sender.c
    Tasks/measurement.c
    Tasks/storage.c
)

# Bootstrap/ and Tasks/ are PUBLIC so CubeMX sources (Core/Src/*) and adapter
# sources that include "can_msg_parser.h", "storage.h", etc. resolve them
# transitively through this library.
target_include_directories(ssm_platform_stm32
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/Bootstrap
            ${CMAKE_CURRENT_SOURCE_DIR}/Tasks
    PRIVATE ${CMAKE_SOURCE_DIR}/Core/Inc
)

target_link_libraries(ssm_platform_stm32 PUBLIC
    ssm_domain
    ssm_adapters_stm32
    stm32cubemx
)

target_compile_options(ssm_platform_stm32 PRIVATE
    -Wall -Wextra -Werror
    -Wno-unused-parameter
)
