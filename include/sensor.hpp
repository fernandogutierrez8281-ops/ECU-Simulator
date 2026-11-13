#pragma once

#include<random>
#include<chrono>
#include<string>
#include <optional>
#include <iostream>
#include <iomanip>

class Sensor{
    private:
        std::string sensorName;
        std::string sensorState = "VALID";

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

        std::vector<std::optional <float>> readingsBuffer = {0.0f, 0.0f, 0.0f};
        double noise = 0;

        uint32_t lastUpdateCicle = 0;

        float factor = 1.0f;
        float offset = 0.0f;

    public:
        Sensor(std::string name, std::string unit);

        virtual ~Sensor();

        void setSensorRange(float minimum, float maximum);

        std::string getName();
        
        float getMin();

        float getMax();

        void setCurrentReading(float reading);

        void setLastReadings(float reading);

        void updateReadingsBuffer(std::optional<float> newReading, uint32_t currentCycle);

        float getCurrentReading();
        
        void initializeRandomReadings(std::optional<unsigned int> seed = std::nullopt);

        void setNoiseRange(float minimum, float maximum);

        void initializeNoise(std::optional<unsigned int> seed = std::nullopt);

        void updateSelfNoise();

        virtual void simReading(bool hasNoise = true) {}
    
        virtual void simReading(float simpleSensorReading, float kp, bool hasNoise = true) {}
        
        const std::vector<std::optional<float>>& getReadingsBuffer() const;

        void setState(const std::string& state);

        std::string getState() const;

        std::string getUnit() const;

        uint32_t getLastUpdateCycle();

        void recoverCycle(uint32_t cycle);

        void setScaling(float factor, float offset);

        float getFactor() const;
        
        float getOffset() const;

        std::optional<uint16_t> getTransmittedRawReading();

        void clearCurrentReading();
};

/////////////////////////////////////////////////////////

class SimpleSensor : public Sensor {
    public:
        SimpleSensor(std::string name, std::string unit);

        ~SimpleSensor() override;

        void simReading(bool hasNoise = true) override;
};

//////////////////////////////////////////////////////////

class OpenLoopSensor : public Sensor {
    private:
        float setpoint = 100.0; // Desired velocity

        // Calculate change in velocity based on acceleration reading
        float changeInReading = 0.0;

    public:
        OpenLoopSensor(std::string name, std::string unit);

        ~OpenLoopSensor() override;

        void setSetpoint(float newSetpoint);
        
        void simReading(float simpleSensorReading, float kp, bool hasNoise = true) override;

};


