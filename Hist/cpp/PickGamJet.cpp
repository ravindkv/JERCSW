#include "PickGamJet.h"

#include "PickGuard.hpp"
#include "ReadConfig.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace {
constexpr double kEps = 1e-12;
}

PickGamJet::PickGamJet(const GlobalFlag& globalFlags)
    : globalFlags_(globalFlags),
      year_(globalFlags_.getYear()),
      channel_(globalFlags_.getChannel()),
      isDebug_(globalFlags_.isDebug()),
      pickJet_(globalFlags_)
{
    loadConfig("config/PickGamJet.json");
    validateConfig_();
}

PickGamJet::~PickGamJet() = default;

void PickGamJet::printDebug(const std::string& message) const {
    if (isDebug_) std::cout << "[PickGamJet] " << message << '\n';
}

void PickGamJet::resetReco_() {
    pickedPhotons_.clear();
    pickedJetsP4_.clear();
    pickedJetsIndex_.clear();
}

void PickGamJet::resetGen_() {
    pickedGenPhotons_.clear();
    pickedGenTags_.clear();
}

// Load configuration from JSON file and store values in private members
void PickGamJet::loadConfig(const std::string& filename) {
    ReadConfig config(filename);

    // Photon pick configuration
    minPtPho_        = config.getValue<double>({"photonPick", "minPt"});
    maxEtaPho_       = config.getValue<double>({"photonPick", "maxEta"});
    tightIdPho_      = config.getValue<int>   ({"photonPick", "tightId"});
    minR9Pho_        = config.getValue<double>({"photonPick", "minR9"});
    maxR9Pho_        = config.getValue<double>({"photonPick", "maxR9"});
    maxHoePho_       = config.getValue<double>({"photonPick", "maxHoe"});
    isEleVetoPho_    = config.getValue<bool>  ({"photonPick", "isEleVeto"});
    hasPixelSeedPho_ = config.getValue<bool>  ({"photonPick", "hasPixelSeed"});

    // Jet pick configuration
    minPtJet_         = config.getValue<double>({"jetPick", "minPt"});
    maxEtaProbeJet_ = config.getValue<double>({"jetPick", "maxEtaProbe"});
    jetIdLabel_       = config.getValue<std::string>({"jetPick", "jetIdLabel"});
    minDeltaRrefJet_  = config.getValue<double>({"jetPick", "minDeltaR"});
    applyJetIdToJet2_ = config.getValue<bool>({"jetPick","applyJetIdToJet2"}); 
    applyEtaToJet2_   = config.getValue<bool>({"jetPick","applyEtaToJet2"}); 

    // Gen Photon pick configuration
    pdgIdGenPho_ = config.getValue<int>({"genPhotonPick", "pdgId"});
    minPtGenPho_        = config.getValue<double>({"genPhotonPick", "minPt"});

    // Gen Tag pick configuration
    maxDeltaRgenTag_ = config.getValue<double>({"genTagPick", "maxDeltaR"});
    minPtGenTag_     = config.getValue<double>({"genTagPick", "minPt"});

    if(globalFlags_.getJetAlgo()==GlobalFlag::JetAlgo::AK8Puppi){
        minPtPho_       = config.getValue<double>({"photonPick", "minPtForAK8"});
        minPtJet_       = config.getValue<double>({"jetPick", "minPtForAK8"});
        minPtGenPho_    = config.getValue<double>({"genPhotonPick", "minPtForAK8"});
        minPtGenTag_    = config.getValue<double>({"genTagPick", "minPtForAK8"});
    }

}

void PickGamJet::validateConfig_() const {
    if (!std::isfinite(minPtPho_) || minPtPho_ <= 0.0) throw std::runtime_error("PickGamJet config invalid: photonPick.minPt must be > 0");
    if (!std::isfinite(maxEtaPho_) || maxEtaPho_ <= 0.0) throw std::runtime_error("PickGamJet config invalid: photonPick.maxEta must be > 0");
    if (!std::isfinite(minR9Pho_) || !std::isfinite(maxR9Pho_) || minR9Pho_ >= maxR9Pho_) throw std::runtime_error("PickGamJet config invalid: photonPick R9 range");
    if (!std::isfinite(maxHoePho_) || maxHoePho_ < 0.0) throw std::runtime_error("PickGamJet config invalid: photonPick.maxHoe must be >= 0");

    if (!std::isfinite(minPtJet_) || minPtJet_ <= 0.0) throw std::runtime_error("PickGamJet config invalid: jetPick.minPt must be > 0");
    if (!std::isfinite(maxEtaProbeJet_) || maxEtaProbeJet_ <= 0.0) throw std::runtime_error("PickGamJet config invalid: jetPick.maxEtaProbe must be > 0");
    if (jetIdLabel_.empty()) throw std::runtime_error("PickGamJet config invalid: jetPick.jetIdLabel is empty");
    if (!std::isfinite(minDeltaRrefJet_) || minDeltaRrefJet_ < 0.0) throw std::runtime_error("PickGamJet config invalid: jetPick.minDeltaR must be >= 0");

    if (!std::isfinite(maxDeltaRgenTag_) || maxDeltaRgenTag_ < 0.0) throw std::runtime_error("PickGamJet config invalid: genTagPick.maxDeltaR must be >= 0");
}

// Photon selection
void PickGamJet::pickPhotons(const SkimTree& skimT) {
    PickGuard g("PickGamJet::pickPhotons");

    resetReco_();
    pickedPhotons_.clear();

    g.require(skimT.nPhoton >= 0, "NEG_NPHOTON", "skimT.nPhoton is negative: " + std::to_string(skimT.nPhoton));
    printDebug("Starting pickPhotons, nPhoton=" + std::to_string(skimT.nPhoton));

    for (int phoInd = 0; phoInd < skimT.nPhoton; ++phoInd) {
        const double pt      = skimT.Photon_pt[phoInd];
        const double eta     = skimT.Photon_eta[phoInd];
        const double absEta  = std::abs(eta);
        const double r9      = skimT.Photon_r9[phoInd];
        const double hoe     = skimT.Photon_hoe[phoInd];
        const bool   eleVeto = skimT.Photon_electronVeto[phoInd];
        const bool   pixSeed = skimT.Photon_pixelSeed[phoInd];
        const int    id      = skimT.Photon_cutBased[phoInd];

        g.requireFinite(pt,  "Photon_pt");
        g.requireFinite(eta, "Photon_eta");
        g.requireFinite(r9,  "Photon_r9");
        g.requireFinite(hoe, "Photon_hoe");

        const bool pass =
            (pt > minPtPho_) &&
            (absEta < maxEtaPho_) &&
            (r9 > minR9Pho_) && (r9 < maxR9Pho_) &&
            (hoe < maxHoePho_) &&
            (id == tightIdPho_) &&
            (eleVeto == isEleVetoPho_) &&
            (pixSeed == hasPixelSeedPho_);

        if (pass) pickedPhotons_.push_back(phoInd);

        if (isDebug_) {
            printDebug("Photon " + std::to_string(phoInd) +
                       " id=" + std::to_string(id) +
                       " pt=" + std::to_string(pt) +
                       " absEta=" + std::to_string(absEta) +
                       " hoe=" + std::to_string(hoe) +
                       " r9=" + std::to_string(r9) +
                       (pass ? "  PASS" : "  fail"));
        }
    }

    printDebug("Total Photons Selected: " + std::to_string(pickedPhotons_.size()));
}

// Tag object picking (Photon -> Tag p4)
// Pick the leading (highest pT) photon as the single tag
void PickGamJet::pickTag(const SkimTree& skimT) {
    PickGuard g("PickGamJet::pickTag");

    pickedTagP4.SetPtEtaPhiM(0,0,0,0);
    pickedTagIndex_ = -1;

    g.require(!pickedPhotons_.empty(),
              "NO_PHOTON",
              "pickedPhotons_ is empty — no tag photon available");

    // Find photon with max pT
    int leadIdx = -1;
    float maxPt = -1.f;

    for (int idx : pickedPhotons_) {
        g.requireIndexInRange(idx, skimT.nPhoton, "pickedPhotons_");

        const float pt = skimT.Photon_pt[idx];
        g.requireFinite(pt, "Photon_pt");

        if (pt > maxPt) {
            maxPt  = pt;
            leadIdx = idx;
        }
    }

    g.require(leadIdx >= 0, "NO_LEADING_PHOTON", "Failed to find leading photon");

    pickedTagIndex_ = leadIdx;
    // Identify the photon->jet index, if present
    phoJetIdx_ = -1;
    if (pickedTagIndex_ != -1) {
        g.requireIndexInRange(pickedTagIndex_, skimT.nPhoton, "pickedPhotons_(front)");
        phoJetIdx_ = skimT.Photon_jetIdx[pickedTagIndex_];
        printDebug("photon->jetIdx = " + std::to_string(phoJetIdx_));
    }

    pickedTagP4 = g.makeP4(
        skimT.Photon_pt[leadIdx],
        skimT.Photon_eta[leadIdx],
        skimT.Photon_phi[leadIdx],
        skimT.Photon_mass[leadIdx],
        "Photon_p4"
    );
    printDebug("Leading photon selected as tag: idx=" +
               std::to_string(leadIdx) +
               ", pt=" + std::to_string(maxPt));

    g.requireFinite(pickedTagP4.Pt(), "pickedTagP4");
    printDebug("Total Tag Objects Selected: 1");
}


/* TO BE IMPLEMENTED //FIXME
    // If the real photon has  leaked to HCAL then the differene between photon and associaated jet energy will be large. The idea is to reject such event.
 *
 * Photon “footprint / protection” object phoj

    This is used only to form a “leakage” variable (pass_leak), and is built from the jet that NanoAOD associates to the photon:

    If Photon_jetIdx[iGam] != -1:

    take that jet index idx

    construct phoj from Jet_pt[idx], Jet_eta[idx], Jet_phi[idx], Jet_mass[idx]

    convert to raw by multiplying (1 - Jet_rawFactor[idx])

    require ΔR(rawgam, phoj) < 0.4

    subtract the photon four-vector from the jet: phoj -= rawgam

    comment notes: in Run-3 the photon-jet association / ΔR condition “does not always hold”, but they still enforce it; if it fails they zero the vector and skip the event.

    Then they apply an L1RC correction to phoj in Run-2 (Run-3 uses corrl1rc=1).

    This phoj is later used for:

    pass_leak = (phoj.Pt() < 0.06 * ptgam)
 */

void PickGamJet::pickJets(const SkimTree& skimT, const TLorentzVector& p4Tag) {
    PickGuard g("PickGamJet::pickJets");

    pickedJetsIndex_.clear();
    pickedJetsP4_.clear();

    g.require(skimT.nJet >= 0, "NEG_NJET", "skimT.nJet is negative: " + std::to_string(skimT.nJet));

    // Basic sanity for reference p4 (DeltaR uses eta/phi)
    g.requireFinite(p4Tag.Pt(),  "p4Tag.Pt()");
    g.requireFinite(p4Tag.Eta(), "p4Tag.Eta()");
    g.requireFinite(p4Tag.Phi(), "p4Tag.Phi()");

    printDebug("pickJets: Starting, nJet=" + std::to_string(skimT.nJet));

    // Collect jet candidates passing basic pt and not being the photon-associated jet
    std::vector<int> cand;
    cand.reserve(std::min<int>(skimT.nJet, 64));

    for (int i = 0; i < skimT.nJet; ++i) {
        const double pt  = skimT.Jet_pt[i];
        g.requireFinite(pt,  "Jet_pt");
        if (pt < minPtJet_) continue;
        if (i == phoJetIdx_) continue;//Somehow phoJetIdx_ is always 0, so probeJet is always subleading /FIXME
        const double eta = skimT.Jet_eta[i];
        const double phi = skimT.Jet_phi[i];
        const double m   = skimT.Jet_mass[i];

        g.requireFinite(eta, "Jet_eta");
        g.requireFinite(phi, "Jet_phi");
        g.requireFinite(m,   "Jet_mass");

        cand.push_back(i);
    }

    std::sort(cand.begin(), cand.end(),
              [&](int a, int b) { return skimT.Jet_pt[a] > skimT.Jet_pt[b]; });

    int iJet1 = (cand.size() >= 1) ? cand[0] : -1;
    int iJet2 = (cand.size() >= 2) ? cand[1] : -1;

    printDebug("Top-2 pt candidates: iJet1=" + std::to_string(iJet1) +
               " iJet2=" + std::to_string(iJet2));

    auto passDeltaR = [&](int jetIdx) -> bool {
        if (jetIdx < 0) return false;

        const auto p4Jet = g.makeP4(skimT.Jet_pt[jetIdx],
                                    skimT.Jet_eta[jetIdx],
                                    skimT.Jet_phi[jetIdx],
                                    skimT.Jet_mass[jetIdx],
                                    "Jet_p4_for_dR");

        const double dR = p4Tag.DeltaR(p4Jet);
        g.requireFinite(dR, "DeltaR(ref,jet)");

        return (dR + kEps >= minDeltaRrefJet_);
    };

    auto passJetIdEta = [&](int jetIdx, bool applyId, bool applyEta) -> bool {
        if (jetIdx < 0) return false;

        if (applyId && !pickJet_.passId(skimT, jetIdx, jetIdLabel_)) return false;

        if (applyEta) {
            const double eta = skimT.Jet_eta[jetIdx];
            g.requireFinite(eta, "Jet_eta(for eta cut)");
            if (std::abs(eta) > maxEtaProbeJet_) return false;
        }
        return true;
    };

    // Apply JetID/eta (always to jet1; optional to jet2)
    if (globalFlags_.getJecDerivationLevel() == GlobalFlag::JecDerivationLevel::L3Residual) {
        if (iJet1 != -1 && !passJetIdEta(iJet1, true, true)) {
            printDebug("iJet1 fails JetID/eta -> drop");
            iJet1 = -1;
        }
    }else{
        if (iJet1 != -1 && !passJetIdEta(iJet1, true, false)) {// no eta-cut for L2Residual
            printDebug("iJet1 fails JetID/eta -> drop");
            iJet1 = -1;
        }
    }
    if (iJet2 != -1 && (applyJetIdToJet2_ || applyEtaToJet2_) &&
        !passJetIdEta(iJet2, applyJetIdToJet2_, applyEtaToJet2_)) {
        printDebug("iJet2 fails JetID/eta -> drop");
        iJet2 = -1;
    }

    // Apply DeltaR cut to both
    if (iJet1 != -1 && !passDeltaR(iJet1)) {
        printDebug("iJet1 fails dR(ref,jet) -> drop");
        iJet1 = -1;
    }
    if (iJet2 != -1 && !passDeltaR(iJet2)) {
        printDebug("iJet2 fails dR(ref,jet) -> drop");
        iJet2 = -1;
    }

    printDebug("After JetID/eta/dR: iJet1=" + std::to_string(iJet1) +
               " iJet2=" + std::to_string(iJet2));

    // Build p4 outputs: always return 3 vectors [jet1, jet2, sumOther]
    TLorentzVector p4Jet1(0,0,0,0), p4Jet2(0,0,0,0), p4SumOther(0,0,0,0);

    for (int i = 0; i < skimT.nJet; ++i) {
        const auto p4 = g.makeP4(skimT.Jet_pt[i],
                                 skimT.Jet_eta[i],
                                 skimT.Jet_phi[i],
                                 skimT.Jet_mass[i],
                                 "Jet_p4");

        if (i == iJet1)       p4Jet1 = p4;
        else if (i == iJet2)  p4Jet2 = p4;
        else                  p4SumOther += p4;
    }

    pickedJetsP4_.push_back(p4Jet1);
    pickedJetsP4_.push_back(p4Jet2);
    pickedJetsP4_.push_back(p4SumOther);

    pickedJetsIndex_.push_back(iJet1);
    pickedJetsIndex_.push_back(iJet2);

    // Contracts
    g.requireSize(pickedJetsP4_.size(), 3, "pickedJetsP4_");
    g.requireAllFinite(pickedJetsP4_, "pickedJetsP4_");

    for (int idx : pickedJetsIndex_) {
        g.requireIndexInRange(idx, skimT.nJet, "pickedJetsIndex_");
    }

    printDebug("pickJets: done. pickedJetsIndex_.size=" + std::to_string(pickedJetsIndex_.size()));
}

void PickGamJet::pickGenPhotons(const SkimTree& skimT) {
    PickGuard g("PickGamJet::pickGenPhotons");

    resetGen_();
    pickedGenPhotons_.clear();

    g.require(skimT.nGenIsolatedPhoton >= 0, "NEG_NGENPHO",
              "nGenIsolatedPhoton is negative: " + std::to_string(skimT.nGenIsolatedPhoton));

    printDebug("pickGenPhotons: nGenIsolatedPhoton=" + std::to_string(skimT.nGenIsolatedPhoton));

    // Current behavior: keep all (pdgIdGenPho_ currently unused).
    for (int i = 0; i < skimT.nGenIsolatedPhoton; ++i) {
        if(skimT.GenIsolatedPhoton_pt[i] < minPtGenPho_) continue;
        pickedGenPhotons_.push_back(i);
    }

    printDebug("Total Gen Photons Selected: " + std::to_string(pickedGenPhotons_.size()));
}

void PickGamJet::pickGenTags(const SkimTree& skimT, const TLorentzVector& p4Tag) {
    PickGuard g("PickGamJet::pickGenTags");

    pickedGenTags_.clear();

    g.requireFinite(p4Tag.Pt(),  "p4Tag.Pt(genTagMatch)");
    g.requireFinite(p4Tag.Eta(), "p4Tag.Eta(genTagMatch)");
    g.requireFinite(p4Tag.Phi(), "p4Tag.Phi(genTagMatch)");

    if (channel_ == GlobalFlag::Channel::GamJet && !pickedGenPhotons_.empty()) {
        for (int idx : pickedGenPhotons_) {
            g.requireIndexInRange(idx, skimT.nGenIsolatedPhoton, "pickedGenPhotons_");

            const auto p4GenTag = g.makeP4(skimT.GenIsolatedPhoton_pt[idx],
                                           skimT.GenIsolatedPhoton_eta[idx],
                                           skimT.GenIsolatedPhoton_phi[idx],
                                           skimT.GenIsolatedPhoton_mass[idx],
                                           "GenIsolatedPhoton_p4");

            const double dR = p4GenTag.DeltaR(p4Tag);
            g.requireFinite(dR, "DeltaR(genTag,ref)");
            if (dR > maxDeltaRgenTag_) continue;

            pickedGenTags_.push_back(p4GenTag);
        }
    }

    g.requireAllFinite(pickedGenTags_, "pickedGenTags_");
    printDebug("Total Gen Tag Objects Picked: " + std::to_string(pickedGenTags_.size()));
}

