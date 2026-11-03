#include "vehicleSimulator.hpp"
#include <iostream>

#include "sensor.hpp"

VehicleSimulator::VehicleSimulator(int seed)
    : seed(seed) {

        sensors.reserve(7); // Reserve space for 7 sensors

        sensors.emplace_back(new SimpleSensor("AccelerationInput", "%"));
        sensors.emplace_back(new OpenLoopSensor("Acceleration", "m/s^2"));
        sensors.emplace_back(new OpenLoopSensor("Velocity", "km/h"));   
        sensors.emplace_back(new OpenLoopSensor("Temperature", "C"));
        sensors.emplace_back(new OpenLoopSensor("RPM", "rpm"));
        sensors.emplace_back(new OpenLoopSensor("Battery", "V"));
        sensors.emplace_back(new OpenLoopSensor("Oil Pressure", "psi"));

        for (std::size_t i = 0; i < sensors.size(); ++i) {
            sensorMap.emplace(sensors[i]->getName(), i);
        }
};

VehicleSimulator::~VehicleSimulator() = default;

Sensor* VehicleSimulator::getSensor(const std::string& name) {
    auto it = sensorMap.find(name);
    if (it != sensorMap.end()) {
        return sensors[it->second];
    }
    return nullptr;
}

void VehicleSimulator::initializeSensors() {

    // Initialize the random number generator for the acceleration input
    this->getSensor("AccelerationInput")->setSensorRange(-5.0, 15.0); // Set initial reading for acceleration input
    this->getSensor("AccelerationInput")->initializeRandomReadings(seed);

    // based on the input change the acceleration
    this->getSensor("Acceleration")->setSensorRange(-20.0, 120.0);
    this->getSensor("Acceleration")->initializeRandomReadings(seed);

    // Set the range for the other sensors
    this->getSensor("Velocity")->setSensorRange(-10.0, 85.0);
    this->getSensor("Velocity")->setNoiseRange(-5.0,5.0);
    this->getSensor("Velocity")->initializeNoise(seed);

    this->getSensor("Temperature")->setSensorRange(-20.0, 110.0);
    this->getSensor("Temperature")->setNoiseRange(-5.0,5.0);
    this->getSensor("Temperature")->initializeNoise(seed);

    this->getSensor("RPM")->setSensorRange(700.0, 7000.0);
    this->getSensor("RPM")->setNoiseRange(-5.0,5.0);
    this->getSensor("RPM")->initializeNoise(seed);

    // Set the range and initial readings for the battery and oil pressure sensors
    this->getSensor("Battery")->setSensorRange(9.5, 12.0);
    this->getSensor("Battery")->setCurrentReading(12.0); // Set initial battery voltage
    this->getSensor("Battery")->setLastReadings(12.0); // Set initial battery voltage as last reading
    this->getSensor("Battery")->setNoiseRange(-0.05,0.05);
    this->getSensor("Battery")->initializeNoise(seed);

    
    this->getSensor("Oil Pressure")->setSensorRange(0.0, 100.0);
    this->getSensor("Oil Pressure")->setCurrentReading(50.0); // Set initial oil pressure
    this->getSensor("Oil Pressure")->setLastReadings(50.0); // Set initial oil pressure as last reading
    this->getSensor("Oil Pressure")->setNoiseRange(-5.0,5.0);
    this->getSensor("Oil Pressure")->initializeNoise(seed);
}

void VehicleSimulator::updateReadings() {
    
    // input to the accelerator
    this->getSensor("AccelerationInput")->simReading(); // Simulate acceleration input reading
    // simulate acceleration reading based on input
    this->getSensor("Acceleration")->simReading(this->getSensor("AccelerationInput")->getCurrentReading(), 0.12); // Simulate acceleration reading based on input
    // simulate velocity reading with noise
    this->getSensor("Velocity")->simReading(this->getSensor("Acceleration")->getCurrentReading(), 0.018); // Simulate velocity reading with noise
    // Simulate temperature reading with noise 
    this->getSensor("Temperature")->simReading(this->getSensor("Velocity")->getCurrentReading(), 0.03); // Simulate temperature reading with noise
    // Simulate RPM reading with noise
    this->getSensor("RPM")->simReading(this->getSensor("Velocity")->getCurrentReading(), 1.3); 
    // Simulate battery voltage reading with noise
    this->getSensor("Battery")->simReading(std::abs(this->getSensor("Velocity")->getCurrentReading()), -0.0001); 
    // Simulate oil pressure reading with noise
    this->getSensor("Oil Pressure")->simReading(this->getSensor("Velocity")->getCurrentReading(), 0.0001); 

}

void VehicleSimulator::showReadings() {
    // Display the current readings for each sensor
    this->getSensor("AccelerationInput")->showReading();
    this->getSensor("Acceleration")->showReading();
    this->getSensor("Velocity")->showReading();
    this->getSensor("Temperature")->showReading();
    this->getSensor("RPM")->showReading();
    this->getSensor("Battery")->showReading();
    this->getSensor("Oil Pressure")->showReading(); 
}