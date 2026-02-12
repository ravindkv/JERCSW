// cpp/PickWqqm.cpp
#include "PickWqqm.h"
#include "ReadConfig.h"
#include <cmath>

PickWqqm::PickWqqm(const GlobalFlag& globalFlags)
  : globalFlags_(globalFlags),
    pickJet_(globalFlags_)
{
    loadConfig("config/PickWqqm.json");
}

void PickWqqm::loadConfig(const std::string& filename) {
    ReadConfig cfg(filename);

    // Muon cuts
    muPtDefault_      = cfg.getValue<double>({"muPtDefault"});
    muPt2017_         = cfg.getValue<double>({"muPt2017"});
    muEtaMax_         = cfg.getValue<double>({"muEtaMax"});
    muRequireTightId_ = cfg.getValue<bool>   ({"muRequireTightId"});
    muIsoMax_         = cfg.getValue<double>({"muIsoMax"});
    muDxyMax_         = cfg.getValue<double>({"muDxyMax"});
    muDzMax_          = cfg.getValue<double>({"muDzMax"});

    // Jet cuts
    jetMinPt_         = cfg.getValue<double>({"jetMinPt"});
    jetMaxEta_        = cfg.getValue<double>({"jetMaxEta"});
    jetIdLabel_       = cfg.getValue<std::string>({"jetIdLabel"});

}

void PickWqqm::pickMuons(const SkimTree& skimT) {
    // --- 1) Muon selection ---
    pickedMuons_.clear();
    double ptThr = (globalFlags_.getYear() == GlobalFlag::Year::Year2017
                        ? muPt2017_
                        : muPtDefault_);
    for (UInt_t m = 0; m < skimT.nMuon; ++m) {
        double pt  = skimT.Muon_pt[m];
        double eta = skimT.Muon_eta[m];

        if (pt <= ptThr)                   continue;
        if (std::abs(eta) > muEtaMax_)     continue;
        if (muRequireTightId_ && !skimT.Muon_tightId[m])    continue;
        if (skimT.Muon_pfRelIso04_all[m] >= muIsoMax_)      continue;
        if (std::abs(skimT.Muon_dxy[m])  >= muDxyMax_)      continue;
        if (std::abs(skimT.Muon_dz[m])   >= muDzMax_)       continue;

        pickedMuons_.push_back(m);
    }
}

void PickWqqm::pickJets(const SkimTree& skimT, double minBJetDisc) {
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
