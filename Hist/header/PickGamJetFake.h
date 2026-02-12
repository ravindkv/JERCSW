#pragma once

#include <vector>
#include <string>
#include <cmath>

#include <TLorentzVector.h>

#include "SkimTree.h"
#include "GlobalFlag.h"

class PickGamJetFake{
public:
    // Constructor accepting a reference to GlobalFlag
    explicit PickGamJetFake(const GlobalFlag& globalFlags);
    ~PickGamJetFake();

    // Reco objects
    void pickJets(const SkimTree& skimT);
    void pickRefs(const SkimTree& skimT, const int& iJet);
    void pickPhotons(const SkimTree& skimT);

    // Accessors for picked objects
    const std::vector<int>& getPickedPhotons() const { return pickedPhotons_; }
    const std::vector<TLorentzVector>& getPickedJetsP4() const { return pickedJetsP4_; }
    const std::vector<int>& getPickedJetsIndex() const { return pickedJetsIndex_; }

    TLorentzVector getMatchedGenPhotonP4(const SkimTree& skimT, int phoInd) const;
    TLorentzVector getMatchedGenJetP4(const SkimTree& skimT, const int& iJet);
    std::vector<double> getRawJetPts(const SkimTree& skimT) const;
    std::vector<double> getRawPhoPts(const SkimTree& skimT) const;

private:
    // Reco objects
    std::vector<int> pickedPhotons_;
    std::vector<TLorentzVector> pickedJetsP4_;
    std::vector<int> pickedJetsIndex_;

    // Photon pick config
    double minPtPho_;
    double maxEtaPho_;
    int tightIdPho_;
    bool mvaIdWp80Pho_;
    double minR9Pho_ ;
    double maxR9Pho_ ;
    double maxHoePho_;
    bool isEleVetoPho_;
    bool hasPixelSeedPho_;

    // Jet pick config
    double minPtJet_;
    double maxEtaJet_;
    double minPtOther_;      

    // Gen Jet pick config
    double maxDeltaRgenJet_;
    double minPtGenJet_;
    double maxEtaGenJet_;

    double maxDeltaRgenPho_;

    // Reference to GlobalFlag instance
    const GlobalFlag& globalFlags_;
    const GlobalFlag::Year year_;
    const GlobalFlag::Channel channel_;
    const bool isDebug_;

    // Helper function for debug printing
    void printDebug(const std::string& message) const;

    // Load configuration from JSON file
    void loadConfig(const std::string& filename);
};

