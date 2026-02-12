#pragma once

#include <string>
#include <vector>

#include <TLorentzVector.h>

#include "GlobalFlag.h"
#include "PickJet.h"
#include "SkimTree.h"

class PickGamJet {
public:
    explicit PickGamJet(const GlobalFlag& globalFlags);
    ~PickGamJet();

    // Reco objects
    void pickPhotons(const SkimTree& skimT);
    void pickTag(const SkimTree& skimT);
    void pickJets(const SkimTree& skimT, const TLorentzVector& p4Tag);

    // Gen objects
    void pickGenPhotons(const SkimTree& skimT);
    void pickGenTags(const SkimTree& skimT, const TLorentzVector& p4Tag);

    // Accessors for picked objects
    const std::vector<int>& getPickedPhotons() const { return pickedPhotons_; }
    const TLorentzVector getPickedTag() const { return pickedTagP4; }
    const std::vector<TLorentzVector>& getPickedJetsP4() const { return pickedJetsP4_; }
    const std::vector<int>& getPickedJetsIndex() const { return pickedJetsIndex_; }

    const std::vector<int>& getPickedGenPhotons() const { return pickedGenPhotons_; }
    const std::vector<TLorentzVector>& getPickedGenTags() const { return pickedGenTags_; }

    // Convenience
    [[nodiscard]] bool hasRecoPhoton() const noexcept { return !pickedPhotons_.empty(); }
    [[nodiscard]] bool hasRecoJets()   const noexcept { return !pickedJetsP4_.empty(); }

private:
    // Reco objects
    std::vector<int> pickedPhotons_;
    TLorentzVector pickedTagP4;
    int pickedTagIndex_{-1};
    int phoJetIdx_{-1};
    std::vector<TLorentzVector> pickedJetsP4_;   // [jet1, jet2, sumOther]
    std::vector<int> pickedJetsIndex_;           // only valid indices (no -1)

    // Gen objects
    std::vector<int> pickedGenPhotons_;
    std::vector<TLorentzVector> pickedGenTags_;

    // Photon pick config
    double minPtPho_{0.0};
    double maxEtaPho_{999.0};
    int    tightIdPho_{0};
    double minR9Pho_{-1.0};
    double maxR9Pho_{ 9.0};
    double maxHoePho_{999.0};
    bool   isEleVetoPho_{true};
    bool   hasPixelSeedPho_{false};

    // Jet pick config
    double      minPtJet_{0.0};
    double      maxEtaProbeJet_{999.0};
    std::string jetIdLabel_;
    double      minDeltaRrefJet_{0.0};

    // Optional: apply JetID/eta to jet2 as well (robust default = true)
    bool applyJetIdToJet2_{true};
    bool applyEtaToJet2_{true};

    // Gen Photon pick config
    int pdgIdGenPho_{22}; // currently unused in your code, but kept
    double minPtGenPho_{0.0};

    // Gen Tag pick config
    double maxDeltaRgenTag_{0.2};
    double      minPtGenTag_{0.0};

    // Global state
    const GlobalFlag& globalFlags_;
    const GlobalFlag::Year year_;
    const GlobalFlag::Channel channel_;
    const bool isDebug_;
    PickJet pickJet_;

    void printDebug(const std::string& message) const;

    // Config
    void loadConfig(const std::string& filename);
    void validateConfig_() const;

    // Reset helpers
    void resetReco_();
    void resetGen_();
};

