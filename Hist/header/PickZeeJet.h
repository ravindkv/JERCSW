#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <memory>
#include <cmath>

#include <TH1F.h>
#include <TH1D.h>
#include <TLorentzVector.h>

#include "SkimTree.h"
#include "GlobalFlag.h"
#include "PickZJet.h"

class PickZeeJet {
public:
    explicit PickZeeJet(const GlobalFlag& globalFlags);
    ~PickZeeJet();

    // Reco objects
    void pickElectrons(const SkimTree& skimT);
    void pickTags(const SkimTree& skimT);
    void pickJets(const SkimTree& skimT, const TLorentzVector& p4Tag);
    void pickGenJets(const SkimTree& skimT, const int& iJet1, const int& iJet2,
                     const TLorentzVector& p4Jet1, const TLorentzVector& p4Jet2);

    // Gen objects
    void pickGenElectrons(const SkimTree& skimT);
    void pickGenTags(const SkimTree& skimT, const TLorentzVector& p4Tag);

    // Accessors
    const std::vector<int>&              getPickedElectrons()  const { return pickedElectrons_; }
    const std::vector<TLorentzVector>&   getPickedTags()       const { return pickedTags_; }
    const std::vector<TLorentzVector>&   getPickedJetsP4()     const { return pickedJetsP4_; }
    const std::vector<int>&              getPickedJetsIndex()  const { return pickedJetsIndex_; }

    const std::vector<int>&              getPickedGenElectrons() const { return pickedGenElectrons_; }
    const std::vector<TLorentzVector>&   getPickedGenTags()      const { return pickedGenTags_; }
private:
    // Reco
    std::vector<int>            pickedElectrons_;
    std::vector<TLorentzVector> pickedTags_;
    std::vector<TLorentzVector> pickedJetsP4_;
    std::vector<int>            pickedJetsIndex_;

    // Gen
    std::vector<int>            pickedGenElectrons_;
    std::vector<TLorentzVector> pickedGenTags_;

    // Electron pick config
    double minPtLeadEle_;
    double minPtSubLeadEle_;
    double maxEtaEle_;
    int    tightIdEle_;
    double minEbEeGap_;
    double maxEbEeGap_;

    // Gen electron pick
    int pdgIdGenEle_;

    // Global flags
    const GlobalFlag&             globalFlags_;
    const GlobalFlag::Year  year_;
    const GlobalFlag::Channel channel_;
    const bool              isDebug_;

    // Common Z+jet helper
    PickZJet zJet_;

    void printDebug(const std::string& message) const;
    void loadConfig(const std::string& filename);
};

