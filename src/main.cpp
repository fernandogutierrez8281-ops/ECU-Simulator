#include<iostream>

#include "sensor.hpp"
#include "vehicleSimulator.hpp"


int main(){

    float simtime = 100.0; // seconds
    float dt = 1; // seconds
    int seed = 55; // Seed for random number generator

    VehicleSimulator simulator(seed);
    simulator.initializeSensors();

    for (float t = 0; t < simtime; t += dt) {
        simulator.updateReadings();
        simulator.showReadings();
        std::cout << " " << std::endl;

    }

    return 0;
}


// g++ src/*.cpp -I include -o programa