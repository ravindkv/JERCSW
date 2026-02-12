#include "PickZeeJet.h"
#include "ReadConfig.h"
#include "Helper.hpp"

PickZeeJet::PickZeeJet(const GlobalFlag& globalFlags)
    : globalFlags_(globalFlags),
      year_(globalFlags_.getYear()),
      channel_(globalFlags_.getChannel()),
      isDebug_(globalFlags_.isDebug()),
      zJet_(globalFlags_)        // load common Z+jet config
{
    loadConfig("config/PickZeeJet.json");
}

PickZeeJet::~PickZeeJet() = default;

void PickZeeJet::loadConfig(const std::string& filename) {
    ReadConfig config(filename);

    minPtLeadEle_    = config.getValue<double>({"electronPick", "minPtLead"});
    minPtSubLeadEle_ = config.getValue<double>({"electronPick", "minPtSubLead"});
    maxEtaEle_  = config.getValue<double>({"electronPick", "maxEta"});
    tightIdEle_ = config.getValue<int>   ({"electronPick", "tightId"});
    minEbEeGap_ = config.getValue<double>({"electronPick", "ebEeGap", "min"});
    maxEbEeGap_ = config.getValue<double>({"electronPick", "ebEeGap", "max"});

    pdgIdGenEle_ = config.getValue<int>({"genElectronPick", "pdgId"});
}

void PickZeeJet::printDebug(const std::string& message) const {
    if (isDebug_) std::cout << "[PickZeeJet] " << message << '\n';
}

void PickZeeJet::pickElectrons(const SkimTree& skimT) {
    printDebug("pickElectrons: nElectron = " +
               std::to_string(skimT.nElectron));
    pickedElectrons_.clear();

    // --------------------------------------------------
    // Find leading electron (highest pT)
    // --------------------------------------------------
    int leadIdx = -1;
    double maxPt = -1.0;

    for (int i = 0; i < skimT.nElectron; ++i) {
        if (skimT.Electron_pt[i] > maxPt) {
            maxPt = skimT.Electron_pt[i];
            leadIdx = i;
        }
    }

    // --------------------------------------------------
    // Apply selection
    // --------------------------------------------------
    for (int i = 0; i < skimT.nElectron; ++i) {
        double eta      = skimT.Electron_eta[i];
        double absEta   = std::abs(eta);
        double scEta    = eta + skimT.Electron_deltaEtaSC[i];
        double absSCEta = std::abs(scEta);
        double pt       = skimT.Electron_pt[i];

        bool passGap = (absSCEta < minEbEeGap_) ||
                       (absSCEta > maxEbEeGap_);

        bool passId  = (skimT.Electron_cutBased[i] >= tightIdEle_);

        const double minPtCut =
            (i == leadIdx ? minPtLeadEle_ : minPtSubLeadEle_);

        bool sel =
            passGap &&
            (absEta <= maxEtaEle_) &&
            (pt >= minPtCut) &&
            passId;

        if (sel) {
            pickedElectrons_.push_back(i);
            printDebug("Electron " + std::to_string(i) +
                       (i == leadIdx ? " (LEAD)" : " (SUBLEAD)") +
                       " selected, pt=" + std::to_string(pt) +
                       ", eta=" + std::to_string(eta));
        } else {
            printDebug("Electron " + std::to_string(i) +
                       (i == leadIdx ? " (LEAD)" : " (SUBLEAD)") +
                       " rejected, pt=" + std::to_string(pt) +
                       ", eta=" + std::to_string(eta) +
                       ", passId=" + std::to_string(passId));
        }
    }

    printDebug("pickElectrons: total picked = " +
               std::to_string(pickedElectrons_.size()));
}


void PickZeeJet::pickTags(const SkimTree& skimT) {
    pickedTags_ = zJet_.pickRecoZ(
        pickedElectrons_,
        skimT.Electron_pt,
        skimT.Electron_eta,
        skimT.Electron_phi,
        skimT.Electron_mass,
        skimT.Electron_charge);

    printDebug("pickTags: nTags = " + std::to_string(pickedTags_.size()));
}

void PickZeeJet::pickJets(const SkimTree& skimT, const TLorentzVector& p4Tag) {
    pickedJetsP4_ = zJet_.pickRecoJets(
        skimT,
        pickedElectrons_,
        skimT.Electron_eta,
        skimT.Electron_phi,
        skimT.Jet_electronIdx1,
        skimT.Jet_electronIdx2,
        pickedJetsIndex_);

    printDebug("pickJets: iJet1=" + std::to_string(pickedJetsIndex_.at(0)) +
               ", iJet2=" + std::to_string(pickedJetsIndex_.at(1)));
}

void PickZeeJet::pickGenElectrons(const SkimTree& skimT) {
    pickedGenElectrons_.clear();
    printDebug("pickGenElectrons: nGenDressedLepton = " + std::to_string(skimT.nGenDressedLepton));

    for (int i = 0; i < skimT.nGenDressedLepton; ++i) {
        if (std::abs(skimT.GenDressedLepton_pdgId[i]) == pdgIdGenEle_) {
            pickedGenElectrons_.push_back(i);
            printDebug("Gen electron " + std::to_string(i) + " picked");
        }
    }

    printDebug("pickGenElectrons: total picked = " +
               std::to_string(pickedGenElectrons_.size()));
}

void PickZeeJet::pickGenTags(const SkimTree& skimT, const TLorentzVector& p4Tag) {
    if (channel_ != GlobalFlag::Channel::ZeeJet) {
        pickedGenTags_.clear();
        return;
    }

    pickedGenTags_ = zJet_.pickGenZ(skimT, p4Tag, pickedGenElectrons_);
    printDebug("pickGenTags: nGenTags = " + std::to_string(pickedGenTags_.size()));
}

