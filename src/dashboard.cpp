

#include "dashboard.hpp"

Dashboard::Dashboard(ECU& ecu) : ecu(ecu) {
    // Constructor implementation
}

Dashboard::~Dashboard() {
    // Destructor implementation
}

void Dashboard::buildSensorAsBar(Sensor* sensor) {
    float normalizeReading = (sensor->getCurrentReading() - sensor->getMin())/(sensor->getMax() - sensor->getMin());
    int barrResolution = 10;

    std::cout << "[";
    for(int i = 0; i<= barrResolution; i++){
        if(i <= normalizeReading*barrResolution){
            std::cout << "#";
        } else {
            std::cout << "-";
        }
    }

     std::cout << "] ";

}

void Dashboard::showSensorReading(Sensor* sensor) {

    std::cout << std::fixed << std::setprecision(2);

    std::cout << std::left << std::setw(20) << sensor->getState();
    std::cout << std::left << std::setw(20) << sensor->getName();
    std::cout << std::right << std::setw(10) << sensor->getCurrentReading();
    std::cout << " " << std::left << std::setw(10) << sensor->getUnit();
    this->buildSensorAsBar(sensor);

    std::cout << "\n";
    
}

void Dashboard::showAllSensorReadings() {
    std::cout << "Sensor Readings:\n";
    std::cout << "----------------\n";

    for (auto sensor : activeSensors) {
        this->showSensorReading(sensor);
    }
}

void Dashboard::ShowECUState() {
    EcuStates currentState = ecu.getState();
    std::string stateName = stateNames[currentState];

    std::cout << "ECU State: " << stateName << "\n";
}

void Dashboard::showDashboard() {

    std::cout << "=============================================\n";
    std::cout << "             ECU VEHICLE DASHBOARD           \n";
    std::cout << "=============================================\n\n";

    std::cout << "====================\n";
    this->showAllSensorReadings();
    std::cout << "--------------------\n";
    this->ShowECUState();
    std::cout << "====================\n";

    std::cout << "Failed Sensors: " << std::to_string(ecu.getInvalidReadingsCount()) << "\n";
    std::cout << "=============================================\n";
}