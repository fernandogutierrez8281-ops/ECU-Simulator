cmake_minimum_required(VERSION 3.10)

project(ProyectoECU CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(include)

set(CORE_SOURCES
    src/binaryLogger.cpp
    src/dashboard.cpp
    src/ecu.cpp
    src/logger.cpp
    src/sensor.cpp
    src/vehicleSimulator.cpp
)

add_library(ecu_core ${CORE_SOURCES})
target_include_directories(ecu_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(programa src/main.cpp)

target_link_libraries(programa PRIVATE ecu_core)

enable_testing()

add_executable(testGateway tests/testGateway.cpp)
target_link_libraries(testGateway PRIVATE ecu_core)
add_test(NAME ValidacionGateway COMMAND testGateway)