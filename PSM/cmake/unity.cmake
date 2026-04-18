# Unity test framework via FetchContent
# Included by root CMakeLists.txt only for Host-Test builds.

include(FetchContent)

FetchContent_Declare(
    unity
    GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity.git
    GIT_TAG        v2.6.0
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(unity)

# ---------------------------------------------------------------------------
# Add_Unity_Test(<name> <sources...>)
#   Creates a test executable linked to Unity and the PSM_Domain library,
#   then registers it with CTest.
# ---------------------------------------------------------------------------
function(Add_Unity_Test TEST_NAME)
    set(SOURCES ${ARGN})
    add_executable(${TEST_NAME} ${SOURCES})
    target_link_libraries(${TEST_NAME} PRIVATE
        unity
        PSM_Domain
    )
    target_include_directories(${TEST_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}/Tests/Fixtures
    )
    # Propagate coverage flags to test executables
    target_compile_options(${TEST_NAME} PRIVATE
        $<$<CONFIG:Debug>:-fprofile-arcs -ftest-coverage>
    )
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endfunction()
