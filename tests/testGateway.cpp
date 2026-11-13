#include <iostream>
#include <cassert>
#include <fstream>
#include <vector>
#include <cmath>

#include "sensor.hpp"
#include "vehicleSimulator.hpp"
#include "ecu.hpp"
#include "binaryLogger.hpp"

// FAKE: Substitutes the real simulator to inject precise values and bypass physics
class FakeSimulator : public VehicleSimulator {
public:
    FakeSimulator(int seed) : VehicleSimulator(seed) {}

    void updateReadings() override { }

    void forceSensorReading(const std::string& name, float value) {
        if (auto s = getSensor(name)) {
            s->setCurrentReading(value);
        }
    }
    
    void clearSensorReading(const std::string& name) {
        if (auto s = getSensor(name)) {
            s->clearCurrentReading();
        }
    }
};


// Validation of ranges and boundary values
void testRangeAndBoundaries() {
    FakeSimulator sim(55);
    sim.initializeSensors();
    ECU ecu(sim);

    // Test Oil Pressure boundaries (Range: 0.0 to 100.0)
    sim.forceSensorReading("Oil Pressure", 100.1f); // Out of upper boundary
    ecu.update();
    assert(sim.getSensor("Oil Pressure")->getState() == "INVALID");

    sim.forceSensorReading("Oil Pressure", 100.0f); // Maximum boundary
    ecu.update();
    assert(sim.getSensor("Oil Pressure")->getState() == "VALID");

    sim.forceSensorReading("Oil Pressure", 100.1f); // Out of upper boundary
    ecu.update();
    assert(sim.getSensor("Oil Pressure")->getState() == "INVALID");

    std::cout << "[OK] Range and boundary validation passed.\n";
}

// Signal age, disappearance, and recovery
void testSignalAgeAndRecovery() {
    FakeSimulator sim(55);
    sim.initializeSensors();
    ECU ecu(sim);

    sim.forceSensorReading("Temperature", 90.0f);
    ecu.update(); 
    assert(sim.getSensor("Temperature")->getState() == "VALID");

    sim.clearSensorReading("Temperature");

    for (int i = 0; i < 4; ++i) {
        ecu.update();
    }
    
    assert(sim.getSensor("Temperature")->getState() == "MISSING");
    std::cout << "[OK] Signal age and disappearance passed.\n";

    // Recuperación
    sim.forceSensorReading("Temperature", 92.0f);
    ecu.update(); 

    assert(sim.getSensor("Temperature")->getState() == "VALID");
    std::cout << "[OK] Signal recovery after absence passed.\n";
}

// State machine transitions
void testStateMachineTransitions() {
    FakeSimulator sim(55);
    sim.initializeSensors();
    ECU ecu(sim);

    // INIT -> SELF_TEST -> OPERATIONAL (Normal Scenario: all signals valid)
    ecu.update(); // Moves to SELF_TEST
    ecu.update(); // Moves to OPERATIONAL (if no failures)
    assert(ecu.getStateName() == "OPERATIONAL");

    // Non-critical scenario: Invalid Battery -> DEGRADED
    sim.forceSensorReading("Battery", 16.0f); // Over 15.0V max limit
    ecu.update();
    assert(ecu.getStateName() == "DEGRADED");

    // Critical scenario: Invalid Oil Pressure -> SAFE_STATE
    sim.forceSensorReading("Oil Pressure", 110.0f); // Over 100.0 psi max limit
    ecu.update();
    assert(ecu.getStateName() == "SAFE_STATE");

    std::cout << "[OK] State machine transitions passed.\n";
}

// Binary encoding/decoding and Read/Write records
void testBinarySerialization() {
    // 5. Test encoding/decoding logic (e.g., Temperature: y = xF + O)
    float originalTemp = 85.5f;
    float F = 0.1f;
    float O = -40.0f;
    
    // Encode: x = (y - O) / F
    uint16_t rawEncoded = static_cast<uint16_t>(std::round((originalTemp - O) / F));
    assert(rawEncoded == 1255); // Based on reference example
    
    // Decode: y = xF + O
    float decodedTemp = (rawEncoded * F) + O;
    assert(std::abs(decodedTemp - originalTemp) < 0.01f);
    std::cout << "[OK] Binary encoding and decoding passed.\n";

    // Test Write and Read of binary records
    std::string testFilename = "test_log.bin";
    
    // Delete file if exists from previous tests
    std::remove(testFilename.c_str()); 

    {
        // Write record using BinaryLogger
        BinaryLogger logger(testFilename);
        logger.doRegister(10, 1255, 3000, 3); // Cycle 10, Temp 1255, RPM 3000, State 3 (DEGRADED)
    } // Logger goes out of scope, file is closed and flushed

    {
        // Read record and verify integrity
        std::ifstream inFile(testFilename, std::ios::binary);
        assert(inFile.is_open());

        std::vector<uint8_t> buffer(std::istreambuf_iterator<char>(inFile), {});
        assert(buffer.size() == 11); // Version(1) + Cycle(4) + Temp(2) + RPM(2) + State(1) + Checksum(1)
        
        // Verify explicit Little-Endian decoding for Cycle (4 bytes)
        uint32_t readCycle = buffer[1] | (buffer[2] << 8) | (buffer[3] << 16) | (buffer[4] << 24);
        assert(readCycle == 10);
        
        // Verify Checksum
        uint8_t expectedChecksum = 0;
        for(size_t i = 0; i < buffer.size() - 1; ++i) {
            expectedChecksum += buffer[i];
        }
        assert(expectedChecksum == buffer.back());
    }
    
    std::cout << "[OK] Writing and reading of records passed.\n";
}

int main() {
    std::cout << "--- Starting Automated Tests (Bloque 6) ---\n";
    
    testRangeAndBoundaries();
    testSignalAgeAndRecovery();
    testStateMachineTransitions();
    testBinarySerialization();

    std::cout << "--- All tests passed successfully! ---\n";
    return 0;
}