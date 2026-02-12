#include "fwk/ConfigService.h"

#include <fstream>
#include <stdexcept>

namespace fwk {

nlohmann::json ConfigService::loadJson(const std::string& path) const {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open config file: " + path);
    }
    return nlohmann::json::parse(file);
}

} // namespace fwk
