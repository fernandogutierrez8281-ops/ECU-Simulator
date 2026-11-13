#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include<random>
#include<chrono>
#include<string>
#include <optional>
#include <iostream>
#include <memory>

#include "sensor.hpp"

class VehicleSimulator {
    private:
        int seed;

        std::vector<std::unique_ptr<Sensor>> sensors;
        std::unordered_map<std::string, std::size_t> sensorMap;

        
    public:
        VehicleSimulator(int seed);

        ~VehicleSimulator();

        Sensor* getSensor(const std::string& name);

        void initializeSensors();

        virtual void updateReadings();

};
