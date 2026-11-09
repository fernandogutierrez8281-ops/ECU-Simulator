#pragma once

#include <vector>
#include <string> 
#include <unordered_map>

#include "vehicleSimulator.hpp"
#include "sensor.hpp"

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

    public:
        // CORRECTO: Solo la firma, terminando en punto y coma.
        ECU(VehicleSimulator& simulator);

        ~ECU();

        void gateway();

        void control();

        void update();

        void performSelfTest();

        bool isStuck(Sensor* sensor) const;

        bool IsOutOFRanges(Sensor* sensor) const;

        bool checkSensorBuffer(Sensor* sensor) const;

        EcuStates getState();
};