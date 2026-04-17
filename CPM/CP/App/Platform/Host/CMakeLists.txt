# Host platform — minimal entry point that exercises mock-wired Domain.
# Used for manual smoke testing outside of the CTest test suite.

add_executable(cp_host_runner main_host.c)

target_link_libraries(cp_host_runner PRIVATE
    Domain
    Mock_Adapters
)

target_include_directories(cp_host_runner PRIVATE
    ${CMAKE_SOURCE_DIR}/App/Ports
)
