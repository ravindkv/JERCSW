#include "PickGenJet.h"

#include "ReadConfig.h"

#include <cmath>
#include <sstream>
#include <stdexcept>

namespace {
constexpr double kEps = 1e-12;
}

PickGenJet::PickGenJet(const GlobalFlag& globalFlags)
    : globalFlags_(globalFlags),
      isDebug_(globalFlags_.isDebug())
{
    loadConfig_("config/PickGenJet.json");
    validateConfig_();
}

void PickGenJet::printDebug_(const std::string& msg) const {
    if (!isDebug_) return;
    std::cout << "[PickGenJet] " << msg << "\n";
}

void PickGenJet::requireFinite_(double x, const std::string& what) {
    if (!std::isfinite(x)) {
        throw std::runtime_error("PickGenJet - non-finite value for: " + what);
    }
}

TLorentzVector PickGenJet::makeP4_(double pt, double eta, double phi, double mass) {
    TLorentzVector p4;
    p4.SetPtEtaPhiM(pt, eta, phi, mass);
    return p4;
}

void PickGenJet::loadConfig_(const std::string& cfgFile) {
    ReadConfig cfg(cfgFile);

    maxDeltaRgenJet_ = cfg.getValue<double>({"genJetPick", "maxDeltaR"});
    minPtGenJet_     = cfg.getValue<double>({"genJetPick", "minPt"});
    if(globalFlags_.getJetAlgo()==GlobalFlag::JetAlgo::AK8Puppi){
        minPtGenJet_     = cfg.getValue<double>({"genJetPick", "minPtForAK8"});
        maxDeltaRgenJet_ = cfg.getValue<double>({"genJetPick", "maxDeltaRForAK8"});
    }
}

void PickGenJet::validateConfig_() const {
    requireFinite_(maxDeltaRgenJet_, "maxDeltaRgenJet");
    if (!(maxDeltaRgenJet_ > kEps && maxDeltaRgenJet_ < 10.0)) {
        std::ostringstream oss;
        oss << "PickGenJet - invalid maxDeltaRgenJet=" << maxDeltaRgenJet_;
        throw std::runtime_error(oss.str());
    }
}

PickGenJet::Result PickGenJet::matchByRecoIndices(const SkimTree& skimT,
                                                  int iJet1, int iJet2,
                                                  const TLorentzVector& p4Jet1,
                                                  const TLorentzVector& p4Jet2) const
{
    // Distangle from call site: if not MC, do nothing and return default result.
    if (!globalFlags_.isMC()) return Result{};

    const bool want1 = (iJet1 != -1);
    const bool want2 = (iJet2 != -1);
    return matchImpl_(skimT, want1, want2, p4Jet1, p4Jet2);
}

PickGenJet::Result PickGenJet::matchByRecoP4(const SkimTree& skimT,
                                             const TLorentzVector& p4Jet1,
                                             const TLorentzVector& p4Jet2) const
{
    if (!globalFlags_.isMC()) return Result{};

    const bool want1 = (p4Jet1.Pt() > 0.0);
    const bool want2 = (p4Jet2.Pt() > 0.0);
    return matchImpl_(skimT, want1, want2, p4Jet1, p4Jet2);
}

PickGenJet::Result PickGenJet::matchImpl_(const SkimTree& skimT,
                                          bool want1, bool want2,
                                          const TLorentzVector& p4Jet1,
                                          const TLorentzVector& p4Jet2) const
{
    if (skimT.nGenJet < 0) {
        throw std::runtime_error("PickGenJet::match - skimT.nGenJet is negative");
    }

    printDebug_("match: nGenJet=" + std::to_string(skimT.nGenJet) +
                " want1=" + std::to_string(want1) +
                " want2=" + std::to_string(want2));

    if (want1) {
        requireFinite_(p4Jet1.Pt(),  "p4Jet1.Pt(genMatch)");
        requireFinite_(p4Jet1.Eta(), "p4Jet1.Eta(genMatch)");
        requireFinite_(p4Jet1.Phi(), "p4Jet1.Phi(genMatch)");
    }
    if (want2) {
        requireFinite_(p4Jet2.Pt(),  "p4Jet2.Pt(genMatch)");
        requireFinite_(p4Jet2.Eta(), "p4Jet2.Eta(genMatch)");
        requireFinite_(p4Jet2.Phi(), "p4Jet2.Phi(genMatch)");
    }

    Result res;

    // -------------------------
    // 1) Find closest genJet for jet1
    // -------------------------
    if (want1) {
        double bestDR1 = 1e9;
        int bestIdx1   = -1;
        TLorentzVector bestP4_1;

        for (int i = 0; i < skimT.nGenJet; ++i) {
            if(skimT.GenJet_pt[i] < minPtGenJet_) continue;
            const TLorentzVector p4Gen = makeP4_(skimT.GenJet_pt[i],
                                                 skimT.GenJet_eta[i],
                                                 skimT.GenJet_phi[i],
                                                 skimT.GenJet_mass[i]);

            const double dR1 = p4Gen.DeltaR(p4Jet1);
            requireFinite_(dR1, "DeltaR(genJet,recoJet1)");
            if (dR1 < maxDeltaRgenJet_ && dR1 < bestDR1 ) {
                bestDR1  = dR1;
                bestIdx1 = i;
                bestP4_1 = p4Gen;
            }
        }

        if (bestIdx1 != -1) {
            res.iGenJet1  = bestIdx1;
            res.p4GenJet1 = bestP4_1;
        }
    }

    // -------------------------
    // 2) Find closest genJet for jet2 among the remaining
    // -------------------------
    if (want2) {
        double bestDR2 = 1e9;
        int bestIdx2   = -1;
        TLorentzVector bestP4_2;

        for (int i = 0; i < skimT.nGenJet; ++i) {
            if (i == res.iGenJet1) continue; // enforce uniqueness
            if(skimT.GenJet_pt[i] < minPtGenJet_) continue;

            const TLorentzVector p4Gen = makeP4_(skimT.GenJet_pt[i],
                                                 skimT.GenJet_eta[i],
                                                 skimT.GenJet_phi[i],
                                                 skimT.GenJet_mass[i]);

            const double dR2 = p4Gen.DeltaR(p4Jet2);
            requireFinite_(dR2, "DeltaR(genJet,recoJet2)");
            if (dR2 < maxDeltaRgenJet_ && dR2 < bestDR2 && p4Gen.Pt() > minPtGenJet_) {
                bestDR2  = dR2;
                bestIdx2 = i;
                bestP4_2 = p4Gen;
            }
        }

        if (bestIdx2 != -1) {
            res.iGenJet2  = bestIdx2;
            res.p4GenJet2 = bestP4_2;
        }
    }

    printDebug_("match: iGenJet1=" + std::to_string(res.iGenJet1) +
                " iGenJet2=" + std::to_string(res.iGenJet2));

    return res;
}


// General match for single jet 
TLorentzVector PickGenJet:: matchedP4GenJet(const SkimTree& skimT,
                              const TLorentzVector& p4Jet) const
{
    if (skimT.nGenJet < 0) {
        throw std::runtime_error("PickGenJet::match - skimT.nGenJet is negative");
    }

    requireFinite_(p4Jet.Pt(),  "p4Jet.Pt(genMatch)");
    requireFinite_(p4Jet.Eta(), "p4Jet.Eta(genMatch)");
    requireFinite_(p4Jet.Phi(), "p4Jet.Phi(genMatch)");

    // -------------------------
    // 1) Find closest genJet for jet
    // -------------------------
    double bestDR = 1e9;
    int bestIdx   = -1;
    TLorentzVector bestP4_{0,0,0,0};
    for (int i = 0; i < skimT.nGenJet; ++i) {
        if(skimT.GenJet_pt[i] < minPtGenJet_) continue;
        const TLorentzVector p4Gen = makeP4_(skimT.GenJet_pt[i],
                                             skimT.GenJet_eta[i],
                                             skimT.GenJet_phi[i],
                                             skimT.GenJet_mass[i]);

        const double dR = p4Gen.DeltaR(p4Jet);
        requireFinite_(dR, "DeltaR(genJet,recoJet)");
        if (dR < maxDeltaRgenJet_ && dR < bestDR ) {
            bestDR  = dR;
            bestIdx = i;
            bestP4_ = p4Gen;
        }
    }
    
    return bestP4_;
}
