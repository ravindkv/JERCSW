// header/HemVeto.h
#pragma once

#include <string>
#include "SkimTree.h"
#include "GlobalFlag.h"
#include "ReadConfig.h"

class HemVeto {
public:
    /// ctor will load everything from config/HemVeto.json for the current year
    HemVeto(const GlobalFlag& globalFlags);

    /// returns true if the event should be vetoed due to HEM
    bool isHemVeto(const SkimTree& skimT) const;
    double getMcWeight(){return mcWeight_;}

private:
    void loadConfig_(const std::string& filename);

    const GlobalFlag& globalFlags_;
    const bool isMC_;

    // master switch and run threshold (only veto data after a given run)
    bool applyHemVeto_;
    int  runThreshold_;
    double mcWeight_;

    // per-object HEM cuts
    struct Cuts { double ptMin, etaMin, etaMax, phiMin, phiMax; };
    Cuts eleCuts_, phoCuts_, jetCuts_;
};

