#include <chrono>
#include <iostream>
#include <iomanip>

#include "logger.hpp"
#include "dashboard.hpp"

Logger::Logger(const std::string& fileName) {

    file.open(fileName, std::ios::out | std::ios::app);

    if (!file.is_open()) {
        std::cerr << "Error opening log file: " << fileName << std::endl;
    }


    file << "Timestamp,"
         <<  "Cycle,"
         <<  "throttle,"
         <<  "acceleration,"
         <<  "Velocity,"
         <<  "Temperature,"
         <<  "RPM,"
         <<  "Battery,"
         <<  "Oil Pressure,"
         <<  "ECU State" << std::endl;;

}

Logger::~Logger() {
    if (file.is_open()) {
        file.close();
    }
}

void Logger::updateFile(const ECU& ecu, int cicle) {
    if (!file.is_open()) {
        std::cerr << "Log file is not open." << std::endl;
        return;
    }

    auto now = std::chrono::system_clock::now();
    auto now_time_t = std::chrono::system_clock::to_time_t(now);
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    file << std::put_time(std::localtime(&now_time_t), "%Y-%m-%d %H:%M:%S")
         << "." << std::setfill('0') << std::setw(3) << now_ms.count() << ","
         << cicle << ",";

    for (auto sensor : ecu.getActiveSensors()) {
        file << sensor->getCurrentReading() << ",";
    }

    std::string currentState = ecu.getStateName();
    file << currentState << std::endl;;
}



