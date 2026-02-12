#include "PickZJet.h"

#include "PickGuard.hpp"
#include "ReadConfig.h"
#include "HelperDelta.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <sstream>
#include <stdexcept>

namespace {
constexpr double kEps = 1e-12;
}

PickZJet::PickZJet(const GlobalFlag& globalFlags)
    : globalFlags_(globalFlags),
      pickJet_(globalFlags_),
      isDebug_(globalFlags_.isDebug())
{
    loadConfig("config/PickZJet.json");
    validateConfig_();
}

void PickZJet::printDebug(const std::string& msg) const {
    if (isDebug_) std::cout << "[PickZJet] " << msg << '\n';
}

void PickZJet::loadConfig(const std::string& filename) {
    ReadConfig config(filename);

    massTag_       = config.getValue<double>({"tagPick", "mass"});
    massWindowTag_ = config.getValue<double>({"tagPick", "massWindow"});
    minPtTag_      = config.getValue<double>({"tagPick", "minPt"});
    maxEtaTag_      = config.getValue<double>({"tagPick", "maxEta"});
    nLepMin_       = config.getValue<int>({"tagPick", "nLepMin"});
    nLepMax_       = config.getValue<int>({"tagPick", "nLepMax"});

    minPtJet_            = config.getValue<double>({"jetPick", "minPt"});
    maxEtaProbeJet_      = config.getValue<double>({"jetPick", "maxEtaProbe"});
    minDeltaRtoLepton_   = config.getValue<double>({"jetPick", "minDeltaRtoLepton"});
    jetIdLabel_          = config.getValue<std::string>({"jetPick", "jetIdLabel"});
    requiredJet_lepIdx1_ = config.getValue<int>({"jetPick", "requiredJet_lepIdx1"});
    requiredJet_lepIdx2_ = config.getValue<int>({"jetPick", "requiredJet_lepIdx2"});

    maxDeltaRgenTag_ = config.getValue<double>({"genTagPick", "maxDeltaR"});
    minPtGenTag_      = config.getValue<double>({"genTagPick", "minPt"});

    if(globalFlags_.getJetAlgo()==GlobalFlag::JetAlgo::AK8Puppi){
        minPtTag_      = config.getValue<double>({"tagPick", "minPtForAK8"});
        minPtJet_      = config.getValue<double>({"jetPick", "minPtForAK8"});
        minPtGenTag_      = config.getValue<double>({"genTagPick", "minPtForAK8"});
    }
}

void PickZJet::validateConfig_() const {
    if (!std::isfinite(massTag_) || massTag_ <= 0.0)
        throw std::runtime_error("PickZJet config invalid: tagPick.mass must be > 0");
    if (!std::isfinite(massWindowTag_) || massWindowTag_ <= 0.0)
        throw std::runtime_error("PickZJet config invalid: tagPick.massWindow must be > 0");
    if (!std::isfinite(minPtTag_) || minPtTag_ < 0.0)
        throw std::runtime_error("PickZJet config invalid: tagPick.minPt must be >= 0");
    if (!std::isfinite(maxEtaTag_) || maxEtaTag_ < 0.0)
        throw std::runtime_error("PickZJet config invalid: tagPick.maxPt must be >= 0");
    if (nLepMin_ < 2)
        throw std::runtime_error("PickZJet config invalid: tagPick.nLepMin must be >= 2");
    if (nLepMax_ < nLepMin_)
        throw std::runtime_error("PickZJet config invalid: tagPick.nLepMax must be >= nLepMin");

    if (!std::isfinite(minPtJet_) || minPtJet_ < 0.0)
        throw std::runtime_error("PickZJet config invalid: jetPick.minPt must be >= 0");
    if (!std::isfinite(maxEtaProbeJet_) || maxEtaProbeJet_ <= 0.0)
        throw std::runtime_error("PickZJet config invalid: jetPick.maxEtaProbe must be > 0");
    if (!std::isfinite(minDeltaRtoLepton_) || minDeltaRtoLepton_ < 0.0)
        throw std::runtime_error("PickZJet config invalid: jetPick.minDeltaRtoLepton must be >= 0");
    if (jetIdLabel_.empty())
        throw std::runtime_error("PickZJet config invalid: jetPick.jetIdLabel is empty");

    if (!std::isfinite(maxDeltaRgenTag_) || maxDeltaRgenTag_ < 0.0)
        throw std::runtime_error("PickZJet config invalid: genTagPick.maxDeltaR must be >= 0");
}

// ------------------- Reco Z boson ---------------------------------

std::vector<TLorentzVector> PickZJet::pickRecoZ(
    const std::vector<int>& pickedLeptons,
    const float* lep_pt,
    const float* lep_eta,
    const float* lep_phi,
    const float* lep_mass,
    const int*   lep_charge) const
{
    PickGuard g("PickZJet::pickRecoZ");

    std::vector<TLorentzVector> outTags;

    g.requireNonNull(lep_pt,     "lep_pt");
    g.requireNonNull(lep_eta,    "lep_eta");
    g.requireNonNull(lep_phi,    "lep_phi");
    g.requireNonNull(lep_mass,   "lep_mass");
    g.requireNonNull(lep_charge, "lep_charge");

    const std::size_t nLep = pickedLeptons.size();
    printDebug("pickRecoZ: nLeptons = " + std::to_string(nLep));

    if (nLep < static_cast<std::size_t>(nLepMin_)) return outTags;

    if (nLep > static_cast<std::size_t>(nLepMax_)) {
        printDebug("pickRecoZ: more than nLepMax -> veto event.");
        return outTags;
    }

    bool foundBest = false;
    TLorentzVector bestP4;
    double bestDiff = std::numeric_limits<double>::max();

    for (std::size_t a = 0; a < nLep; ++a) {
        const int idxA = pickedLeptons[a];
        g.require(idxA >= 0, "NEG_INDEX", "negative lepton index in pickedLeptons: idx=" + std::to_string(idxA));

        for (std::size_t b = a + 1; b < nLep; ++b) {
            const int idxB = pickedLeptons[b];
            g.require(idxB >= 0, "NEG_INDEX", "negative lepton index in pickedLeptons: idx=" + std::to_string(idxB));

            // Opposite-sign requirement (or reject 0-charge)
            if (lep_charge[idxA] * lep_charge[idxB] >= 0) continue;

            const auto p4A = g.makeP4(lep_pt[idxA], lep_eta[idxA], lep_phi[idxA], lep_mass[idxA], "lepA");
            const auto p4B = g.makeP4(lep_pt[idxB], lep_eta[idxB], lep_phi[idxB], lep_mass[idxB], "lepB");

            const TLorentzVector p4Tag = p4A + p4B;

            const double mass = p4Tag.M();
            const double pt   = p4Tag.Pt();
            const double eta  = std::abs(p4Tag.Eta());
            g.requireFinite(mass, "recoZ.mass");
            g.requireFinite(pt,   "recoZ.pt");

            const double diff = std::abs(mass - massTag_);
            g.requireFinite(diff, "abs(mass-massTag)");

            if (diff > massWindowTag_) continue;
            if (pt   < minPtTag_)      continue;
            if (eta  > maxEtaTag_)      continue;

            if (!foundBest || diff < bestDiff) {
                foundBest = true;
                bestDiff  = diff;
                bestP4    = p4Tag;
            }
        }
    }

    if (foundBest) {
        outTags.push_back(bestP4);
        printDebug("pickRecoZ: candidate mass=" + std::to_string(bestP4.M()) +
                   " pt=" + std::to_string(bestP4.Pt()));
    } else {
        printDebug("pickRecoZ: no valid OS pair in mass window.");
    }

    // Contract: 0 or 1
    g.require(outTags.size() <= 1, "BAD_SIZE", "pickRecoZ must return 0 or 1 TLV");

    return outTags;
}

// ------------------- Reco jets ------------------------------------

std::vector<TLorentzVector> PickZJet::pickRecoJets(
    const SkimTree& skimT,
    const std::vector<int>& pickedLeptons,
    const float* lep_eta,
    const float* lep_phi,
    const int* jet_lepIdx1,
    const int* jet_lepIdx2,
    std::vector<int>& outJetIndices) const
{
    PickGuard g("PickZJet::pickRecoJets");

    printDebug("pickRecoJets: nJet=" + std::to_string(skimT.nJet));

    g.require(skimT.nJet >= 0, "NEG_NJET", "skimT.nJet is negative: " + std::to_string(skimT.nJet));
    g.requireNonNull(lep_eta, "lep_eta");
    g.requireNonNull(lep_phi, "lep_phi");

    //sort jet by pT
    std::vector<int> sortedJetIndices;
    sortedJetIndices.reserve(std::min<int>(skimT.nJet, 64));
    for (int i = 0; i < skimT.nJet; ++i) {
        sortedJetIndices.push_back(i);
    }
    std::sort(sortedJetIndices.begin(), sortedJetIndices.end(),
              [&](int i1, int i2) { return skimT.Jet_pt[i1] > skimT.Jet_pt[i2]; });


    outJetIndices.clear();
    outJetIndices.reserve(2);
    std::vector<TLorentzVector> outJets;
    outJets.reserve(3);
    std::vector<int> candIndices;
    candIndices.reserve(std::min<int>(skimT.nJet, 64));

    for (int i: sortedJetIndices) {
        const double pt  = skimT.Jet_pt[i];
        const double eta = skimT.Jet_eta[i];
        const double phi = skimT.Jet_phi[i];
        const double m   = skimT.Jet_mass[i];

        g.requireFinite(pt,  "Jet_pt");
        g.requireFinite(eta, "Jet_eta");
        g.requireFinite(phi, "Jet_phi");
        g.requireFinite(m,   "Jet_mass");

        printDebug(
            "Jet[" + std::to_string(i) + "]: "
            "pt=" + std::to_string(pt) +
            ", eta=" + std::to_string(eta) +
            ", phi=" + std::to_string(phi)
        );


		if (pt < minPtJet_) {
			printDebug(
				"Jet[" + std::to_string(i) + "] FAIL minPt: "
				"pt=" + std::to_string(pt) +
				" < minPtJet=" + std::to_string(minPtJet_)
			);
			continue;
		}


        // AK4 jets: check lepton cleaning indices if arrays are provided
        if (globalFlags_.getJetAlgo() == GlobalFlag::JetAlgo::AK4Chs ||
            globalFlags_.getJetAlgo() == GlobalFlag::JetAlgo::AK4Puppi)
        {
			if (jet_lepIdx1 && jet_lepIdx1[i] != requiredJet_lepIdx1_) {
				printDebug(
					"Jet[" + std::to_string(i) + "] FAIL lepIdx1: "
					"jet_lepIdx1=" + std::to_string(jet_lepIdx1[i]) +
					" required=" + std::to_string(requiredJet_lepIdx1_)
				);
				continue;
			}

			if (jet_lepIdx2 && jet_lepIdx2[i] != requiredJet_lepIdx2_) {
				printDebug(
					"Jet[" + std::to_string(i) + "] FAIL lepIdx2: "
					"jet_lepIdx2=" + std::to_string(jet_lepIdx2[i]) +
					" required=" + std::to_string(requiredJet_lepIdx2_)
				);
				continue;
			}
        }

        // ΔR to leptons
        double minDr = std::numeric_limits<double>::infinity();
        for (const int idxL : pickedLeptons) {
            g.require(idxL >= 0, "NEG_INDEX", "negative lepton index in pickedLeptons(for dR): " + std::to_string(idxL));

            const double dr = HelperDelta::DELTAR(phi, lep_phi[idxL], eta, lep_eta[idxL]);
            g.requireFinite(dr, "DeltaR(jet,lep)");

            if (dr < minDr) minDr = dr;
            if (minDr < minDeltaRtoLepton_) break;
        }
		if (minDr < minDeltaRtoLepton_) {
			printDebug(
				"Jet[" + std::to_string(i) + "] FAIL ΔR: "
				"minDr=" + std::to_string(minDr) +
				" < minDeltaRtoLepton=" + std::to_string(minDeltaRtoLepton_)
			);
			continue;
		}


        candIndices.push_back(i);
    }

	printDebug("Selected candidate jets (by pt):");

	for (size_t k = 0; k < candIndices.size(); ++k) {
		const int idx = candIndices[k];
		printDebug(
			"  rank " + std::to_string(k) +
			" -> Jet[" + std::to_string(idx) + "] "
			"pt=" + std::to_string(skimT.Jet_pt[idx])
		);
	}

    int iJet1 = -1, iJet2 = -1;
    if (!candIndices.empty())   iJet1 = candIndices[0];
    if (candIndices.size() > 1) iJet2 = candIndices[1];

    printDebug("pickRecoJets: candidates iJet1=" + std::to_string(iJet1) +
               " iJet2=" + std::to_string(iJet2));

    // JetId + |eta| on leading jet (keep your physics choice)
    if (iJet1 != -1 && !pickJet_.passId(skimT, iJet1, jetIdLabel_)) {
        printDebug("pickRecoJets: iJet1 fails JetID");
        iJet1 = -1;
    }

    if (globalFlags_.getJecDerivationLevel() == GlobalFlag::JecDerivationLevel::L3Residual) {
        if (iJet1 != -1 && std::abs(skimT.Jet_eta[iJet1]) > maxEtaProbeJet_) {
            printDebug("pickRecoJets: iJet1 fails |eta| = "+std::to_string(skimT.Jet_eta[iJet1]));
            iJet1 = -1;
        }
    }

    // Keep the method contract: always push exactly 2 entries; -1 allowed
    outJetIndices.push_back(iJet1);
    outJetIndices.push_back(iJet2);

    TLorentzVector p4Jet1(0,0,0,0), p4Jet2(0,0,0,0), p4JetN(0,0,0,0);

    for (int i = 0; i < skimT.nJet; ++i) {
        const auto p4J = g.makeP4(skimT.Jet_pt[i], skimT.Jet_eta[i],
                                 skimT.Jet_phi[i], skimT.Jet_mass[i], "jet");

        if (i == iJet1)      p4Jet1 = p4J;
        else if (i == iJet2) p4Jet2 = p4J;
        else                 p4JetN += p4J;
    }

    outJets.push_back(p4Jet1);
    outJets.push_back(p4Jet2);
    outJets.push_back(p4JetN);

    // Contracts:
    g.requireSize(outJets.size(), 3, "outJets");
    g.requireAllFinite(outJets, "outJets");
    g.requireSize(outJetIndices.size(), 2, "outJetIndices");

    // Indices: allow -1 sentinel; otherwise must be in range
    for (int idx : outJetIndices) {
        if (idx == -1) continue;
        g.requireIndexInRange(idx, skimT.nJet, "outJetIndices");
    }

    return outJets;
}

// ------------------- Gen Z ----------------------------------------

std::vector<TLorentzVector> PickZJet::pickGenZ(
    const SkimTree& skimT,
    const TLorentzVector& p4RecoZ,
    const std::vector<int>& pickedGenLeptons) const
{
    PickGuard g("PickZJet::pickGenZ");

    std::vector<TLorentzVector> out;

    // If reco Z is invalid, do not try matching
    g.requireFinite(p4RecoZ.Pt(),  "p4RecoZ.Pt()");
    g.requireFinite(p4RecoZ.Eta(), "p4RecoZ.Eta()");
    g.requireFinite(p4RecoZ.Phi(), "p4RecoZ.Phi()");

    const std::size_t nLep = pickedGenLeptons.size();
    if (nLep < 2) return out;

    bool found = false;
    TLorentzVector best;
    double bestScore = std::numeric_limits<double>::max();

    for (std::size_t a = 0; a < nLep; ++a) {
        const int ia = pickedGenLeptons[a];
        g.require(ia >= 0, "NEG_INDEX", "negative gen lepton index in pickedGenLeptons: " + std::to_string(ia));

        for (std::size_t b = a + 1; b < nLep; ++b) {
            const int ib = pickedGenLeptons[b];
            g.require(ib >= 0, "NEG_INDEX", "negative gen lepton index in pickedGenLeptons: " + std::to_string(ib));

            // Optional OS check using pdgId sign
            if (skimT.GenDressedLepton_pdgId[ia] * skimT.GenDressedLepton_pdgId[ib] > 0) continue;

            const auto p4A = g.makeP4(skimT.GenDressedLepton_pt[ia],
                                     skimT.GenDressedLepton_eta[ia],
                                     skimT.GenDressedLepton_phi[ia],
                                     skimT.GenDressedLepton_mass[ia], "genLepA");
            const auto p4B = g.makeP4(skimT.GenDressedLepton_pt[ib],
                                     skimT.GenDressedLepton_eta[ib],
                                     skimT.GenDressedLepton_phi[ib],
                                     skimT.GenDressedLepton_mass[ib], "genLepB");

            const TLorentzVector p4Gen = p4A + p4B;
            if(p4Gen.Pt() < minPtGenTag_) continue;

            const double dR = p4Gen.DeltaR(p4RecoZ);
            g.requireFinite(dR, "DeltaR(genZ,recoZ)");
            if (dR > maxDeltaRgenTag_) continue;

            const double score = dR;
            if (!found || score < bestScore) {
                found = true;
                bestScore = score;
                best = p4Gen;
            }
        }
    }

    if (found) out.push_back(best);

    // Contract: 0 or 1
    g.require(out.size() <= 1, "BAD_SIZE", "pickGenZ must return 0 or 1 TLV");
    g.requireAllFinite(out, "outGenZ");

    return out;
}

