// header/PickWqqe.h
#pragma once

#include <vector>
#include <string>
#include "GlobalFlag.h"
#include "PickJet.h"
#include "ReadConfig.h"
#include "SkimTree.h"

class PickWqqe {
public:
    explicit PickWqqe(const GlobalFlag& globalFlags);
    ~PickWqqe() = default;

    // Perform electron + jet + b‑jet selection
    void pickElectrons(const SkimTree& skimT);
    void pickJets(const SkimTree& skimT, double minBJetDisc);

    // Accessors
    const std::vector<int>& getPickedElectrons() const { return pickedElectrons_; }
    const std::vector<int>& getPickedJets()  const { return pickedJets_; }
    const std::vector<int>& getPickedBJets() const { return pickedBJets_; }

private:
    const GlobalFlag& globalFlags_;
    PickJet pickJet_;

    // Electron pick config
    double minPtEle_;
    double maxEtaEle_;
    int tightIdEle_;
    double minEbEeGap_;
    double maxEbEeGap_;

    // --- jet cuts ---
    double jetMinPt_;
    double jetMaxEta_;
    std::string jetIdLabel_;

    // temporary containers
    std::vector<int> pickedElectrons_;
    std::vector<int> pickedJets_;
    std::vector<int> pickedBJets_;

    void printDebug(const std::string& message);

    // load *all* thresholds from JSON
    void loadConfig(const std::string& filename);
};
