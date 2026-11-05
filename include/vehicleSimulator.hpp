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

        void updateReadings();

        void showReadings();



};

        






/**
# clean: remove build artifacts (.o objects, .elf/.hex/.bin, .a libraries);
#        keeps the CMake cache + Kconfig state, so no re-configure next build
cmake --build path/to/build-<p> --target clean

# re-configure: drop only CMakeCache.txt + CMakeFiles/, keep built artifacts,
#               force a fresh configure without a full wipe
cmake -B path/to/build-<p> --preset <b> --fresh -DPROJECT_DEFCONFIG=<p>/<v>.conf

# reset: remove the whole build dir (artifacts, cache, generated config,
#        IDE projects, compile database, logs)
rm -rf path/to/build-<p>


por ejemplo yo que uso `cmake -G Ninja` puedo hacer `ninja clean`


######

rm -rf build && cmake -B build -S . && cmake --build build

########

**/