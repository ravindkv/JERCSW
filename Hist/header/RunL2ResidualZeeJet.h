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

class RunL2ResidualZeeJet {
public:
    explicit RunL2ResidualZeeJet(const GlobalFlag& globalFlags);
    ~RunL2ResidualZeeJet() = default;

    auto Run(std::shared_ptr<SkimTree>& skimT,
             ScaleEvent* scaleEvent,
             TFile* fout) -> int;

private:
    // Reference to GlobalFlag instance
    const GlobalFlag& globalFlags_;

    // thresholds loaded from JSON
    std::vector<std::string> cutflows_;
    double dPhiWindow_;
    double maxAsymmetry_;

    // Method to load the configuration.
    void loadConfig(const std::string& filename);

};

