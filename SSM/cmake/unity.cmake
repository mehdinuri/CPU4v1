# Unity test framework via FetchContent
# Included by root CMakeLists.txt only for Host-Test builds.

include(FetchContent)

if(DEFINED FETCHCONTENT_SOURCE_DIR_UNITY
   AND EXISTS "${FETCHCONTENT_SOURCE_DIR_UNITY}/unity.c"
   AND EXISTS "${FETCHCONTENT_SOURCE_DIR_UNITY}/unity.h"
   AND NOT EXISTS "${FETCHCONTENT_SOURCE_DIR_UNITY}/CMakeLists.txt")
    add_library(unity STATIC
        "${FETCHCONTENT_SOURCE_DIR_UNITY}/unity.c"
    )
    target_include_directories(unity PUBLIC
        "${FETCHCONTENT_SOURCE_DIR_UNITY}"
    )
else()
    FetchContent_Declare(
        unity
        GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity.git
        GIT_TAG        v2.6.0
        GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(unity)
endif()

# ---------------------------------------------------------------------------
# Add_Unity_Test(<name> <sources...>)
#   Creates a test executable linked to Unity and registers it with CTest.
#   Include directories: Core/Inc (shared types), Tasks/Inc, Tests/Fixtures.
# ---------------------------------------------------------------------------
function(Add_Unity_Test TEST_NAME)
    set(SOURCES ${ARGN})
    add_executable(${TEST_NAME} ${SOURCES})
    target_link_libraries(${TEST_NAME} PRIVATE unity)
    target_include_directories(${TEST_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}/Core/Inc
        ${CMAKE_SOURCE_DIR}/Tasks/Inc
        ${CMAKE_SOURCE_DIR}/Tests/Fixtures
        ${CMAKE_SOURCE_DIR}/App
        ${CMAKE_SOURCE_DIR}/App/Domain
    )
    target_compile_options(${TEST_NAME} PRIVATE
        $<$<CONFIG:Debug>:-fprofile-arcs -ftest-coverage>
    )
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endfunction()

# Add_Unity_Domain_Test(<name> <sources...>)
#   Variant of Add_Unity_Test that links against the ssm_domain static
#   library so Domain-level tests don't need to re-list every source.
function(Add_Unity_Domain_Test TEST_NAME)
    set(SOURCES ${ARGN})
    Add_Unity_Test(${TEST_NAME} ${SOURCES})
    target_link_libraries(${TEST_NAME} PRIVATE ssm_domain)
endfunction()
