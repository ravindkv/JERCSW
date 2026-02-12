#pragma once

namespace fwk {

struct Event {
    long long entry = -1;

    unsigned int run = 0;
    unsigned int lumi = 0;
    unsigned long long event = 0;

    double weight = 1.0;
};

} // namespace fwk
