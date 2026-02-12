#include "fwk/ScaleMuonModule.h"

#include "ScaleMuon.h"

namespace fwk {

ScaleMuonModule::ScaleMuonModule(const GlobalFlag& gf)
    : scaleMuon_(std::make_shared<ScaleMuon>(gf)) {}

void ScaleMuonModule::applyCorrections(std::shared_ptr<SkimTree>& skimT) const {
    scaleMuon_->applyCorrections(skimT);
}

ScaleMuon::MuSf ScaleMuonModule::getMuonSfs(const SkimTree& skimT,
                                            int index,
                                            ScaleMuon::SystLevel syst) const {
    return scaleMuon_->getMuonSfs(skimT, index, syst);
}

} // namespace fwk
