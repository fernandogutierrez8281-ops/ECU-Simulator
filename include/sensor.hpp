#pragma once

#include<random>
#include<chrono>
#include<string>
#include <optional>
#include <iostream>

class Sensor{
    private:
        std::string simulationName;
        std::string simulationType;

    protected:
        std::string unit;
        double min = 0.0f;
        double max = 100.0f;

        double noiseMin = -1;
        double noiseMax = 1;

        // Random number generator for simulating readings
        std::mt19937 generator;
        std::uniform_real_distribution<double> distribution;

        // Noise genetator
        std::mt19937 noiseGenerator;
        std::uniform_real_distribution<double> noiseDistribution;


        double currentReading = 0;
        double noise = 0;

    public:
        Sensor(std::string name, std::string unit);

        virtual ~Sensor();

        void setSensorRange(float minimum, float maximum);

        std::string getName();
        
        float getMin();

        float getMax();

        float setCurrentReading(float reading);

        float getCurrentReading();

        bool isValidReading(float reading);
        
        void initializeRandomReadings(std::optional<unsigned int> seed = std::nullopt);

        void setNoiseRange(float minimum, float maximum);

        void initializeNoise(std::optional<unsigned int> seed = std::nullopt);

        void updateSelfNoise();

        void showReading();
        
        
};

/////////////////////////////////////////////////////////

class SimpleSensor : public Sensor {
    public:
        SimpleSensor(std::string name, std::string unit);

        ~SimpleSensor() override;

        void simReading(bool hasNoise = true);
};

//////////////////////////////////////////////////////////

class OpenLoopSensor : public Sensor {
    private:
        float setpoint = 100.0; // Desired velocity

        float lastReading = 0.0;

        // Calculate change in velocity based on acceleration reading
        float changeInReading = 0.0;

    public:
        OpenLoopSensor(std::string name, std::string unit);

        ~OpenLoopSensor() override;

        void setSetpoint(float newSetpoint);
        
        void simReading(float simpleSensorReading, float kp, bool hasNoise = true);

        void setLastReading(float reading);

};


