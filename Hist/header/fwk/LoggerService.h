#pragma once

#include <iostream>
#include <string>

namespace fwk {

class LoggerService {
public:
    void info(const std::string& msg) const;
    void warn(const std::string& msg) const;
    void error(const std::string& msg) const;
};

} // namespace fwk
