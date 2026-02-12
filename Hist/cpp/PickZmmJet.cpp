#include "PickZmmJet.h"
#include "ReadConfig.h"

PickZmmJet::PickZmmJet(const GlobalFlag& globalFlags)
    : globalFlags_(globalFlags),
      year_(globalFlags_.getYear()),
      channel_(globalFlags_.getChannel()),
      isDebug_(globalFlags_.isDebug()),
      zJet_(globalFlags_)
{
    loadConfig("config/PickZmmJet.json");
}

PickZmmJet::~PickZmmJet() = default;

void PickZmmJet::loadConfig(const std::string& filename) {
    ReadConfig config(filename);

    minPtLeadMu_    = config.getValue<double>({"muonPick", "minPtLead"});
    minPtSubLeadMu_ = config.getValue<double>({"muonPick", "minPtSubLead"});
    maxEtaMu_    = config.getValue<double>({"muonPick", "maxEta"});
    tightIdMu_   = config.getValue<int>   ({"muonPick", "tightId"});
    maxRelIsoMu_ = config.getValue<double>({"muonPick", "maxRelIso"});
    maxDxyMu_    = config.getValue<double>({"muonPick", "maxDxy"});
    maxDzMu_     = config.getValue<double>({"muonPick", "maxDz"});

    pdgIdGenMu_ = config.getValue<int>({"genMuonPick", "pdgId"});
}

void PickZmmJet::printDebug(const std::string& message) const {
    if (isDebug_) std::cout << "[PickZmmJet] " << message << '\n';
}

void PickZmmJet::pickMuons(const SkimTree& skimT) {
    printDebug("pickMuons: nMuon = " + std::to_string(skimT.nMuon));
    pickedMuons_.clear();

    // --------------------------------------------------
    // Find leading muon index (highest pT)
    // --------------------------------------------------
    int leadIdx = -1;
    double maxPt = -1.0;

    for (int i = 0; i < skimT.nMuon; ++i) {
        if (skimT.Muon_pt[i] > maxPt) {
            maxPt = skimT.Muon_pt[i];
            leadIdx = i;
        }
    }

    // --------------------------------------------------
    // Apply selection
    // --------------------------------------------------
    for (int i = 0; i < skimT.nMuon; ++i) {
        double eta = skimT.Muon_eta[i];
        double pt  = skimT.Muon_pt[i];

        const double minPtCut =
            (i == leadIdx ? minPtLeadMu_ : minPtSubLeadMu_);

        bool sel =
            (std::abs(eta) <= maxEtaMu_ &&
             pt >= minPtCut &&
             skimT.Muon_tightId[i] == tightIdMu_ &&
             skimT.Muon_pfRelIso04_all[i] < maxRelIsoMu_ &&
             std::abs(skimT.Muon_dxy[i]) < maxDxyMu_ &&
             std::abs(skimT.Muon_dz[i])  < maxDzMu_);

        if (sel) {
            pickedMuons_.push_back(i);
            printDebug("Muon " + std::to_string(i) +
                       (i == leadIdx ? " (LEAD)" : " (SUBLEAD)") +
                       " selected, pt=" + std::to_string(pt) +
                       ", eta=" + std::to_string(eta));
        } else {
            printDebug("Muon " + std::to_string(i) +
                       (i == leadIdx ? " (LEAD)" : " (SUBLEAD)") +
                       " rejected, pt=" + std::to_string(pt) +
                       ", eta=" + std::to_string(eta));
        }
    }

    printDebug("pickMuons: total picked = " +
               std::to_string(pickedMuons_.size()));
}


void PickZmmJet::pickTags(const SkimTree& skimT) {
    pickedTags_ = zJet_.pickRecoZ(
        pickedMuons_,
        skimT.Muon_pt,
        skimT.Muon_eta,
        skimT.Muon_phi,
        skimT.Muon_mass,
        skimT.Muon_charge);

    printDebug("pickTags: nTags = " + std::to_string(pickedTags_.size()));
}

void PickZmmJet::pickJets(const SkimTree& skimT, const TLorentzVector& p4Tag) {
    pickedJetsP4_ = zJet_.pickRecoJets(
        skimT,
        pickedMuons_,
        skimT.Muon_eta,
        skimT.Muon_phi,
        skimT.Jet_muonIdx1,
        skimT.Jet_muonIdx2,
        pickedJetsIndex_);

    printDebug("pickJets: iJet1=" + std::to_string(pickedJetsIndex_.at(0)) +
               ", iJet2=" + std::to_string(pickedJetsIndex_.at(1)));
}

void PickZmmJet::pickGenMuons(const SkimTree& skimT) {
    pickedGenMuons_.clear();
    printDebug("pickGenMuons: nGenDressedLepton = " + std::to_string(skimT.nGenDressedLepton));

    for (int i = 0; i < skimT.nGenDressedLepton; ++i) {
        if (std::abs(skimT.GenDressedLepton_pdgId[i]) == pdgIdGenMu_) {
            pickedGenMuons_.push_back(i);
            printDebug("Gen muon " + std::to_string(i) + " picked");
        }
    }

    printDebug("pickGenMuons: total picked = " +
               std::to_string(pickedGenMuons_.size()));
}

void PickZmmJet::pickGenTags(const SkimTree& skimT, const TLorentzVector& p4Tag) {
    if (channel_ != GlobalFlag::Channel::ZmmJet) {
        pickedGenTags_.clear();
        return;
    }

    pickedGenTags_ = zJet_.pickGenZ(skimT, p4Tag, pickedGenMuons_);
    printDebug("pickGenTags: nGenTags = " + std::to_string(pickedGenTags_.size()));
}

