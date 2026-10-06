#include<iostream>

#include "sensor.hpp"
#include "vehicleSimulator.hpp"


int main(){

    float simtime = 100.0; // seconds
    float dt = 1; // seconds
    int seed = 12345; // Seed for random number generator

    SimpleSensor noise("noise", " ");
    noise.setRange(-5.5, 5.5);
    noise.initializeRandomGenerator(seed);

    SimpleSensor accelerationSensor("Acceleration", "m/s^2");
    accelerationSensor.setRange(-8.0, 20.0);
    accelerationSensor.initializeRandomGenerator(seed);
    std::cout << "Initial Acceleration: " << std::endl;
    accelerationSensor.showReading();

    OpenLoopSensor velocitySensor("Velocity", "km/h");
    velocitySensor.setRange(0.0, 85.0);

    OpenLoopSensor temperatureSensor("Temperature", "°C");
    temperatureSensor.setRange(-20.0, 110.0);

    OpenLoopSensor rpm("RPM", "rpm");
    rpm.setRange(700.0, 7000.0);





    for (float t = 0; t < simtime; t += dt) {
        accelerationSensor.simReading(noise.getCurrentReading()); // Simulate acceleration reading with noise
        velocitySensor.simReading(accelerationSensor.getCurrentReading(), 0.05, noise.getCurrentReading()); // Example proportional gain
        temperatureSensor.simReading(velocitySensor.getCurrentReading(), 0.03, noise.getCurrentReading()); // Simulate temperature reading with noise
        rpm.simReading(velocitySensor.getCurrentReading(), 1.3, noise.getCurrentReading()*20); // Simulate RPM reading with noise

        accelerationSensor.showReading();
        velocitySensor.showReading();
        temperatureSensor.showReading();
        rpm.showReading();

    }

    return 0;
}


// g++ src/*.cpp -I include -o programa