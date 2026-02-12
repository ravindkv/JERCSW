#pragma once

#include <memory>
#include <vector>
#include <string>
#include <map>
#include <TFile.h>
#include <TLorentzVector.h>

#include "GlobalFlag.h"
#include "SkimTree.h"
#include "ScaleEvent.h"
#include "PickMultiJet.h"

class RunL3ResidualMultiJet {
public:
    explicit RunL3ResidualMultiJet(const GlobalFlag& globalFlags);
    ~RunL3ResidualMultiJet() = default;

    auto Run(std::shared_ptr<SkimTree>& skimT,
             ScaleEvent* scaleEvent,
             TFile* fout) -> int;

private:
    // Reference to GlobalFlag instance
    const GlobalFlag& globalFlags_;

    // thresholds loaded from JSON
    std::vector<std::string> cutflows_;
    std::vector<int> minTagPts_;
    int    minRecoilJets_;
    double dPhiWindow_;
    double maxRecoilFraction_;
    bool   fillPerHlt_;

    // Method to load the configuration.
    void loadConfig(const std::string& filename);

};

