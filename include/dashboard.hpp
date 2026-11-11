#pragma once

#include <map>
#include <string>
#include <algorithm>

#include "ecu.hpp"


class Dashboard {
    private:
        ECU& ecu;

        std::map<EcuStates, std::string> stateNames = {
            {EcuStates::INIT, "INIT"},
            {EcuStates::SELF_TEST, "SELF_TEST"},
            {EcuStates::OPERATIONAL, "OPERATIONAL"},
            {EcuStates::DEGRADED, "DEGRADED"},
            {EcuStates::SAFE_STATE, "SAFE_STATE"},
            {EcuStates::SHUTDOWN, "SHUTDOWN"}
        };

        std::vector<Sensor*> activeSensors = {
            ecu.getSimulator().getSensor("AccelerationInput"),
            ecu.getSimulator().getSensor("Acceleration"),
            ecu.getSimulator().getSensor("Velocity"),
            ecu.getSimulator().getSensor("Temperature"),
            ecu.getSimulator().getSensor("RPM"),
            ecu.getSimulator().getSensor("Battery"),
            ecu.getSimulator().getSensor("Oil Pressure")
        };

    public:
        Dashboard(ECU& ecu);

        ~Dashboard();

        void buildSensorAsBar(Sensor* sensor);

        void showSensorReading(Sensor* sensor);

        void showAllSensorReadings();

        void ShowECUState();

        void showDashboard(int CurrentCycle);
};

