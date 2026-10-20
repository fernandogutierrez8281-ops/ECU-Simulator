#include "sensor.hpp"

#include <iostream>
#include <algorithm>
#include <random>
#include <chrono>
#include <optional>
#include <string>

// Sensor class implementation

Sensor::Sensor(std::string name, std::string unit = "")
    : simulationName(name),
      unit(unit),
      min(0.0f),
      max(100.0f) {
}


Sensor::~Sensor() = default;

void Sensor::setSensorRange(float minimum, float maximum) {
    min = minimum;
    max = maximum;
}

std::string Sensor::getName()  { return simulationName; }

float Sensor::getMin() {
    return min;
}

float Sensor::getMax() {
    return max;
}

float Sensor::setCurrentReading(float reading) {
    if (isValidReading(reading)) {
        currentReading = reading;
    } else {
        std::cerr << "Error: Reading " << reading << " is out of range [" << min << ", " << max << "]" << std::endl;
    }
    return currentReading;
}

float Sensor::getCurrentReading() {
    return currentReading;
}

bool Sensor::isValidReading(float reading) {
    return (reading >= min && reading <= max);
}

void Sensor::initializeRandomReadings(std::optional<unsigned int> seed) {
    // Initialize the random number generator with a seed based on the current time
    std::random_device rd;
    if (seed.has_value()) {
        generator.seed(seed.value());
    } else {
        generator.seed(rd());
    }

    distribution = std::uniform_real_distribution<double>(min,max);

}

void Sensor::setNoiseRange(float minimum, float maximum){
    noiseMin = minimum;
    noiseMax = maximum;
}

void Sensor::initializeNoise(std::optional<unsigned int> seed){
    // Initialize the random number generator with a seed based on the current time
    std::random_device rd;
    if (seed.has_value()) {
        noiseGenerator.seed(seed.value());
    } else {
        noiseGenerator.seed(rd());
    }

    noiseDistribution = std::uniform_real_distribution<double>(noiseMin,noiseMax);
}

void Sensor::updateSelfNoise() {
    noise = noiseDistribution(noiseGenerator); 
}

void Sensor::showReading() {
    std::cout << getName() << " Reading: " << getCurrentReading() << " " << unit << std::endl;
}



////////////////////////////////////////////////////////

SimpleSensor::SimpleSensor(std::string name, std::string unit = "")
    : Sensor(name, unit) {}

SimpleSensor::~SimpleSensor() = default;

void SimpleSensor::simReading(bool hasNoise) {

    if(hasNoise){
        updateSelfNoise();
        currentReading = distribution(generator) +  noise;
    } else {
        currentReading = distribution(generator);
    }
    
};

/////////////////////////////////////////////////////////


OpenLoopSensor::OpenLoopSensor(std::string name, std::string unit)
    : Sensor(name, unit) {}

OpenLoopSensor::~OpenLoopSensor() = default;

void OpenLoopSensor::setSetpoint(float newSetpoint) {
    setpoint = newSetpoint;
}

void OpenLoopSensor::simReading(float simpleSensorReading, float kp, bool hasNoise) {

    // Calculate the change in reading based on the simple sensor's reading and the proportional gain
    changeInReading = simpleSensorReading * kp;

    // Update the current reading based on the change in reading
    currentReading = lastReading + changeInReading;

    // Ensure the current reading stays within the specified range

    if(hasNoise){
        updateSelfNoise();
        currentReading = std::clamp<double>(currentReading, static_cast<float>(min), static_cast<float>(max)) + noise; // Add noise to the reading
    }else{

        currentReading = std::clamp<double>(currentReading, static_cast<float>(min), static_cast<float>(max));
    }

    lastReading = currentReading; // Update last reading for the next iteration

}

void OpenLoopSensor::showReading() {
    std::cout << getName() << ": " << getCurrentReading() << " " << unit << std::endl;
}

void OpenLoopSensor::setLastReading(float reading) {
    lastReading = reading;
}



