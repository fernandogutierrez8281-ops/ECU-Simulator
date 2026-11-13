===============================================================================
                     ECU VEHICLE SIMULATION & GATEWAY SYSTEM
===============================================================================

DESCRIPTION
-----------
This project implements a software architecture for an Electronic Control Unit 
(ECU) and a Vehicle Simulator written in C++. It simulates dynamic physical 
vehicle parameters (Speed, RPM, Temperature, Oil Pressure, Battery Voltage), 
encodes/decodes signals over a virtual CAN bus using linear scaling, validates 
signal integrity in a Gateway module, manages system operating states via a 
Finite State Machine (FSM), and exports execution metrics using both CSV and 
binary logging with checksum verification.

-------------------------------------------------------------------------------

KEY FEATURES
------------
1. Vehicle Physics Simulator:
   - Dynamic simulation of vehicle parameters (Throttle, Acceleration, 
     Velocity, RPM, Temperature, Battery Voltage, Oil Pressure).
   - Support for open-loop integration and noise simulation.

2. CAN Bus Signal Scaling & Serialization:
   - Physical to binary raw value conversions using linear scaling (y = x * F + O).
   - Range clamping (0 to 65535) for 16-bit unsigned raw values.

3. Gateway Signal Validation:
   - Strict range and boundary checks (Min/Max thresholds).
   - Temporal plausibility & signal failure detection (MISSING state based on cycle age).

4. Finite State Machine (FSM):
   - Deterministic transitions across ECU states:
     INIT -> SELF_TEST -> OPERATIONAL / DEGRADED -> SAFE_STATE -> SHUTDOWN.

5. Data Logging & Persistence:
   - Human-readable CSV logging (`ecu_log.csv`).
   - Compact Little-Endian binary logging with byte-level checksum validation.

6. Automated Unit Testing Suite (Bloque 6):
   - Automated testing for boundary values, signal loss, signal recovery, 
     FSM state transitions, and binary encoding/decryption.

-------------------------------------------------------------------------------

PROJECT STRUCTURE
-----------------
.
├── CMakeLists.txt         # Build configuration file
├── README.txt             # Project documentation
├── include/               # Header files
│   ├── binaryLogger.hpp
│   ├── dashboard.hpp
│   ├── ecu.hpp
│   ├── logger.hpp
│   ├── sensor.hpp
│   └── vehicleSimulator.hpp
├── src/                   # Core C++ source files
│   ├── binaryLogger.cpp
│   ├── dashboard.cpp
│   ├── ecu.cpp
│   ├── logger.cpp
│   ├── main.cpp
│   ├── sensor.cpp
│   └── vehicleSimulator.cpp
└── tests/                 # Automated testing suite
    └── testGateway.cpp

-------------------------------------------------------------------------------

SENSOR SCALING AND BOUNDARIES SPECIFICATION
-------------------------------------------
+-------------------+---------+------------+------------+---------+--------+
| Sensor Name       | Unit    | Min Limit  | Max Limit  | Factor  | Offset |
+-------------------+---------+------------+------------+---------+--------+
| AccelerationInput | %       | -5.0       | 15.0       | 1.0     | 0.0    |
| Acceleration      | m/s^2   | -20.0      | 120.0      | 1.0     | 0.0    |
| Velocity          | km/h    | -10.0      | 85.0       | 1.0     | 0.0    |
| Temperature       | °C      | -20.0      | 110.0      | 0.1     | -40.0  |
| RPM               | rpm     | 700.0      | 7000.0     | 0.5     | 0.0    |
| Battery           | V       | 9.5        | 15.0       | 1.0     | 0.0    |
| Oil Pressure      | psi     | 0.0        | 100.0      | 0.1     | 0.0    |
+-------------------+---------+------------+------------+---------+--------+

-------------------------------------------------------------------------------

PREREQUISITES
-------------
- C++17 compatible compiler (g++, clang++, MSVC)
- CMake 3.14 or higher
- Make or Ninja build system

-------------------------------------------------------------------------------

COMPILATION & BUILDING INSTRUCTIONS
-----------------------------------

1. Create and configure the build directory:
   cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug

2. Build all executables (Main application and Test Suite):
   cmake --build build

-------------------------------------------------------------------------------

EXECUTION
---------

- Run the main vehicle simulation & live dashboard:
  ./build/programa

- Run the automated tests suite:
  ./build/testGateway

-------------------------------------------------------------------------------

BINARY PROTOCOL SPECIFICATION (LOGGING)
---------------------------------------
Each binary frame consists of 11 bytes structured as Little-Endian:

Byte 0:      Version Tag (1 byte) -> 0x01
Bytes 1-4:   Cycle Count (4 bytes, uint32_t)
Bytes 5-6:   Raw Temperature (2 bytes, uint16_t)
Bytes 7-8:   Raw RPM (2 bytes, uint16_t)
Byte 9:      Control State (1 byte, uint8_t)
Byte 10:     Checksum (1 byte, sum of bytes 0-9 modulo 256)

===============================================================================

FORMAT COMPARISON & STORAGE METRICS 

----------------------------------------------

The table below compares the CSV text logging format against the Custom Binary 

logging format using empirical measurement data gathered over 400 simulation cycles.

+-----------------------+--------------------------+--------------------------+

| Metric / Aspect       | CSV Logger (Text)        | Binary Logger (Custom)   |

+-----------------------+--------------------------+--------------------------+

| File Size (400 cycles)| 46,241 bytes (~46.2 KB)  | 4,400 bytes (~4.4 KB)    |

+-----------------------+--------------------------+--------------------------+

| Storage Efficiency    | Baseline (1.0x)          | ~10.5x size reduction    |

+-----------------------+--------------------------+--------------------------+

| Record Size           | Variable (~115.6 bytes)  | Fixed (Exactly 11 bytes) |

+-----------------------+--------------------------+--------------------------+

| Ease of Inspection    | High. Directly readable  | Low. Requires a hex      |

|                       | in plain text editors or | editor or a dedicated    |

|                       | spreadsheet tools.       | decoding program.        |

+-----------------------+--------------------------+--------------------------+

| Error Recovery &      | Low. Truncated or bad    | High. Single-byte corrupted|

| Integrity Validation  | lines can go unnoticed   | frames are caught by the |

|                       | without standard checks. | byte-level Checksum.     |

+-----------------------+--------------------------+--------------------------+



Test Environment: WSL (Debian) on an AMD Ryzen 5 5600X processor, compiled with GCC and CMake in Debug configuration.

Number of samples: The simulation of both ECUs was executed for 10,000 uninterrupted cycles to collect a representative volume of data.

Measurement methodology:

Storage: The operating system's file explorer was used to measure the exact disk size of the .csv file versus the .bin file after logging the same number of cycles.