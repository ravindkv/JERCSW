// cpp/PickWqqe.cpp
#include "PickWqqe.h"
#include "ReadConfig.h"
#include <cmath>
#include <iostream>

PickWqqe::PickWqqe(const GlobalFlag& globalFlags)
  : globalFlags_(globalFlags),
    pickJet_(globalFlags_)
{
    loadConfig("config/PickWqqe.json");
}

void PickWqqe::loadConfig(const std::string& filename) {
    ReadConfig config(filename);

    // Electron pick configuration
    minPtEle_       = config.getValue<double>({"electronPick", "minPt"});
    maxEtaEle_      = config.getValue<double>({"electronPick", "maxEta"});
    tightIdEle_     = config.getValue<int>({"electronPick", "tightId"});
    minEbEeGap_     = config.getValue<double>({"electronPick", "ebEeGap", "min"});
    maxEbEeGap_     = config.getValue<double>({"electronPick", "ebEeGap", "max"});

    // Jet cuts
    jetMinPt_         = config.getValue<double>({"jetMinPt"});
    jetMaxEta_        = config.getValue<double>({"jetMaxEta"});
    jetIdLabel_       = config.getValue<std::string>({"jetIdLabel"});

}

// Helper function for debug printing
void PickWqqe::printDebug(const std::string& message){
    if (globalFlags_.isDebug()) {
        std::cout << message << '\n';
    }
}

void PickWqqe::pickElectrons(const SkimTree& skimT) {
    printDebug("Starting pickElectrons, nElectron = " + std::to_string(skimT.nElectron));
    pickedElectrons_.clear();

    for (int eleInd = 0; eleInd < skimT.nElectron; ++eleInd) {
        double eta = skimT.Electron_eta[eleInd];
        double absEta = std::abs(eta);
        double SCeta = eta + skimT.Electron_deltaEtaSC[eleInd];
        double absSCEta = std::abs(SCeta);
        double pt = skimT.Electron_pt[eleInd];

        // Check that the supercluster does not fall within the barrel-endcap gap.
        bool passEtaEBEEGap = (absSCEta < minEbEeGap_) || (absSCEta > maxEbEeGap_);
        // Tight electron ID check
        bool passTightID = (skimT.Electron_cutBased[eleInd] >= tightIdEle_);

        bool eleSel = (passEtaEBEEGap && absEta <= maxEtaEle_ && pt >= minPtEle_ && passTightID);
        if (eleSel) {
            pickedElectrons_.push_back(eleInd);
            printDebug("Electron " + std::to_string(eleInd) + " selected: pt = " +
                       std::to_string(pt) + ", eta = " + std::to_string(eta));
        } else {
            printDebug("Electron " + std::to_string(eleInd) + " rejected: pt = " +
                       std::to_string(pt) + ", eta = " + std::to_string(eta));
        }
    }

    printDebug("Total Electrons Picked: " + std::to_string(pickedElectrons_.size()));
}

void PickWqqe::pickJets(const SkimTree& skimT, double minBJetDisc) {
    // --- 2) Jet & b‑jet selection ---
    pickedJets_.clear();
    pickedBJets_.clear();
    for (int i = 0; i < skimT.nJet; ++i) {
        double pt  = skimT.Jet_pt[i];
        double eta = skimT.Jet_eta[i];
        
        if (pt < jetMinPt_)                 continue;
        if (std::abs(eta) > jetMaxEta_)     continue;
        if (!pickJet_.passId(skimT, i, jetIdLabel_)) continue;

        pickedJets_.push_back(i);
        if (skimT.Jet_btagDeepFlavB[i] > minBJetDisc)
            pickedBJets_.push_back(i);
    }
}
