#pragma once

#include <memory>
#include <vector>

#include <TLorentzVector.h>

#include "GlobalFlag.h"

class ScaleMet;
class SkimTree;

namespace fwk {

class ScaleMetModule {
public:
    explicit ScaleMetModule(const GlobalFlag& gf);

    void applyCorrections(const std::shared_ptr<SkimTree>& skimT,
                          const std::vector<double>& jetPtRaw) const;

    TLorentzVector correctedMet() const;

private:
    std::shared_ptr<ScaleMet> scaleMet_;
};

} // namespace fwk
