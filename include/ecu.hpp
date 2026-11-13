#pragma once

#include <vector>
#include <string> 
#include <unordered_map>

#include "vehicleSimulator.hpp"
#include "sensor.hpp"
#include "binaryLogger.hpp"

enum class EcuStates {
    INIT,
    SELF_TEST,
    OPERATIONAL,
    DEGRADED,
    SAFE_STATE,
    SHUTDOWN
};

class ECU {

    private: 

        int currentCycle = 0;
        EcuStates ecuState = EcuStates::INIT;

        VehicleSimulator& simulator;

        std::vector<Sensor*> activeSensors = {
            simulator.getSensor("AccelerationInput"),
            simulator.getSensor("Acceleration"),
            simulator.getSensor("Velocity"),
            simulator.getSensor("Temperature"),
            simulator.getSensor("RPM"),
            simulator.getSensor("Battery"),
            simulator.getSensor("Oil Pressure")
        };

        int invalidReadingsCount = 0;

        BinaryLogger logger{"binary_data_ecu.bin"};

    public:
        ECU(VehicleSimulator& simulator);

        ~ECU();

        void gateway();

        void control();

        void update();

        bool isStuck(Sensor* sensor) const;

        bool IsOutOFRanges(Sensor* sensor) const;

        bool checkSensorCycles(Sensor* sensor) const;

        void fetchAndDecodeReadings();

        VehicleSimulator& getSimulator();

        EcuStates getState();

        int getInvalidReadingsCount();

        std::vector<Sensor*> getActiveSensors() const;

        std::string getStateName() const;

        uint16_t getRawValue(const std::string& sensorName);

        bool hasCriticalFailure() const;

        bool hasMinorFailure() const;
};