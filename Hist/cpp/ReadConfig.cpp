#include "ReadConfig.h"
#include <fstream>
#include <stdexcept>

ReadConfig::ReadConfig(const std::string& filename) : filename_(filename) {
    std::ifstream configFile(filename);
    if (!configFile.is_open()){
        throw std::runtime_error("Could not open configuration file: " + filename);
    }
    configFile >> config_;
}

