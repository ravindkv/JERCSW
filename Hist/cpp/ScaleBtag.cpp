#include "ScaleBtag.h"
#include <vector>

ScaleBtag::ScaleBtag(const GlobalFlag& globalFlags)
    : functions_(globalFlags),
      globalFlags_(globalFlags),
      isDebug_(globalFlags_.isDebug()),
      isData_(globalFlags_.isData()),
      isMC_(globalFlags_.isMC()) {}

double ScaleBtag::getEventWeight(const SkimTree& skimT,
                                const std::vector<int>& jetIdx,
                                const std::string& syst,
                                double btagCut) const {
    return functions_.getEventWeight(skimT, jetIdx, syst, btagCut);
}

