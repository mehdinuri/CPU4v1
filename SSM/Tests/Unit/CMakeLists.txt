# SSM unit tests — run on x86-64 via the Host-Test preset.
# Add_Unity_Domain_Test links against ssm_domain; Add_Unity_Test does not.

# Infrastructure smoke test: verifies the host test pipeline is operational.
Add_Unity_Test(Test_Smoke
    ${CMAKE_CURRENT_SOURCE_DIR}/Test_Smoke.c
)
