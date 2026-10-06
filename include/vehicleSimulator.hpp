#pragma once

#include<random>
#include<chrono>
#include<string>
#include <optional>
#include <iostream>

#include "sensor.hpp"

class PIDcontroller {
    private:
        double kp;
        double ki;
        double kd;

        double error = 0.0;
        double previousError = 0.0;

        double output = 0.0;
        

    public:
        PIDcontroller(double kp, double ki, double kd);

        void compute(double setpoint, double measuredValue, double dt);

        void showOutput();
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