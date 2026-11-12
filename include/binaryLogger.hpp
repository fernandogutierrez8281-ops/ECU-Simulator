#pragma once
#include <vector>
#include <cstdint>
#include <fstream>
#include <numeric>

class BinaryLogger {
private:
    std::ofstream outputFile;

public:
    BinaryLogger(const std::string& filePath);

    ~BinaryLogger();

    void doRegister(uint32_t cycle, uint16_t rawTemperature, uint16_t rawRPM, uint8_t controlState);
};