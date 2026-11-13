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
    }

ECU::~ECU() = default;

bool ECU::IsOutOFRanges(Sensor* sensor) const {
    if (!sensor) return true; 

    const auto& buffer = sensor->getReadingsBuffer();
    float minLimit = sensor->getMin();
    float maxLimit = sensor->getMax();

    for (const auto& val : buffer) {
        if (val.has_value()) {
            return val.value() < minLimit || val.value() > maxLimit;
        }
    }
    
    return false; 
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

void ECU::update() {

    this->fetchAndDecodeReadings();

    this->gateway();
    this->control();

    uint16_t rawTemperature = getRawValue("Temperature");
    uint16_t rawRPM = getRawValue("RPM");
    uint8_t currentECUState = static_cast<uint8_t>(this->ecuState);

    logger.doRegister(currentCycle, rawTemperature, rawRPM, currentECUState);

    currentCycle++;
}

void ECU::gateway() {
    invalidReadingsCount = std::count_if(activeSensors.begin(), activeSensors.end(), 
        [this](Sensor* s) {
            
            bool isMissing = this->checkSensorCycles(s);
            
            bool outOfRange = this->IsOutOFRanges(s);

            if (isMissing) {
                s->setState("MISSING");
            } 
            else if (outOfRange) {
                s->setState("INVALID"); 
            } else {
                s->setState("VALID");
            }
            
            return outOfRange || isMissing; 
        }
    );
}

void ECU::control() {
    bool criticalFail = hasCriticalFailure();
    bool minorFailure = hasMinorFailure();
    bool noFail = !criticalFail && !minorFailure;

    switch (ecuState) {
        case EcuStates::INIT:
            ecuState = EcuStates::SELF_TEST;
            break;

        case EcuStates::SELF_TEST:
            if (noFail) {
                ecuState = EcuStates::OPERATIONAL;
            } else if (criticalFail) {
                ecuState = EcuStates::SAFE_STATE; 
            } else if (minorFailure || invalidReadingsCount > 2) {
                ecuState = EcuStates::DEGRADED; 
            }
            break;

        case EcuStates::OPERATIONAL:
            if (criticalFail) {
                ecuState = EcuStates::SAFE_STATE; 
            } else if (minorFailure || invalidReadingsCount > 2) {
                ecuState = EcuStates::DEGRADED;
            }
            break;

        case EcuStates::DEGRADED:
            if (criticalFail) {
                ecuState = EcuStates::SAFE_STATE; 
            } else if (noFail) {
                ecuState = EcuStates::OPERATIONAL; 
            }
            break;

        case EcuStates::SAFE_STATE:
            ecuState = EcuStates::SHUTDOWN;
            break;

        case EcuStates::SHUTDOWN:
            break;
            
        default:
            break;
    }
}

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
};

uint16_t ECU::getRawValue(const std::string& sensorName){
    for (Sensor* s : activeSensors) {
        if (s && s->getName() == sensorName) {
            std::optional<uint16_t> valor = s->getTransmittedRawReading();
            return valor.value_or(0); 
        }
    }
    return 0;
};

bool ECU::hasCriticalFailure() const {
    for (Sensor* s : activeSensors) {
        if (!s) continue;
        std::string name = s->getName();
        if ((name == "Temperature" || name == "Oil Pressure") && s->getState() != "VALID") {
            return true;
        }
    }
    return false;
}

bool ECU::hasMinorFailure() const {
    for (Sensor* s : activeSensors) {
        if (!s) continue;
        std::string name = s->getName();
        if ((name == "Velocity" || name == "RPM" || name == "Battery") && s->getState() != "VALID") {
            return true;
        }
    }
    return false;
}