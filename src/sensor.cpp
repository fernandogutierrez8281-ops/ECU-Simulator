#include "sensor.hpp"

#include <iostream>
#include <algorithm>
#include <random>
#include <chrono>
#include <optional>
#include <string>

// Sensor class implementation

Sensor::Sensor(std::string name, std::string unit = "")
    : sensorName(name),
      unit(unit),
      min(0.0f),
      max(100.0f) {
}


Sensor::~Sensor() = default;

void Sensor::setSensorRange(float minimum, float maximum) {
    min = minimum;
    max = maximum;
}

std::string Sensor::getName()  { return sensorName; }

float Sensor::getMin() {
    return min;
}

float Sensor::getMax() {
    return max;
}

void Sensor::setCurrentReading(float reading) {
       readingsBuffer.assign(3, reading);
}

float Sensor::getCurrentReading() {
    return readingsBuffer.front().value_or(0.0f);
}

void Sensor::updateReadingsBuffer(std::optional<float> newReading, uint32_t currentCycle) {
    readingsBuffer[2] = readingsBuffer[1];
    readingsBuffer[1] = readingsBuffer[0];
    readingsBuffer[0] = newReading;
    
    if (newReading.has_value()) {
        // Solo actualizamos la marca temporal si llegó una muestra real
        this->lastUpdateCicle = currentCycle;
    }
}

void Sensor::setLastReadings(float reading) {
    readingsBuffer.assign(3, reading);
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

const std::vector<std::optional<float>>& Sensor::getReadingsBuffer() const {
    return readingsBuffer;
}

void Sensor::setState(const std::string& state) {
    sensorState = state;
}

std::string Sensor::getState() const {
    return sensorState;
}

std::string Sensor::getUnit() const {
    return unit;
}

uint32_t Sensor::getLastUpdateCycle() {
    return lastUpdateCicle;
}

void Sensor::recoverCycle(uint32_t cycle) {
    lastUpdateCicle = cycle;
}

void Sensor::setScaling(float factor, float offset) {
    this->factor = factor;
    this->offset = offset;
}

float Sensor::getFactor() const {
    return factor;
}

float Sensor::getOffset() const {
    return offset;
}

std::optional<uint16_t> Sensor::getTransmittedRawReading() {
    float physicalValue = getCurrentReading(); 
    
    if (factor == 0.0f) return std::nullopt; 

    float rawFloat = std::round((physicalValue - offset) / factor);
    
    rawFloat = std::clamp(rawFloat, 0.0f, 65535.0f);
    
    return static_cast<uint16_t>(rawFloat);
}

////////////////////////////////////////////////////////

SimpleSensor::SimpleSensor(std::string name, std::string unit = "")
    : Sensor(name, unit) {}

SimpleSensor::~SimpleSensor() = default;

void SimpleSensor::simReading(bool hasNoise) {

    if(hasNoise){
        updateSelfNoise();
        this->setCurrentReading(this->distribution(generator) + noise);
    } else {
        this->setCurrentReading(this->distribution(generator));
        
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
    float currentReading = readingsBuffer[1].value_or(0.0f) + changeInReading;

    // Ensure the current reading stays within the specified range

    if(hasNoise){
        updateSelfNoise();
        currentReading = std::clamp<double>(currentReading, static_cast<float>(min), static_cast<float>(max)) + noise; // Add noise to the reading
        this->setCurrentReading(currentReading);
    }else{

        currentReading = std::clamp<double>(currentReading, static_cast<float>(min), static_cast<float>(max));
        this->setCurrentReading(currentReading);
    }

}



