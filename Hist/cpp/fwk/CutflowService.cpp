#include "fwk/CutflowService.h"

namespace fwk {

void CutflowService::fill(const std::string& cut, double w) {
    counts_[cut] += w;
}

const std::unordered_map<std::string, double>& CutflowService::counts() const {
    return counts_;
}

} // namespace fwk
