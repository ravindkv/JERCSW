#include "PickDiJet.h"

#include "PickGuard.hpp"
#include "ReadConfig.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <random>
#include <stdexcept>

namespace {
constexpr double kEps = 1e-12;

// A small hash combiner for deterministic seeding
inline std::uint64_t hashCombine(std::uint64_t h, std::uint64_t v) {
    // 64-bit mix (splitmix-like)
    v += 0x9e3779b97f4a7c15ULL;
    v = (v ^ (v >> 30)) * 0xbf58476d1ce4e5b9ULL;
    v = (v ^ (v >> 27)) * 0x94d049bb133111ebULL;
    v = v ^ (v >> 31);
    return h ^ v;
}
} // namespace

PickDiJet::PickDiJet(const GlobalFlag& flags)
    : globalFlags_(flags)
    , pickJet_(globalFlags_)
    , isDebug_(flags.isDebug())
{
    loadConfig("config/PickDiJet.json");
    validateConfig_();
}

PickDiJet::~PickDiJet() = default;

// Load configuration from JSON file and store values in private members
void PickDiJet::loadConfig(const std::string& filename) {
    ReadConfig config(filename);

    minPtTnP_   = config.getValue<double>({"diJetPick","minPtTnP"});
    if(globalFlags_.getJetAlgo()==GlobalFlag::JetAlgo::AK8Puppi){
        minPtTnP_   = config.getValue<double>({"diJetPick","minPtTnPForAK8"});
    }
    jetIdLabel_ = config.getValue<std::string>({"diJetPick","jetIdLabel"});
    minPtOther_ = config.getValue<double>({"diJetPick","minPtOther"});
    maxEtaTag_  = config.getValue<double>({"diJetPick","maxEtaTag"});
    useDeterministicSwap_ = config.getValue<bool>({"diJetPick","useDeterministicSwap"});
}

void PickDiJet::validateConfig_() const {
    if (!std::isfinite(minPtTnP_) || minPtTnP_ <= 0.0) {
        throw std::runtime_error("PickDiJet config invalid: minPtTnP must be > 0");
    }
    if (!std::isfinite(minPtOther_) || minPtOther_ < 0.0) {
        throw std::runtime_error("PickDiJet config invalid: minPtOther must be >= 0");
    }
    if (!std::isfinite(maxEtaTag_) || maxEtaTag_ <= 0.0) {
        throw std::runtime_error("PickDiJet config invalid: maxEtaTag must be > 0");
    }
    if (jetIdLabel_.empty()) {
        throw std::runtime_error("PickDiJet config invalid: jetIdLabel is empty");
    }
}

void PickDiJet::printDebug(const std::string& msg) const {
    if (isDebug_) std::cout << "[PickDiJet] " << msg << "\n";
}

void PickDiJet::resetOutputs_() {
    indexTag_   = -1;
    indexProbe_ = -1;
    p4Tag_.SetPtEtaPhiM(0,0,0,0);
    p4Probe_.SetPtEtaPhiM(0,0,0,0);
    p4SumOther_.SetPtEtaPhiM(0,0,0,0);
}

void PickDiJet::maybeSwapLeadingTwo_(std::vector<int>& good, const SkimTree& skimT) const {
    if (good.size() < 2) return;

    // For truly *random*, keep std::shuffle with static RNG.
    // For robust & reproducible physics production, deterministic is better.
    if (!useDeterministicSwap_) {
        static thread_local std::mt19937 rng{std::random_device{}()};
        std::shuffle(good.begin(), good.begin() + 2, rng);
        return;
    }

    // Deterministic swap based on event identity if available.
    // SkimTree typically has run/lumi/event; adjust names if yours differ.
    std::uint64_t seed = 0x123456789abcdefULL;

    seed = hashCombine(seed, static_cast<std::uint64_t>(skimT.run));
    seed = hashCombine(seed, static_cast<std::uint64_t>(skimT.luminosityBlock));
    seed = hashCombine(seed, static_cast<std::uint64_t>(skimT.event));

    // 50/50 swap decision:
    if ((seed & 1ULL) == 1ULL) std::swap(good[0], good[1]);
}

void PickDiJet::pickJets(const SkimTree& skimT) {
    PickGuard g("PickDiJet::pickJets");

    resetOutputs_();

    g.require(skimT.nJet >= 0, "NEG_NJET", "skimT.nJet is negative: " + std::to_string(skimT.nJet));
    printDebug("Starting pickJets() with nJet=" + std::to_string(skimT.nJet));

    // 1) collect all jets passing pT & Id
    std::vector<int> good;
    good.reserve(std::min<int>(skimT.nJet, 32));

    for (int i = 0; i < skimT.nJet; ++i) {
        const double pt  = skimT.Jet_pt[i];
        if (pt < minPtTnP_) continue;
        if (!pickJet_.passId(skimT, i, jetIdLabel_)) continue;
        const double eta = skimT.Jet_eta[i];
        const double phi = skimT.Jet_phi[i];
        const double m   = skimT.Jet_mass[i];

        // If any of these are non-finite, that's a red flag worth surfacing.
        g.requireFinite(pt,  "Jet_pt");
        g.requireFinite(eta, "Jet_eta");
        g.requireFinite(phi, "Jet_phi");
        g.requireFinite(m,   "Jet_mass");

        good.push_back(i);
    }

    if (good.size() < 2) {
        printDebug("→ fewer than two jets pass pT/id → event rejected");
        return;
    }

    // 2a) sort by descending pT
    std::sort(good.begin(), good.end(),
              [&skimT](int a, int b) { return skimT.Jet_pt[a] > skimT.Jet_pt[b]; });

    // 2b) (robust) swap leading two in a reproducible way
    maybeSwapLeadingTwo_(good, skimT);

    // 3) take first two as Tag & Probe
    indexTag_   = good[0];
    indexProbe_ = good[1];

    g.requireIndexInRange(indexTag_,   skimT.nJet, "tag");
    g.requireIndexInRange(indexProbe_, skimT.nJet, "probe");

    // fill their four-vectors (via PickGuard makeP4)
    p4Tag_ = g.makeP4(skimT.Jet_pt[indexTag_],
                      skimT.Jet_eta[indexTag_],
                      skimT.Jet_phi[indexTag_],
                      skimT.Jet_mass[indexTag_],
                      "tagJet");

    p4Probe_ = g.makeP4(skimT.Jet_pt[indexProbe_],
                        skimT.Jet_eta[indexProbe_],
                        skimT.Jet_phi[indexProbe_],
                        skimT.Jet_mass[indexProbe_],
                        "probeJet");

    if (std::abs(p4Tag_.Eta()) > maxEtaTag_) {
        printDebug("→ tag |η| too large → reject (|eta|=" + std::to_string(std::abs(p4Tag_.Eta())) + ")");
        resetOutputs_();
        return;
    }

    printDebug("→ Tag idx=" + std::to_string(indexTag_) +
               ", Probe idx=" + std::to_string(indexProbe_) +
               ", Tag pt=" + std::to_string(p4Tag_.Pt()) +
               ", Probe pt=" + std::to_string(p4Probe_.Pt()));

    // 4) sum all other jets above minPtOther_
    for (int i = 0; i < skimT.nJet; ++i) {
        if (i == indexTag_ || i == indexProbe_) continue;

        const double pt  = skimT.Jet_pt[i];
        const double eta = skimT.Jet_eta[i];
        const double phi = skimT.Jet_phi[i];
        const double m   = skimT.Jet_mass[i];

        g.requireFinite(pt,  "Jet_pt(other)");
        g.requireFinite(eta, "Jet_eta(other)");
        g.requireFinite(phi, "Jet_phi(other)");
        g.requireFinite(m,   "Jet_mass(other)");

        if (pt < minPtOther_) continue;

        const TLorentzVector p4 = g.makeP4(pt, eta, phi, m, "otherJet");
        p4SumOther_ += p4;
    }

    // sanity on sum
    g.requireFinite(p4SumOther_.Pt(),  "p4SumOther.Pt()");
    g.requireFinite(p4SumOther_.Phi(), "p4SumOther.Phi()");

    // Contract sanity (optional but useful): if one index is set, both should be
    g.require((indexTag_ >= 0) == (indexProbe_ >= 0),
              "INCONSISTENT_STATE",
              "indexTag/indexProbe inconsistency after pickJets");

    g.require(hasTagProbe() ? (p4Tag_.Pt() > 0 && p4Probe_.Pt() > 0) : true,
          "BAD_OUTPUTS", "hasTagProbe but tag/probe pT not > 0");

    printDebug("→ summed other jets with pT > " + std::to_string(minPtOther_) +
               " : sumOtherPt=" + std::to_string(p4SumOther_.Pt()));
}

