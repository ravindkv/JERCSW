#pragma once

#include <chrono>
#include <string>
#include <unordered_map>

namespace fwk {

class TimerService {
public:
    void start(const std::string& key);
    void stop(const std::string& key);

    const std::unordered_map<std::string, double>& totals() const;

private:
    std::unordered_map<std::string, std::chrono::high_resolution_clock::time_point> start_;
    std::unordered_map<std::string, double> total_;
};

} // namespace fwk
