#include <vector>
#include <cstdint>
#include <fstream>
#include <numeric>

#include "binaryLogger.hpp"

BinaryLogger::BinaryLogger(const std::string& filePath) {
    outputFile.open(filePath, std::ios::binary | std::ios::app);
}

BinaryLogger::~BinaryLogger() {
    if (outputFile.is_open()) {
        outputFile.close();
    }
}

void BinaryLogger::doRegister(uint32_t cycle, uint16_t rawTemperature, uint16_t rawRPM, uint8_t controlState) {
    std::vector<uint8_t> buffer;
    
    // Version (1 byte)
    buffer.push_back(1); 
    
    // Cycle (4 bytes, Little-Endian)
    buffer.push_back(cycle & 0xFF);
    buffer.push_back((cycle >> 8) & 0xFF);
    buffer.push_back((cycle >> 16) & 0xFF);
    buffer.push_back((cycle >> 24) & 0xFF);
    
    // Raw Temperature (2 bytes, Little-Endian)
    buffer.push_back(rawTemperature & 0xFF);
    buffer.push_back((rawTemperature >> 8) & 0xFF);
    
    // Raw RPM (2 bytes, Little-Endian)
    buffer.push_back(rawRPM & 0xFF);
    buffer.push_back((rawRPM >> 8) & 0xFF);
    
    // Control State (1 byte)
    buffer.push_back(controlState);
    
    // Checksum (1 byte, sum of all previous bytes modulo 256)
    uint8_t checksum = std::accumulate(buffer.begin(), buffer.end(), 0);
    buffer.push_back(checksum);
    
    // Write to file
    if (outputFile.is_open()) {
            outputFile.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
            outputFile.flush(); 
        }
}
