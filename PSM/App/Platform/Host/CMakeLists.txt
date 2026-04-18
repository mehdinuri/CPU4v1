# psm_host_runner — interactive smoke-test binary for host builds.
# Primary test coverage comes from Tests/Unit/; this is for development.

add_executable(psm_host_runner main_host.c)

target_link_libraries(psm_host_runner
    PSM_Domain
    PSM_MockAdapters
)
