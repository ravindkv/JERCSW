#pragma once

#include <iostream>
#include <cmath>

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

class RunL3ResidualGamJetFake{
public:
    // Constructor accepting a reference to GlobalFlag
    explicit RunL3ResidualGamJetFake(const GlobalFlag& globalFlags);
    ~RunL3ResidualGamJetFake() = default;

    int Run(std::shared_ptr<SkimTree>& skimT, ScaleEvent* scaleEvent, TFile* fout);

private:
    // Reference to GlobalFlag instance
    const GlobalFlag& globalFlags_;

    // Configuration parameters loaded from JSON
    std::vector<std::string> cutflows_;
    std::vector<int> minTagPts_;
    double maxDeltaPhiTagProbe_;
    double maxAlpha_;
    double minPtJet2InAlpha_;
    double maxTagEta_;
    double minTagPt_;
    std::vector<double> alphaCuts_;
    double minResp_;
    double maxResp_;
    double maxDeltaRgenJetPhoton_;

    // Method to load the configuration.
    void loadConfig(const std::string& filename);
};

