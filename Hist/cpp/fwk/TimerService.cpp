#include "fwk/TimerService.h"

namespace fwk {

void TimerService::start(const std::string& key) {
    start_[key] = std::chrono::high_resolution_clock::now();
}

void TimerService::stop(const std::string& key) {
    const auto t1 = std::chrono::high_resolution_clock::now();
    const auto it = start_.find(key);
    if (it == start_.end()) {
        return;
    }
    total_[key] += std::chrono::duration<double>(t1 - it->second).count();
}

const std::unordered_map<std::string, double>& TimerService::totals() const {
    return total_;
}

} // namespace fwk
