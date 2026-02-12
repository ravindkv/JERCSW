#include "fwk/ScaleMetModule.h"

#include "ScaleMet.h"

namespace fwk {

ScaleMetModule::ScaleMetModule(const GlobalFlag& gf)
    : scaleMet_(std::make_shared<ScaleMet>(gf)) {}

void ScaleMetModule::applyCorrections(const std::shared_ptr<SkimTree>& skimT,
                                      const std::vector<double>& jetPtRaw) const {
    scaleMet_->applyCorrection(skimT, jetPtRaw);
}

TLorentzVector ScaleMetModule::correctedMet() const {
    return scaleMet_->getP4CorrectedMet();
}

} // namespace fwk
