#include "vehicleSimulator.hpp"
#include <iostream>

#include "sensor.hpp"

VehicleSimulator::VehicleSimulator(int seed)
    : seed(seed) {

};

VehicleSimulator::~VehicleSimulator() = default;

void VehicleSimulator::initializeSensors() {

    // Initialize the random number generator for the acceleration input
    accelerationChange.setSensorRange(-5.0, 5.0);
    accelerationChange.initializeRandomReadings(seed);

    // based on the input change the acceleration
    acceleration.setSensorRange(-20.0, 120.0);
    acceleration.initializeRandomReadings(seed);

    // Set the range for the other sensors
    velocity.setSensorRange(-10.0, 85.0);
    velocity.setNoiseRange(-5.0,5.0);
    velocity.initializeNoise(seed);

    temperature.setSensorRange(-20.0, 110.0);
    temperature.setNoiseRange(-5.0,5.0);
    temperature.initializeNoise(seed);

    rpm.setSensorRange(700.0, 7000.0);
    rpm.setNoiseRange(-5.0,5.0);
    rpm.initializeNoise(seed);
    

    // Set the range and initial readings for the battery and oil pressure sensors
    battery.setSensorRange(9.5, 12.0);
    battery.setCurrentReading(12.0); // Set initial battery voltage
    battery.setLastReading(12.0); // Set initial battery voltage as last reading
    battery.setNoiseRange(-0.05,0.05);
    battery.initializeNoise(seed);

    oilPressure.setSensorRange(0.0, 100.0);
    oilPressure.setCurrentReading(50.0); // Set initial oil pressure
    oilPressure.setLastReading(50.0); // Set initial oil pressure as last reading
    oilPressure.setNoiseRange(-5.0,5.0);
    oilPressure.initializeNoise(seed);
}

void VehicleSimulator::updateReadings() {
    
    // input to the accelerator
    accelerationChange.simReading(); 
    // simulate acceleration reading based on input
    acceleration.simReading(accelerationChange.getCurrentReading(), 0.12);
    // simulate velocity reading with noise
    velocity.simReading(acceleration.getCurrentReading(), 0.018);
    // Simulate temperature reading with noise 
    temperature.simReading(velocity.getCurrentReading(), 0.03);
    // Simulate RPM reading with noise
    rpm.simReading(velocity.getCurrentReading(), 1.3); 
    // Simulate battery voltage reading with noise
    battery.simReading(std::abs(velocity.getCurrentReading()), -0.0001); 
    // Simulate oil pressure reading with noise
    oilPressure.simReading(velocity.getCurrentReading(), 0.0001); 

}

void VehicleSimulator::showReadings() {
    // Display the current readings for each sensor
    accelerationChange.showReading();
    acceleration.showReading();
    velocity.showReading();
    temperature.showReading();
    rpm.showReading();
    battery.showReading();
    oilPressure.showReading();
}