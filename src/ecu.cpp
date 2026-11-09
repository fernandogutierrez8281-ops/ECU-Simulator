#include <vector>
#include <string> 
#include <unordered_map>
#include <iostream>
#include <iomanip>
#include <algorithm>

#include "ecu.hpp"
#include "vehicleSimulator.hpp"


ECU::ECU(VehicleSimulator& simulator)
    : simulator(simulator) {
        performSelfTest();
    }

ECU::~ECU() = default;

bool ECU::IsOutOFRanges(Sensor* sensor) const {
    if (!sensor) return true; 

    const auto& buffer = sensor->getReadingsBuffer();
    float minLimit = sensor->getMin();
    float maxLimit = sensor->getMax();

    return std::any_of(buffer.begin(), buffer.end(), [minLimit, maxLimit](const std::optional<float>& val) {
        if (!val.has_value()) return true; 
        return val.value() < minLimit || val.value() > maxLimit; 
    });
}

bool ECU::isStuck(Sensor* sensor) const {
    if (!sensor) return true;
    const auto& buffer = sensor->getReadingsBuffer();
    
    if (buffer.size() < 3) return false;

    auto primerValor = buffer.front();
    
    // El lambda recibe el optional
    return std::all_of(buffer.begin(), buffer.end(), [primerValor](const std::optional<float>& val) {
        return val == primerValor; 
    });
}

bool ECU::checkSensorBuffer(Sensor* sensor) const {
    if (!sensor) return true; 

    const auto& buffer = sensor->getReadingsBuffer();

    bool hasFail = std::any_of(buffer.begin(), buffer.end(), [](const std::optional<float>& val) {
        return !val.has_value(); 
    });

    int zeroCount = std::count_if(buffer.begin(), buffer.end(), [](const std::optional<float>& val) {
        return val.has_value() && val.value() == 0.0f;
    });

    return hasFail || (zeroCount > 2);
}

void ECU::performSelfTest() {

    int invalidCount = std::count_if(activeSensors.begin(), activeSensors.end(), 
        [this](Sensor* s) {
            return this->IsOutOFRanges(s) || this->isStuck(s) || this->checkSensorBuffer(s); 
        }
    );

    if (invalidCount == 0) {
        ecuState = EcuStates::OPERATIONAL;
    } else if (invalidCount <= 2) {
        ecuState = EcuStates::DEGRADED;
    } else if (invalidCount <= 4) {
        ecuState = EcuStates::SAFE_STATE;
    } else {
        ecuState = EcuStates::SHUTDOWN;
    }
}

void ECU::update() {

    simulator.updateReadings();

    this->gateway();
    this->control();
}

void ECU::gateway() {

    invalidReadingsCount = std::count_if(activeSensors.begin(), activeSensors.end(), 
        [this](Sensor* s) {
            
            if (this->IsOutOFRanges(s) || this->isStuck(s)){
                s->setState("INVALID");
            }

            if (this->checkSensorBuffer(s)){
                s->setState("MISSING");
            }

            if (s->getState() != "INVALID" && s->getState() != "MISSING") {
                s->setState("VALID");
            }
            
            return this->IsOutOFRanges(s) || this->isStuck(s) || this->checkSensorBuffer(s); 
        }
    );
};

void ECU::control() {
    if (invalidReadingsCount == 0) {
        ecuState = EcuStates::OPERATIONAL;
    } else if (invalidReadingsCount <= 2) {
        ecuState = EcuStates::DEGRADED;
    } else if (invalidReadingsCount <= 4) {
        ecuState = EcuStates::SAFE_STATE;
    } else {
        ecuState = EcuStates::SHUTDOWN;
    }
};