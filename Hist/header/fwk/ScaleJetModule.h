#pragma once

#include <memory>
#include <vector>

#include "GlobalFlag.h"

class ScaleJet;
class SkimTree;

namespace fwk {

class ScaleJetModule {
public:
    explicit ScaleJetModule(const GlobalFlag& gf);

    void applyCorrections(std::shared_ptr<SkimTree>& skimT) const;

    const std::vector<double>& jetPtRaw() const;

private:
    std::shared_ptr<ScaleJet> scaleJet_;
};

} // namespace fwk
