cmake_minimum_required(VERSION 3.22)

# ---------------------------------------------------------------------------
# FORMAT TARGET — uncrustify user code (MISRA-C style), skips ST libraries
# Available on all presets:
#   cmake --build <build-dir> --target Format        # reFormat in-place
#   cmake --build <build-dir> --target Format-Check  # verify (CI-safe, no writes)
# ---------------------------------------------------------------------------
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
        COMMENT "Checking user code Formatting (MISRA-C)"
        VERBATIM
    )
else()
    message(STATUS "Uncrustify     : not found — 'Format'/'Format-Check' targets unavailable")
    message(STATUS "                 Install: sudo apt install uncrustify")
endif()

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

set(CMAKE_PROJECT_NAME CP)
project(${CMAKE_PROJECT_NAME})
message(STATUS "Build target : ${BUILD_TARGET}")
message(STATUS "Build type   : ${CMAKE_BUILD_TYPE}")

# ---------------------------------------------------------------------------
# HOST BUILD — Domain + Mock Adapters + Unit/Integration Tests (x86-64)
# ---------------------------------------------------------------------------
if(BUILD_TARGET STREQUAL "Host")
    enable_language(C)
    enable_testing()

    include(cmake/unity.cmake)

    add_subdirectory(App/Domain)
    add_subdirectory(App/Ports)
    add_subdirectory(App/Adapters/Mock)
    add_subdirectory(Tests)
    add_subdirectory(App/Platform/Host)

# ---------------------------------------------------------------------------
# STM32 BUILD — full firmware for STM32H743VIT6
# ---------------------------------------------------------------------------
else()
    enable_language(C ASM)

    # Main firmware executable (must be created before subdirs that add sources)
    add_executable(${CMAKE_PROJECT_NAME})

    # CubeMX-generated HAL, middleware, and application sources (untouched)
    add_subdirectory(cmake/stm32cubemx)

    # Application layers
    add_subdirectory(App/Domain)
    add_subdirectory(App/Ports)
    add_subdirectory(App/Adapters/STM32)
    add_subdirectory(App/Platform/STM32)

    target_link_libraries(${CMAKE_PROJECT_NAME}
        stm32cubemx          # HAL + middleware interface (includes/defines)
        Domain
        Adapters_STM32
        Platform_STM32
    )

    target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE)
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE)
    target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE)
    target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE)

    list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

    set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES
        ADDITIONAL_CLEAN_FILES ${CMAKE_PROJECT_NAME}.map)
endif()
