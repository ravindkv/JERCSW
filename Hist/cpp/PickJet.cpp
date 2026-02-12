// cpp/PickJet.cpp
#include "PickJet.h"

#include <algorithm>
#include <cmath>
#include <iostream>

PickJet::PickJet(const GlobalFlag& globalFlags)
    : globalFlags_(globalFlags)
    , nanoVersion_(globalFlags_.getNanoVersion())
    , jetAlgo_(globalFlags_.getJetAlgo())
{
}

void PickJet::printDebug(const std::string& message) const {
    if (globalFlags_.isDebug()) {
        std::cout << "[PickJet] " << message << '\n';
    }
}

// -------------------------------------------------------------
// Version helpers
// -------------------------------------------------------------
bool PickJet::useLegacyJetId_() const {
    // nanoAOD < v15: Jet_jetId branch is present and should be used
    // (extend if you later use V12)
    return (nanoVersion_ == GlobalFlag::NanoVersion::V9);
}

// -------------------------------------------------------------
// Public interface: generic WP-based API
// -------------------------------------------------------------
bool PickJet::passId(const SkimTree& skimT,
                     int iJ,
                     WorkingPoint wp) const
{

    switch (wp) {
        case WorkingPoint::Tight:
            return passTight(skimT, iJ);
        case WorkingPoint::TightLepVeto:
            return passTightLepVeto(skimT, iJ);
        default:
            return false;
    }
}

bool PickJet::passId(const SkimTree& skimT,
                     int iJ,
                     const std::string& label) const
{
    // Allow label from config: "Tight", "tight", "TightLepVeto", "tightlepveto", etc.
    std::string l = label;
    std::transform(l.begin(), l.end(), l.begin(), ::tolower);

    if (l == "tight") {
        return passId(skimT, iJ, WorkingPoint::Tight);
    } else if (l == "tightlepveto" || l == "tightlep" || l == "tight_lepveto") {
        return passId(skimT, iJ, WorkingPoint::TightLepVeto);
    }

    if (globalFlags_.isDebug()) {
        printDebug("Unknown JetId label: '" + label + "', returning false");
    }
    return false; 
}

// -------------------------------------------------------------
// Public interface: explicit helpers
// -------------------------------------------------------------
bool PickJet::passTight(const SkimTree& skimT, int iJ) const {
    bool pass = useLegacyJetId_()
              ? passTightLegacy_(skimT, iJ)
              : passTightManual_(skimT, iJ);

    if (globalFlags_.isDebug()) {
        const float eta = skimT.Jet_eta[iJ];
        printDebug("jet " + std::to_string(iJ) +
                   " (eta=" + std::to_string(eta) +
                   "): passTight=" + (pass ? "true" : "false"));
    }

    return pass;
}

bool PickJet::passTightLepVeto(const SkimTree& skimT, int iJ) const {
    bool pass = useLegacyJetId_()
              ? passTightLepVetoLegacy_(skimT, iJ)
              : passTightLepVetoManual_(skimT, iJ);

    if (globalFlags_.isDebug()) {
        const float eta = skimT.Jet_eta[iJ];
        printDebug("jet " + std::to_string(iJ) +
                   " (eta=" + std::to_string(eta) +
                   "): passTightLepVeto=" + (pass ? "true" : "false"));
    }

    return pass;
}

// -------------------------------------------------------------
// Legacy Jet_jetId (nanoAOD < v15)
// Jet_jetId: 0=fail, 2=loose, 4=tight, 6=tightLepVeto
// -------------------------------------------------------------
bool PickJet::passTightLegacy_(const SkimTree& skimT, int iJ) const {
    const int jetId = skimT.Jet_jetId[iJ];
    return (jetId >= 4); // Tight and TightLepVeto
}

bool PickJet::passTightLepVetoLegacy_(const SkimTree& skimT, int iJ) const {
    const int jetId = skimT.Jet_jetId[iJ];
    return (jetId >= 6);
}

bool PickJet::passTightManual_(const SkimTree& skimT, int iJ) const {
    const float eta            = skimT.Jet_eta[iJ];
    const float absEta         = std::fabs(eta);
    const float neHEF          = skimT.Jet_neHEF[iJ];
    const float neEmEF         = skimT.Jet_neEmEF[iJ];
    const float chHEF          = skimT.Jet_chHEF[iJ];
    const Short_t chMultiplicity = skimT.Jet_chMultiplicity[iJ];
    const Short_t neMultiplicity = skimT.Jet_neMultiplicity[iJ];

    bool pass = false;

    if (absEta <= 2.6f) {
        pass = (neHEF < 0.99f) &&
               (neEmEF < 0.9f) &&
               (chMultiplicity + neMultiplicity > 1) &&
               (chHEF > 0.01f) &&
               (chMultiplicity > 0);
    } else if (absEta > 2.6f && absEta <= 2.7f) {
        pass = (neHEF < 0.90f) &&
               (neEmEF < 0.99f);
    } else if (absEta > 2.7f && absEta <= 3.0f) {
        pass = (neHEF < 0.99f);
    } else if (absEta > 3.0f) {
        pass = (neMultiplicity >= 2) &&
               (neEmEF < 0.4f);
    }

    if (globalFlags_.isDebug()) {
        printDebug(
            "[passTightManual] jet " + std::to_string(iJ) +
            " eta=" + std::to_string(eta) +
            " absEta=" + std::to_string(absEta) +
            " neHEF=" + std::to_string(neHEF) +
            " neEmEF=" + std::to_string(neEmEF) +
            " chHEF=" + std::to_string(chHEF) +
            " chMult=" + std::to_string(chMultiplicity) +
            " neMult=" + std::to_string(neMultiplicity) +
            " → pass=" + std::string(pass ? "true" : "false")
        );
    }

    return pass;
}



bool PickJet::passTightLepVetoManual_(const SkimTree& skimT, int iJ) const {
    const float eta    = skimT.Jet_eta[iJ];
    const float absEta = std::fabs(eta);

    const bool passTightId = passTightManual_(skimT, iJ);

    if (!passTightId) {
        if (globalFlags_.isDebug()) {
            printDebug(
                "[passTightLepVetoManual] jet " + std::to_string(iJ) +
                " eta=" + std::to_string(eta) +
                " absEta=" + std::to_string(absEta) +
                " passTightId=false → pass=false"
            );
        }
        return false;
    }

    bool pass = true;

    if (absEta <= 2.7f) {
        const float muEF   = skimT.Jet_muEF[iJ];
        const float chEmEF = skimT.Jet_chEmEF[iJ];

        pass = (muEF < 0.8f) &&
               (chEmEF < 0.8f);

        if (globalFlags_.isDebug()) {
            printDebug(
                "[passTightLepVetoManual] jet " + std::to_string(iJ) +
                " eta=" + std::to_string(eta) +
                " absEta=" + std::to_string(absEta) +
                " muEF=" + std::to_string(muEF) +
                " chEmEF=" + std::to_string(chEmEF) +
                " passTightId=true → pass=" + (pass ? "true" : "false")
            );
        }
    } else {
        // |eta| > 2.7 → same as tight
        pass = true;

        if (globalFlags_.isDebug()) {
            printDebug(
                "[passTightLepVetoManual] jet " + std::to_string(iJ) +
                " eta=" + std::to_string(eta) +
                " absEta=" + std::to_string(absEta) +
                " region: |eta|>2.7 (inherits Tight) → pass=true"
            );
        }
    }

    return pass;
}


