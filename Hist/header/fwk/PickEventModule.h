#pragma once

#include <memory>

#include <TLorentzVector.h>

#include "GlobalFlag.h"

class HistCutflow;
class PickEvent;
class SkimTree;

namespace fwk {

class PickEventModule {
public:
    explicit PickEventModule(const GlobalFlag& gf);

    bool passCoreEventCuts(const std::shared_ptr<SkimTree>& skimT,
                           HistCutflow* cutflow,
                           double weight) const;

    bool passProbeJetVeto(const TLorentzVector& p4Probe,
                          HistCutflow* cutflow,
                          double weight) const;

private:
    std::shared_ptr<PickEvent> pickEvent_;
};

} // namespace fwk
