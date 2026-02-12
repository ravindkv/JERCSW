#pragma once

#include <string>
#include <vector>

#include "GlobalFlag.h"
#include "SkimTree.h"
#include "ScaleBtagFunction.h"

class ScaleBtag {
public:
    explicit ScaleBtag(const GlobalFlag& globalFlags);

    double getEventWeight(const SkimTree& skimT,
                          const std::vector<int>& jetIdx,
                          const std::string& syst = "central",
                          double btagCut = -1.0) const;

    void setEffType(const std::string& effType) { functions_.setEffType(effType); }

    const std::string& effType() const { return functions_.effType(); }
    const std::string& algo()    const { return functions_.algo(); }
    const std::string& wp()      const { return functions_.wp(); }

private:
    ScaleBtagFunction functions_;

    const GlobalFlag& globalFlags_;
    const bool isDebug_;
    const bool isData_;
    const bool isMC_;
};

