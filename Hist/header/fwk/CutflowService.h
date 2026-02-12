#pragma once

#include <string>
#include <unordered_map>

namespace fwk {

class CutflowService {
public:
    void fill(const std::string& cut, double w = 1.0);
    const std::unordered_map<std::string, double>& counts() const;

private:
    std::unordered_map<std::string, double> counts_;
};

} // namespace fwk
