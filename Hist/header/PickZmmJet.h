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

class PickZmmJet {
public:
    explicit PickZmmJet(const GlobalFlag& globalFlags);
    ~PickZmmJet();

    // Reco objects
    void pickMuons(const SkimTree& skimT);
    void pickTags(const SkimTree& skimT);
    void pickJets(const SkimTree& skimT, const TLorentzVector& p4Tag);
    void pickGenJets(const SkimTree& skimT, const int& iJet1, const int& iJet2,
                     const TLorentzVector& p4Jet1, const TLorentzVector& p4Jet2);

    // Gen objects
    void pickGenMuons(const SkimTree& skimT);
    void pickGenTags(const SkimTree& skimT, const TLorentzVector& p4Tag);

    // Accessors
    const std::vector<int>&              getPickedMuons()     const { return pickedMuons_; }
    const std::vector<TLorentzVector>&   getPickedTags()      const { return pickedTags_; }
    const std::vector<TLorentzVector>&   getPickedJetsP4()    const { return pickedJetsP4_; }
    const std::vector<int>&              getPickedJetsIndex() const { return pickedJetsIndex_; }

    const std::vector<int>&              getPickedGenMuons()   const { return pickedGenMuons_; }
    const std::vector<TLorentzVector>&   getPickedGenTags()    const { return pickedGenTags_; }

private:
    // Reco
    std::vector<int>            pickedMuons_;
    std::vector<TLorentzVector> pickedTags_;
    std::vector<TLorentzVector> pickedJetsP4_;
    std::vector<int>            pickedJetsIndex_;

    // Gen
    std::vector<int>            pickedGenMuons_;
    std::vector<TLorentzVector> pickedGenTags_;

    // Muon pick config
    double minPtLeadMu_;
    double minPtSubLeadMu_;
    double maxEtaMu_;
    int    tightIdMu_;
    double maxRelIsoMu_;
    double maxDxyMu_;
    double maxDzMu_;

    // Gen muon pick
    int pdgIdGenMu_;

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

