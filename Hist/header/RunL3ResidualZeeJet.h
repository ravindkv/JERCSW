
#pragma once

#include <iostream>
#include <cmath>
#include <vector>
#include <string>

// ROOT includes
#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TProfile.h>
#include <TProfile2D.h>
#include <TLorentzVector.h>

// User-defined includes
#include "SkimTree.h"
#include "ScaleEvent.h"
#include "GlobalFlag.h"

class RunL3ResidualZeeJet{
public:
    // Constructor accepting a reference to GlobalFlag
    explicit RunL3ResidualZeeJet(const GlobalFlag& globalFlags);
    ~RunL3ResidualZeeJet() = default;

    int Run(std::shared_ptr<SkimTree>& skimT, ScaleEvent* scaleEvent, TFile* fout);

private:
    // Reference to GlobalFlag instance
    const GlobalFlag& globalFlags_;

    // Configuration parameters loaded from JSON
    std::vector<std::string> cutflows_;
    std::vector<int> minTagPts_;
    double maxDeltaPhiTagProbe_;
    double maxAlpha_;
    std::vector<double> alphaCuts_;
    double minPtJet2InAlpha_;
    double minResp_;
    double maxResp_;

    // Method to load the configuration.
    void loadConfig(const std::string& filename);
};

