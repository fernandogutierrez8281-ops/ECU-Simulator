#include "vehicleSimulator.hpp"
#include <iostream>

#include "sensor.hpp"

VehicleSimulator::VehicleSimulator(int seed)
    : seed(seed) {

        sensors.reserve(7); // Reserve space for 7 sensors

        sensors.emplace_back(std::make_unique<SimpleSensor>("AccelerationInput", "%"));
        sensors.emplace_back(std::make_unique<OpenLoopSensor>("Acceleration", "m/s^2"));
        sensors.emplace_back(std::make_unique<OpenLoopSensor>("Velocity", "km/h"));   
        sensors.emplace_back(std::make_unique<OpenLoopSensor>("Temperature", "C"));
        sensors.emplace_back(std::make_unique<OpenLoopSensor>("RPM", "rpm"));
        sensors.emplace_back(std::make_unique<OpenLoopSensor>("Battery", "V"));
        sensors.emplace_back(std::make_unique<OpenLoopSensor>("Oil Pressure", "psi"));

        for (std::size_t i = 0; i < sensors.size(); ++i) {
            sensorMap.emplace(sensors[i]->getName(), i);
        }
};

VehicleSimulator::~VehicleSimulator() = default;

Sensor* VehicleSimulator::getSensor(const std::string& name) {
    auto it = sensorMap.find(name);
    if (it != sensorMap.end()) {
        return sensors[it->second].get();
    }
    return nullptr;
}

void VehicleSimulator::initializeSensors() {

    auto accelInput = getSensor("AccelerationInput");
    auto accel = getSensor("Acceleration");
    auto velocity = getSensor("Velocity");
    auto temperature = getSensor("Temperature");
    auto rpm = getSensor("RPM");
    auto battery = getSensor("Battery");
    auto oilPressure = getSensor("Oil Pressure"); 

    // Initialize the random number generator for the acceleration input
    accelInput->setSensorRange(-5.0, 15.0);
    accelInput->initializeRandomReadings(seed);

    // based on the input change the acceleration
    accel->setSensorRange(-20.0, 120.0);
    accel->initializeRandomReadings(seed);

    // Set the range for the other sensors
    velocity->setSensorRange(-10.0, 85.0);
    velocity->setNoiseRange(-5.0,5.0);
    velocity->initializeNoise(seed);

    temperature->setSensorRange(-20.0, 110.0);
    temperature->setNoiseRange(-5.0,5.0);
    temperature->initializeNoise(seed);

    rpm->setSensorRange(700.0, 7000.0);
    rpm->setNoiseRange(-5.0,5.0);
    rpm->initializeNoise(seed);

    // Set the range and initial readings for the battery and oil pressure sensors
    battery->setSensorRange(9.5, 12.0);
    battery->setCurrentReading(12.0); // Set initial battery voltage
    battery->setLastReadings(12.0); // Set initial battery voltage as last reading
    battery->setNoiseRange(-0.05,0.05);
    battery->initializeNoise(seed);

    
    oilPressure->setSensorRange(0.0, 100.0);
    oilPressure->setCurrentReading(50.0); // Set initial oil pressure
    oilPressure->setLastReadings(50.0); // Set initial oil pressure as last reading
    oilPressure->setNoiseRange(-5.0,5.0);
    oilPressure->initializeNoise(seed);
}

void VehicleSimulator::updateReadings() {
    
    auto accelInput = getSensor("AccelerationInput");
    auto accel = getSensor("Acceleration");
    auto velocity = getSensor("Velocity");
    auto temp = getSensor("Temperature");
    auto rpm = getSensor("RPM");
    auto battery = getSensor("Battery");
    auto oil = getSensor("Oil Pressure");

    // 1. INPUT DEL CONDUCTOR (-5.0 Freno a 15.0 Acelerador)
    accelInput->simReading(); 
    float throttle = accelInput->getCurrentReading();

    // 2. ACELERACIÓN (Física: Potencia del motor - Fricción del aire)
    // El arrastre (drag) aumenta con la velocidad actual
    float drag = velocity->getCurrentReading() * 0.015f; 
    float currentAccel = (throttle * 0.8f) - drag; 
    accel->updateReadingsBuffer(currentAccel); // Valor directo, no se integra

    // 3. VELOCIDAD (Física: Integral de la Aceleración)
    // Aquí SÍ usamos tu simReading integrador. 
    // Si asumes un ciclo de simulación de dt=0.1s, para pasar m/s^2 a km/h multiplicas por 3.6 (kp = 0.36)
    velocity->simReading(accel->getCurrentReading(), 0.36f); 
    float currentVel = velocity->getCurrentReading();

    // 4. RPM (Física: Proporcional a la velocidad + Ralentí)
    // Simulando una marcha fija. Ralentí = 800 RPM. A 100 km/h = 4800 RPM.
    float currentRPM = 800.0f + (std::max(0.0f, currentVel) * 40.0f);
    rpm->updateReadingsBuffer(currentRPM); 

    // 5. TEMPERATURA (Física: Equilibrio térmico)
    // Generación de calor por RPM - Enfriamiento por aire (Velocidad)
    float currentTemp = temp->getCurrentReading();
    float calorGenerado = (currentRPM - 800.0f) * 0.00005f; 
    float enfriamiento = currentVel * 0.001f;
    
    // Tendencia natural a volver a 90°C (termostato del auto)
    float deltaTemp = calorGenerado - enfriamiento + ((90.0f - currentTemp) * 0.01f);
    temp->updateReadingsBuffer(currentTemp + deltaTemp);

    // 6. PRESIÓN DE ACEITE (Física: Depende exclusivamente de las RPM)
    // 20 psi base + ganancia por RPM. A 6000 RPM -> ~68 psi.
    float currentOil = 20.0f + (currentRPM * 0.008f);
    oil->updateReadingsBuffer(currentOil);

    // 7. VOLTAJE DE BATERÍA (Física: Carga del alternador)
    // Si el motor gira (RPM > 500), el alternador provee ~14V. Fluctúa muy levemente con las RPM.
    float currentBatt = (currentRPM > 500.0f) ? 14.0f : 12.0f;
    currentBatt += (currentRPM / 7000.0f) * 0.4f; // Sube hasta 14.4V a tope de RPM
    battery->updateReadingsBuffer(currentBatt);
}

void VehicleSimulator::showReadings() {
    for (const auto& sensor : sensors) {
        sensor->showReading();
    }
}