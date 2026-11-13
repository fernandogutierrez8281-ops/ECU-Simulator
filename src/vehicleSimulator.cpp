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
    auto temp = getSensor("Temperature");
    auto rpm = getSensor("RPM");
    auto battery = getSensor("Battery");
    auto oil = getSensor("Oil Pressure"); 

    // 1. INPUT DEL CONDUCTOR (El único con lecturas aleatorias base)
    accelInput->setSensorRange(-5.0, 15.0);
    accelInput->initializeRandomReadings(seed);
    accelInput->setLastReadings(0.0f); // Pedal suelto

    // 2. ACELERACIÓN Y VELOCIDAD
    accel->setSensorRange(-20.0, 120.0);
    accel->setLastReadings(0.0f); // Sin aceleración inicial

    velocity->setSensorRange(-10.0, 85.0);
    velocity->setNoiseRange(-1.5, 1.5); // Ruido más realista (5.0 era mucho brinco)
    velocity->initializeNoise(seed);
    velocity->setLastReadings(0.0f); // Vehículo detenido

    // 3. MOTOR (RPM y Temperatura)
    rpm->setSensorRange(700.0, 7000.0);
    rpm->setNoiseRange(-15.0, 15.0); 
    rpm->initializeNoise(seed);
    rpm->setLastReadings(800.0f); // Régimen de ralentí (idle)
    rpm->setScaling(0.5f, 0.0f);

    temp->setSensorRange(-20.0, 110.0);
    temp->setNoiseRange(-0.5, 0.5); 
    temp->initializeNoise(seed);
    temp->setLastReadings(90.0f); // Temperatura óptima de operación
    temp->setScaling(0.1f, -40.0f);

    // 4. PRESIÓN DE ACEITE (Física: 20 psi base + (800 RPM * 0.008) = 26.4 psi)
    oil->setSensorRange(0.0, 100.0);
    oil->setNoiseRange(-1.0, 1.0);
    oil->initializeNoise(seed);
    oil->setLastReadings(26.4f); 
    oil->setScaling(0.1f, 0.0f);

    // 5. BATERÍA (Física: Motor a 800 RPM = Alternador entregando ~14.0V)
    battery->setSensorRange(9.5, 15.0); // Ajustado a 15.0V para permitir los 14.4V de tu updateReadings
    battery->setNoiseRange(-0.05, 0.05);
    battery->initializeNoise(seed);
    battery->setLastReadings(14.0f); 
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
    accel->setCurrentReading(currentAccel); // Valor directo, no se integra

    // 3. VELOCIDAD (Física: Integral de la Aceleración)
    // Aquí SÍ usamos tu simReading integrador. 
    // Si asumes un ciclo de simulación de dt=0.1s, para pasar m/s^2 a km/h multiplicas por 3.6 (kp = 0.36)
    velocity->simReading(accel->getCurrentReading(), 0.36f); 
    float currentVel = velocity->getCurrentReading();

    // 4. RPM (Física: Proporcional a la velocidad + Ralentí)
    // Simulando una marcha fija. Ralentí = 800 RPM. A 100 km/h = 4800 RPM.
    float currentRPM = 800.0f + (std::max(0.0f, currentVel) * 40.0f);
    rpm->setCurrentReading(currentRPM); 

    // 5. TEMPERATURA (Física: Equilibrio térmico)
    // Generación de calor por RPM - Enfriamiento por aire (Velocidad)
    float currentTemp = temp->getCurrentReading();
    float calorGenerado = (currentRPM - 800.0f) * 0.00005f; 
    float enfriamiento = currentVel * 0.001f;
    
    // Tendencia natural a volver a 90°C (termostato del auto)
    float deltaTemp = calorGenerado - enfriamiento + ((90.0f - currentTemp) * 0.01f);
    temp->setCurrentReading(currentTemp + deltaTemp);

    // 6. PRESIÓN DE ACEITE (Física: Depende exclusivamente de las RPM)
    // 20 psi base + ganancia por RPM. A 6000 RPM -> ~68 psi.
    float currentOil = 20.0f + (currentRPM * 0.008f);
    oil->setCurrentReading(currentOil);

    // 7. VOLTAJE DE BATERÍA (Física: Carga del alternador)
    // Si el motor gira (RPM > 500), el alternador provee ~14V. Fluctúa muy levemente con las RPM.
    float currentBatt = (currentRPM > 500.0f) ? 14.0f : 12.0f;
    currentBatt += (currentRPM / 7000.0f) * 0.4f; // Sube hasta 14.4V a tope de RPM
    battery->setCurrentReading(currentBatt);
}
