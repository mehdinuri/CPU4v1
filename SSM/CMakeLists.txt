cmake_minimum_required(VERSION 3.22)

# ---------------------------------------------------------------------------
# FORMAT TARGET — uncrustify user code (MISRA-C style), skips ST libraries
# Available on all presets:
#   cmake --build <build-dir> --target Format        # reformat in-place
#   cmake --build <build-dir> --target Format-Check  # verify (CI-safe, no writes)
# ---------------------------------------------------------------------------

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

# Enable compile command export for clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Build target: STM32 (default) or Host (for unit testing on x86)
if(NOT DEFINED BUILD_TARGET)
    set(BUILD_TARGET "STM32")
endif()

set(CMAKE_PROJECT_NAME SSM)
project(${CMAKE_PROJECT_NAME})
message(STATUS "Build target : ${BUILD_TARGET}")
message(STATUS "Build type   : ${CMAKE_BUILD_TYPE}")

find_program(UNCRUSTIFY_EXECUTABLE uncrustify)
if(UNCRUSTIFY_EXECUTABLE)
    message(STATUS "Uncrustify     : ${UNCRUSTIFY_EXECUTABLE}")
    add_custom_target(Format
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/Tools/Scripts/format-user-code.sh
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Formatting user code with Uncrustify (MISRA-C)"
        VERBATIM
    )
    add_custom_target(Format-Check
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/Tools/Scripts/format-user-code.sh --check
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Checking user code formatting (MISRA-C)"
        VERBATIM
    )
else()
    message(STATUS "Uncrustify     : not found — 'Format'/'Format-Check' targets unavailable")
    message(STATUS "                 Install: sudo apt install uncrustify")
endif()

# ---------------------------------------------------------------------------
# HOST BUILD — Unity unit/integration tests (x86-64, no hardware needed)
# ---------------------------------------------------------------------------
if(BUILD_TARGET STREQUAL "Host")
    enable_language(C)
    enable_testing()

    include(cmake/unity.cmake)

    add_subdirectory(App/Domain)
    add_subdirectory(App/Adapters/Mock)
    add_subdirectory(Tests)

# ---------------------------------------------------------------------------
# STM32 BUILD — full firmware for STM32G473
# ---------------------------------------------------------------------------
else()
    enable_language(C ASM)

    # Main firmware executable (must be created before subdirs that add sources)
    add_executable(${CMAKE_PROJECT_NAME})

    # CubeMX-generated HAL, FreeRTOS, and all application sources
    add_subdirectory(cmake/stm32cubemx)

    # Domain layer (pure computation, no HAL/FreeRTOS)
    add_subdirectory(App/Domain)

    # STM32 adapters (HAL implementations of App/Ports/*)
    add_subdirectory(App/Adapters/STM32)

    # Link directories setup
    target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
        # Add user defined library search paths
    )

    # Add sources to executable
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE
        Core/Src/flash.c
        Core/Src/utilities.c
        Tasks/Src/can_msg_parser.c
        Tasks/Src/can_msg_sender.c
        Tasks/Src/measurement.c
        Tasks/Src/storage.c
    )

    # Add include paths
    target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
        Tasks/Inc
    )

    # Add project symbols (macros)
    target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
        # Add user defined symbols
    )

    # Remove wrong libob.a library dependency when using cpp files
    list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

    # Add linked libraries
    target_link_libraries(${CMAKE_PROJECT_NAME}
        stm32cubemx
        ssm_domain
        ssm_adapters_stm32
        # Add user defined libraries
    )

    set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES
        ADDITIONAL_CLEAN_FILES ${CMAKE_PROJECT_NAME}.map)
endif()
