#include "fwk/PickEventModule.h"

#include "HistCutflow.h"
#include "PickEvent.h"

namespace fwk {

PickEventModule::PickEventModule(const GlobalFlag& gf)
    : pickEvent_(std::make_shared<PickEvent>(gf)) {}

bool PickEventModule::passCoreEventCuts(const std::shared_ptr<SkimTree>& skimT,
                                        HistCutflow* cutflow,
                                        double weight) const {
    if (!pickEvent_->passHlt(skimT)) {
        return false;
    }
    if (cutflow) {
        cutflow->fill("passHlt", weight);
    }

    if (!pickEvent_->passGoodLumi(skimT->run, skimT->luminosityBlock)) {
        return false;
    }
    if (cutflow) {
        cutflow->fill("passGoodLumi", weight);
    }

    if (!pickEvent_->passMatchedGenVtx(*skimT)) {
        return false;
    }
    if (cutflow) {
        cutflow->fill("passMatchedGenVtx", weight);
    }

    return true;
}

bool PickEventModule::passProbeJetVeto(const TLorentzVector& p4Probe,
                                       HistCutflow* cutflow,
                                       double weight) const {
    if (!pickEvent_->passJetVetoMapOnProbe(p4Probe)) {
        return false;
    }
    if (cutflow) {
        cutflow->fill("passJetVetoMap", weight);
    }

    return true;
}

} // namespace fwk
