#include "fwk/ScaleJetModule.h"

#include "ScaleJet.h"

namespace fwk {

ScaleJetModule::ScaleJetModule(const GlobalFlag& gf)
    : scaleJet_(std::make_shared<ScaleJet>(gf)) {}

void ScaleJetModule::applyCorrections(std::shared_ptr<SkimTree>& skimT) const {
    scaleJet_->applyCorrection(skimT);
}

const std::vector<double>& ScaleJetModule::jetPtRaw() const {
    return scaleJet_->getJetPtRaw();
}

} // namespace fwk
