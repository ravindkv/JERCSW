#pragma once

#include <memory>

#include "GlobalFlag.h"
#include "ScaleMuon.h"

class SkimTree;
class ScaleMuon;

namespace fwk {

class ScaleMuonModule {
public:
    explicit ScaleMuonModule(const GlobalFlag& gf);

    void applyCorrections(std::shared_ptr<SkimTree>& skimT) const;

    ScaleMuon::MuSf getMuonSfs(const SkimTree& skimT,
                               int index,
                               ScaleMuon::SystLevel syst = ScaleMuon::SystLevel::Nominal) const;

private:
    std::shared_ptr<ScaleMuon> scaleMuon_;
};

} // namespace fwk
