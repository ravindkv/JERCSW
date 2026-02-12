#pragma once

#include <string>
#include "GlobalFlag.h"
#include "JecUncBandLoader.h"

class JecUncBandFunction {
public:
    JecUncBandFunction(const GlobalFlag& globalFlags);

    double getJesRelUncForBand(double eta, double ptAfterJes) const;
    double getJerScaleFactor(double eta, const std::string& syst) const;
    double getJerSfRelUncForBand(double eta, const std::string& syst) const;

private:
    JecUncBandLoader loader_;
    const GlobalFlag& globalFlags_;
    const bool isDebug_;
};

