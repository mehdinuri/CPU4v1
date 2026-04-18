# Unit tests — pure Domain logic with no HAL/FreeRTOS dependencies.

Add_Unity_Test(Test_Measurement        Test_Measurement.c)
Add_Unity_Test(Test_MeasurementService Test_MeasurementService.c)

# MeasurementService tests need mock adapters
target_link_libraries(Test_MeasurementService PRIVATE PSM_MockAdapters)
target_include_directories(Test_MeasurementService PRIVATE
    ${CMAKE_SOURCE_DIR}/App/Ports
    ${CMAKE_SOURCE_DIR}/App/Adapters/Mock
)
