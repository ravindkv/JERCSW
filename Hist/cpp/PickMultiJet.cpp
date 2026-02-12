#include "PickMultiJet.h"
#include "HelperDelta.hpp"
#include <TMath.h>
#include "ReadConfig.h"

// Constructor: you can load these from JSON if you like,
// or just hard‐code the defaults shown here:
PickMultiJet::PickMultiJet(const GlobalFlag& flags)
  : globalFlags_(flags)
  , pickJet_(globalFlags_)
  , isDebug_(flags.isDebug())
  , iProbe_(-1)
  , p4Probe_(0,0,0,0)
  , p4SumRecoiledJets_(0,0,0,0)
  , p4SumOther_(0,0,0,0)
  , vetoNear_(false)
  , vetoFwd_(false)
  , ptHardestInRecoil_(0.0)
{
    loadConfig("config/PickMultiJet.json");
}

PickMultiJet::~PickMultiJet() = default;

// Load configuration from JSON file and store values in private members
void PickMultiJet::loadConfig(const std::string& filename) {
    ReadConfig config(filename);

    // Jet pick configuration
    minPtProbe_      = config.getValue<double>({"pickJet","minPtProbe"});
    jetIdLabel_     = config.getValue<std::string>({"pickJet", "jetIdLabel"});
    maxEtaRecoil_   = config.getValue<double>({"pickJet","maxEtaRecoil"});
    minPtRecoil_    = config.getValue<double>({"pickJet","minPtRecoil"});
    minPtOther_     = config.getValue<double>({"pickJet","minPtOther"});
    minDPhiRecoil_  = config.getValue<double>({"pickJet","minDPhiRecoil"});
}

void PickMultiJet::printDebug(const std::string& msg) const {
    if (isDebug_) std::cout << "[PickMultiJet] " << msg << "\n";
    //std::cout << "[PickMultiJet] " << msg << "\n";
}

void PickMultiJet::pickJets(const SkimTree& skimT) {
    // reset everything
    iProbe_            = -1;
    recoilIndices_.clear();
    p4Probe_.SetPtEtaPhiM(0,0,0,0);
    p4SumRecoiledJets_.SetPtEtaPhiM(0,0,0,0);
    p4SumOther_.SetPtEtaPhiM(0,0,0,0);
    vetoNear_           = false;
    vetoFwd_            = false;
    ptHardestInRecoil_  = 0.0;

    printDebug("Starting pick() with nJet=" + std::to_string(skimT.nJet));

    // 1) find highest-pT jet
    double maxPt = 0.0;
    for (int i = 0; i < skimT.nJet; ++i) {
        double pt = skimT.Jet_pt[i];
        if (pt > maxPt) {
            maxPt = pt;
            iProbe_ = i;
        }
    }
    printDebug("Found lead jet idx=" + std::to_string(iProbe_) +
               " pt=" + std::to_string(maxPt));

    // 2) require pT & ID
    if (iProbe_ < 0 || maxPt < minPtProbe_) {
        printDebug("→ no valid lead or below pT threshold");
        iProbe_ = -1;
        return;
    }
    if (!pickJet_.passId(skimT, iProbe_, jetIdLabel_)) {
        printDebug("→ lead fails jetId, rejecting");
        iProbe_ = -1;
        return;
    }

    // store lead four‐vector
    p4Probe_.SetPtEtaPhiM(skimT.Jet_pt[iProbe_],
                            skimT.Jet_eta[iProbe_],
                            skimT.Jet_phi[iProbe_],
                            skimT.Jet_mass[iProbe_]);

    // 3) loop all other jets → classify
    for (int i = 0; i < skimT.nJet; ++i) {
        if (i == iProbe_) continue;

        TLorentzVector p4;
        p4.SetPtEtaPhiM(skimT.Jet_pt[i],
                        skimT.Jet_eta[i],
                        skimT.Jet_phi[i],
                        skimT.Jet_mass[i]);

        double pt  = p4.Pt();
        double eta = p4.Eta();
        double dphi= HelperDelta::DELTAPHI(p4.Phi(), p4Probe_.Phi());

        // recoil criteria
        if (pt > minPtRecoil_ && fabs(eta) < maxEtaRecoil_ && dphi > minDPhiRecoil_) {
            if (!pickJet_.passId(skimT, i, jetIdLabel_)) {
                printDebug("→ a recoil jet fails ID, rejecting whole event");
                // treat as event‐level fail:
                iProbe_ = -1;
                return;
            }
            recoilIndices_.push_back(i);
            p4SumRecoiledJets_ += p4;
            ptHardestInRecoil_ = std::max(ptHardestInRecoil_, pt);
        }
        // “other” jets
        else if (pt > minPtOther_) {
            p4SumOther_ += p4;
        }

        // to veto events if there are near‐by jets
        if (pt > minPtRecoil_ && fabs(eta) < maxEtaRecoil_ && dphi <= minDPhiRecoil_) {
            vetoNear_ = true;
        }
        // to veto events if there are forward jets
        if (pt > minPtRecoil_ && fabs(eta) >= maxEtaRecoil_) {
            vetoFwd_ = true;
        }
    }

    printDebug("→ recoil count=" + std::to_string(recoilIndices_.size()) +
               " vetoNear=" + std::to_string(vetoNear_) +
               " vetoFwd="  + std::to_string(vetoFwd_));
}

