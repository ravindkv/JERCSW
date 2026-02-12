// header/PickWqqm.h
#pragma once

#include <vector>
#include <string>
#include "GlobalFlag.h"
#include "PickJet.h"
#include "SkimTree.h"

class PickWqqm {
public:
    explicit PickWqqm(const GlobalFlag& globalFlags);
    ~PickWqqm() = default;

    // Perform muon + jet + b‑jet selection
    void pickMuons(const SkimTree& skimT);
    void pickJets(const SkimTree& skimT, double minBJetDisc);

    // Accessors
    const std::vector<int>& getPickedMuons() const { return pickedMuons_; }
    const std::vector<int>& getPickedJets()  const { return pickedJets_; }
    const std::vector<int>& getPickedBJets() const { return pickedBJets_; }

private:
    const GlobalFlag& globalFlags_;
    PickJet pickJet_;

    // --- muon cuts ---
    double muPtDefault_;
    double muPt2017_;
    double muEtaMax_;
    bool   muRequireTightId_;
    double muIsoMax_;
    double muDxyMax_;
    double muDzMax_;

    // --- jet cuts ---
    double jetMinPt_;
    double jetMaxEta_;
    std::string jetIdLabel_;

    // temporary containers
    std::vector<int> pickedMuons_;
    std::vector<int> pickedJets_;
    std::vector<int> pickedBJets_;

    // load *all* thresholds from JSON
    void loadConfig(const std::string& filename);
};
