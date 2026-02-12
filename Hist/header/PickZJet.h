#pragma once

#include <vector>
#include <string>
#include <iostream>

#include <TLorentzVector.h>

#include "SkimTree.h"
#include "GlobalFlag.h"
#include "PickJet.h"

// Common helper for Z(ℓℓ)+jet logic shared by ZeeJet and ZmmJet
class PickZJet {
public:
    explicit PickZJet(const GlobalFlag& globalFlags);

    // --- Reco Z boson from selected leptons (indices into NanoAOD arrays) ---
    // Returns 0 or 1 TLorentzVector (best OS pair closest to mZ within window).
    std::vector<TLorentzVector> pickRecoZ(
        const std::vector<int>& pickedLeptons,
        const float* lep_pt,
        const float* lep_eta,
        const float* lep_phi,
        const float* lep_mass,
        const int*   lep_charge) const;

    // --- Reco jets: leading, subleading, and sum of others ---
    // Returns vector of size 3: [jet1, jet2, jetN]. Fills outJetIndices with [iJet1, iJet2].
    // Note: outJetIndices may contain -1 if no jet is found.
    std::vector<TLorentzVector> pickRecoJets(
        const SkimTree& skimT,
        const std::vector<int>& pickedLeptons,
        const float* lep_eta,
        const float* lep_phi,
        const int* jet_lepIdx1,
        const int* jet_lepIdx2,
        std::vector<int>& outJetIndices) const;

    // --- Gen Z boson from dressed gen leptons ---
    // Returns 0 or 1 TLV that matches reco Z within ΔR<maxDeltaRgenTag_ (best match).
    std::vector<TLorentzVector> pickGenZ(
        const SkimTree& skimT,
        const TLorentzVector& p4RecoZ,
        const std::vector<int>& pickedGenLeptons) const;

    // Optional getters
    double massTag()       const { return massTag_; }
    double massWindowTag() const { return massWindowTag_; }
    double minPtTag()      const { return minPtTag_; }

private:
    // Tagerence pick config
    double massTag_{91.1876};
    double massWindowTag_{20.0};
    double minPtTag_{0.0};
    double maxEtaTag_{0.0};
    int nLepMin_{2};
    int nLepMax_{3};

    // Jet pick config
    double minPtJet_{0.0};
    double maxEtaProbeJet_{999.0};
    double minDeltaRtoLepton_{0.4};
    std::string jetIdLabel_;
    int requiredJet_lepIdx1_{-1};
    int requiredJet_lepIdx2_{-1};

    // Gen picks
    double minPtGenTag_{0.0};
    double maxDeltaRgenTag_{0.2};

    const GlobalFlag& globalFlags_;
    PickJet pickJet_;
    const bool isDebug_;

    void loadConfig(const std::string& filename);
    void validateConfig_() const;

    void printDebug(const std::string& msg) const;
};

