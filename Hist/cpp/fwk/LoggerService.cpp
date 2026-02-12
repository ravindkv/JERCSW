#include "fwk/LoggerService.h"

namespace fwk {

void LoggerService::info(const std::string& msg) const {
    std::cout << "[fwk][INFO] " << msg << "\n";
}

void LoggerService::warn(const std::string& msg) const {
    std::cout << "[fwk][WARN] " << msg << "\n";
}

void LoggerService::error(const std::string& msg) const {
    std::cerr << "[fwk][ERROR] " << msg << "\n";
}

} // namespace fwk
