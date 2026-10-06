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

        // Random number generator for simulating readings
        std::mt19937 generator;
        std::uniform_real_distribution<double> distribution;

        double currentReading = 0;

    public:
        Sensor(std::string name, std::string unit);

        virtual ~Sensor();

        void setRange(float minimum, float maximum);

        std::string getName();
        
        float getMin();

        float getMax();

        float getCurrentReading();

        bool isValidReading(float reading);
        
        void initializeRandomGenerator(std::optional<unsigned int> seed = std::nullopt);

        void showReading();
};

/////////////////////////////////////////////////////////

class SimpleSensor : public Sensor {
    public:
        SimpleSensor(std::string name, std::string unit);

        ~SimpleSensor() override;

        void simReading(float noise = 0.0);
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
        
        void simReading(float simpleSensorReading, float kp, float noise = 0.0);

        void showReading();

};


