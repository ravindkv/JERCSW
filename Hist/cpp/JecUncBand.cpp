#include "JecUncBand.h"
#include <iostream>
#include <cmath>

JecUncBand::JecUncBand(const GlobalFlag& globalFlags)
    : functions_(globalFlags),
      globalFlags_(globalFlags),
      isDebug_(globalFlags_.isDebug()),
      isData_(globalFlags_.isData())
{
}

double JecUncBand::getJesRelUncForBand( double eta, double ptAfterJes) const{
    return functions_.getJesRelUncForBand(eta, ptAfterJes);
}

double JecUncBand::getJerSfRelUncForBand(double eta, const std::string& syst) const {
    return functions_.getJerSfRelUncForBand(eta, syst);

}
