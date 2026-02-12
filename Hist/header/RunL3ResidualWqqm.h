
#pragma once

#include <vector>
#include <string>
#include <memory>
#include <cassert>

// ROOT includes
#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TProfile.h>
#include <TProfile2D.h>
#include <TLorentzVector.h>

// User‑defined includes
#include "SkimTree.h"
#include "ScaleEvent.h"
#include "GlobalFlag.h"

class RunL3ResidualWqqm {
public:
    explicit RunL3ResidualWqqm(const GlobalFlag& globalFlags);
    ~RunL3ResidualWqqm() = default;

    auto Run(std::shared_ptr<SkimTree>& skimT,
             ScaleEvent*             scaleEvent,
             TFile*                  fout) -> int;

private:
    // reference to the global flags singleton
    const GlobalFlag& globalFlags_;

    // --- config parameters loaded from JSON ---
    std::vector<std::string> cutflows_;
    double massW_;
    double massT_;
    double massMu_;
    int maxChi2_;
    int minMet_;

    double resLep_;
    double resMet_;
    double resHadW_;
    double resHadT_;
    double resLepT_;
    bool   useReso_;

    int nJetMin_;
    int nBJetMin_;
    double minBJetDisc_;

    double maxEtaW_;
    double minPtW_;

    // loads all of the above from a JSON file
    void loadConfig(const std::string& filename);
};

