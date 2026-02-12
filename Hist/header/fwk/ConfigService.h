#pragma once

#include <string>

#include <nlohmann/json.hpp>

namespace fwk {

class ConfigService {
public:
    nlohmann::json loadJson(const std::string& path) const;
};

} // namespace fwk
