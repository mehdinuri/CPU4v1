# Unity test framework with an offline-first local source preference.
# Included by root CMakeLists.txt only for Host-Test builds.

set(UNITY_VENDORED_SOURCE_DIR "${CMAKE_SOURCE_DIR}/../ThirdParty/unity")
set(UNITY_SOURCE_DIR "" CACHE PATH "Path to a local Unity source tree")
option(UNITY_FETCH_FROM_NETWORK
       "Allow CMake to fetch Unity from GitHub when no local copy is available"
       OFF)

function(_configure_unity_from_source SOURCE_DIR)
    add_library(unity STATIC
        "${SOURCE_DIR}/unity.c"
    )
    target_include_directories(unity PUBLIC
        "${SOURCE_DIR}"
    )
endfunction()

if(DEFINED UNITY_SOURCE_DIR
   AND NOT "${UNITY_SOURCE_DIR}" STREQUAL ""
   AND EXISTS "${UNITY_SOURCE_DIR}/unity.c"
   AND EXISTS "${UNITY_SOURCE_DIR}/unity.h")
    _configure_unity_from_source("${UNITY_SOURCE_DIR}")
elseif(EXISTS "${UNITY_VENDORED_SOURCE_DIR}/unity.c"
       AND EXISTS "${UNITY_VENDORED_SOURCE_DIR}/unity.h")
    _configure_unity_from_source("${UNITY_VENDORED_SOURCE_DIR}")
elseif(DEFINED FETCHCONTENT_SOURCE_DIR_UNITY
       AND EXISTS "${FETCHCONTENT_SOURCE_DIR_UNITY}/unity.c"
       AND EXISTS "${FETCHCONTENT_SOURCE_DIR_UNITY}/unity.h"
       AND NOT EXISTS "${FETCHCONTENT_SOURCE_DIR_UNITY}/CMakeLists.txt")
    _configure_unity_from_source("${FETCHCONTENT_SOURCE_DIR_UNITY}")
elseif(UNITY_FETCH_FROM_NETWORK)
    include(FetchContent)
    FetchContent_Declare(
        unity
        GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity.git
        GIT_TAG        v2.6.0
        GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(unity)
else()
    message(FATAL_ERROR
        "Unity source not found. Provide UNITY_SOURCE_DIR or use the vendored "
        "copy at ${UNITY_VENDORED_SOURCE_DIR}. To fetch from GitHub, configure "
        "with -DUNITY_FETCH_FROM_NETWORK=ON.")
endif()

# ---------------------------------------------------------------------------
# Add_Unity_Test(<name> <sources...>)
#   Creates a test executable linked to Unity and registers it with CTest.
#   Include directories: Core/Inc (for common headers), Tests/Fixtures.
# ---------------------------------------------------------------------------
function(Add_Unity_Test TEST_NAME)
    set(SOURCES ${ARGN})
    add_executable(${TEST_NAME} ${SOURCES})
    target_link_libraries(${TEST_NAME} PRIVATE unity)
    target_include_directories(${TEST_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}/Core/Inc
        ${CMAKE_SOURCE_DIR}/Tests/Fixtures
        ${CMAKE_SOURCE_DIR}/App
        ${CMAKE_SOURCE_DIR}/App/Domain
        ${CMAKE_SOURCE_DIR}/../Libs
    )
    # Propagate coverage flags to test executables
    target_compile_options(${TEST_NAME} PRIVATE
        $<$<CONFIG:Debug>:-fprofile-arcs -ftest-coverage>
    )
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    set_tests_properties(${TEST_NAME} PROPERTIES
        ENVIRONMENT "ASAN_OPTIONS=detect_leaks=0;LSAN_OPTIONS=detect_leaks=0")
endfunction()

# Add_Unity_Domain_Test(<name> <sources...>)
#   Variant of Add_Unity_Test that links against the mp_domain static
#   library so Domain-level tests don't need to re-list every source.
function(Add_Unity_Domain_Test TEST_NAME)
    set(SOURCES ${ARGN})
    Add_Unity_Test(${TEST_NAME} ${SOURCES})
    target_link_libraries(${TEST_NAME} PRIVATE mp_domain)
endfunction()
