#pragma once 

#include <string>
#include <fstream>

#include "ecu.hpp"

class Logger {
    private:
        std::ofstream file;

    public:
        Logger(const std::string& fileName);

        ~Logger();

        void updateFile(const ECU& ecu, int cicle);

};