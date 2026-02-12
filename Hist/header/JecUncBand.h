#pragma once

#include <unordered_map>
#include <vector>
#include <memory>

#include "JecUncBandFunction.h"
#include "GlobalFlag.h"

class JecUncBand {
public:
    explicit JecUncBand(const GlobalFlag& globalFlags);

    double getJesRelUncForBand(double eta, double ptAfterJes) const;//public APIs
    double getJerSfRelUncForBand(double eta, const std::string& syst) const;

private:
    JecUncBandFunction functions_;
    const GlobalFlag& globalFlags_;

    const bool isDebug_;
    const bool isData_;
};

