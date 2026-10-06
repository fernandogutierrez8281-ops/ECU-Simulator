#include "vehicleSimulator.hpp"

#include <iostream>

PIDcontroller::PIDcontroller(double kp, double ki, double kd)
    : kp(kp), ki(ki), kd(kd), previousError(0.0) {
    }

    void PIDcontroller::compute(double setpoint, double measuredValue, double dt) {
        
        error = setpoint - measuredValue;

        double integral = 0.0;
        integral += error * dt;
        double derivative = (error - previousError) / dt;
        previousError = error;

        output = kp * error + ki * integral + kd * derivative;
}

void PIDcontroller::showOutput() {
    std::cout << "PID Output: " << output << std::endl;
}