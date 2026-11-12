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

    return std::any_of(buffer.begin(), buffer.end(), [](const std::optional<float>& val) {
        return !val.has_value(); 
    });
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

    this->fetchAndDecodeReadings();

    this->gateway();
    this->control();

    currentCycle++;
}

void ECU::gateway() {

    invalidReadingsCount = std::count_if(activeSensors.begin(), activeSensors.end(), 
        [this](Sensor* s) {
            
            bool isMissingByTime = this->checkSensorCycles(s);
            
            bool isMissingByBuffer = this->checkSensorBuffer(s);
            
            bool isMissing = isMissingByTime || isMissingByBuffer;

            bool outOfRangeOrStuck = this->IsOutOFRanges(s) || this->isStuck(s);

            if (isMissing) {
                s->setState("MISSING");
            } 
            else if (outOfRangeOrStuck) {
                s->setState("INVALID"); // Una señal presente pero fuera de rango será INVALID[cite: 7]
            } 
            else {
                s->setState("VALID");
            }
            
            return outOfRangeOrStuck || isMissing; 
        }
    );
}

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

VehicleSimulator& ECU::getSimulator() {
    return simulator;
}

EcuStates ECU::getState() {
    return ecuState;
}

int ECU::getInvalidReadingsCount() {
    return invalidReadingsCount;
}

std::vector<Sensor*> ECU::getActiveSensors() const {
    return activeSensors;
}

std::string ECU::getStateName() const {
    switch (ecuState) {
        case EcuStates::INIT: return "INIT";
        case EcuStates::SELF_TEST: return "SELF_TEST";
        case EcuStates::OPERATIONAL: return "OPERATIONAL";
        case EcuStates::DEGRADED: return "DEGRADED";
        case EcuStates::SAFE_STATE: return "SAFE_STATE";
        case EcuStates::SHUTDOWN: return "SHUTDOWN";
        default: return "UNKNOWN";
    }
}

bool ECU::checkSensorCycles(Sensor* sensor) const {
    if (!sensor) return true;

    constexpr uint32_t N_max = 3; 
    
    uint32_t ultimaRecepcion = sensor->getLastUpdateCycle(); 

    uint32_t deltaC = currentCycle - ultimaRecepcion;

    return deltaC > N_max; 
}

void ECU::fetchAndDecodeReadings() {
    for (Sensor* s : activeSensors) {
        if (!s) continue;

        std::optional<uint16_t> rawOpt = s->getTransmittedRawReading();
        
        std::optional<float> decodedValue = std::nullopt;

        if (rawOpt.has_value()) {
            uint16_t rawValue = rawOpt.value();
            float F = s->getFactor();
            float O = s->getOffset();
            
            // y = xF + O
            decodedValue = (rawValue * F) + O;
        }

        s->updateReadingsBuffer(decodedValue, this->currentCycle);
    }
}
