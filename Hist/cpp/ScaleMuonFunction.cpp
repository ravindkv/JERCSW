#include "ScaleMuonFunction.h"

#include <iostream>
#include <algorithm>
#include <cmath>

#include "HelperDelta.hpp"
#include "ScaleFunctionGuard.hpp"

// ROOT
#include "TH2.h"
#include "TAxis.h"

ScaleMuonFunction::ScaleMuonFunction(const GlobalFlag& globalFlags)
    : loader_(globalFlags),
      globalFlags_(globalFlags),
      isDebug_(globalFlags_.isDebug()),
      isData_(globalFlags_.isData()),
      isMC_(globalFlags_.isMC()) {}

// ---------------- Rochester ----------------
double ScaleMuonFunction::getMuonRochCorrection(const SkimTree& skimT,
                                               int index,
                                               const std::string& syst) const {
    (void)syst; // placeholder for future up/down

    ScaleFunctionGuard guard(globalFlags_, "ScaleMuonFunction::getMuonRochCorrection",
                             skimT.run, skimT.luminosityBlock, skimT.event);

    guard.checkIndex("Muon", index);

    double corrMuRoch = 1.0;
    const double Q   = skimT.Muon_charge[index];
    const double pt  = skimT.Muon_pt[index];
    const double eta = skimT.Muon_eta[index];
    const double phi = skimT.Muon_phi[index];

    guard.checkFinite("Muon_charge", Q);
    guard.checkPtEtaPhi(pt, eta, phi, "muon");

    double s = 0.0;
    double m = 0.0;
    double genPt = -9999.;
    double u = -9999.;
    int nl = -9999;
    bool isMatched = false;

    if (isData_) {
        corrMuRoch = loader_.roch().kScaleDT(Q, pt, eta, phi, s, m);
    }

    if (isMC_) {
        for (int i = 0; i < skimT.nGenDressedLepton; ++i) {
            if (std::abs(skimT.GenDressedLepton_pdgId[i]) == 13) {
                const double delR =
                    HelperDelta::DELTAR(phi, skimT.GenDressedLepton_phi[i],
                                        eta, skimT.GenDressedLepton_eta[i]);
                if (delR < 0.2) {
                    genPt = skimT.GenDressedLepton_pt[i];
                    isMatched = true;
                    break;
                }
            }
        }

        if (isMatched) {
            corrMuRoch = loader_.roch().kSpreadMC(Q, pt, eta, phi, genPt, s, m);
        } else {
            // deterministic event-by-event
            loader_.rng().SetSeed(skimT.event + skimT.run + skimT.luminosityBlock);
            u  = loader_.rng().Uniform(0.0, 1.0);
            nl = skimT.Muon_nTrackerLayers[index];
            corrMuRoch = loader_.roch().kSmearMC(Q, pt, eta, phi, nl, u, s, m);
        }
    }

    // Guard checks (keep debug as-is)
    guard.checkFinite("corrMuRoch", corrMuRoch);
    guard.checkSf("corrMuRoch", corrMuRoch, 0.5, 1.5);

    if (isDebug_) {
        std::cout << "[MuRoch] idx=" << index
                  << ", Q=" << Q
                  << ", pt=" << pt
                  << ", eta=" << eta
                  << ", phi=" << phi
                  << ", genPt=" << genPt
                  << ", nl=" << nl
                  << ", u=" << u
                  << ", s=" << s
                  << ", m=" << m
                  << ", isMatched=" << isMatched
                  << ", corr=" << corrMuRoch
                  << '\n';
    }
    return corrMuRoch;
}

// ---------------- SF helpers ----------------
ScaleMuonFunction::SystLevel ScaleMuonFunction::parseSyst(const std::string& s) {
    if (s == "down" || s == "Down" || s == "DOWN") return SystLevel::Down;
    if (s == "up"   || s == "Up"   || s == "UP")   return SystLevel::Up;
    return SystLevel::Nominal;
}

double ScaleMuonFunction::getSfFromHist(const TH2* h2, double pt, double eta, SystLevel syst)const {
    ScaleFunctionGuard guard(globalFlags_, "ScaleMuonFunction::getSfFromHist");

    if (!h2) {
        guard.noteDefaultedToOne("MuonSF", "missing TH2");
        return 1.0;
    }

    guard.checkPtEta(pt, eta, "muon_sf");

    const TAxis* x = h2->GetXaxis(); // abs η
    const TAxis* y = h2->GetYaxis(); // pT (GeV)

    const double xVal = std::clamp(eta, x->GetXmin() + 1e-6, x->GetXmax() - 1e-6);
    const double yVal = std::clamp(pt,  y->GetXmin() + 1e-6, y->GetXmax() - 1e-6);

    guard.noteClamp("MuonSF_absEta", eta, xVal, "hist x-axis clamp");
    guard.noteClamp("MuonSF_pt",     pt,  yVal, "hist y-axis clamp");

    const int binX = x->FindBin(xVal);
    const int binY = y->FindBin(yVal);

    const double sf  = h2->GetBinContent(binX, binY);
    const double err = h2->GetBinError  (binX, binY);

    if (sf == 0.0 && err == 0.0) {
        guard.noteDefaultedToOne("MuonSF", "bin content and error are 0");
        return 1.0;
    }

    const int lvl = static_cast<int>(syst); // Down=0, Nominal=1, Up=2
    const double out = sf + (lvl - 1) * err;

    guard.checkFinite("MuonSF", out);
    guard.checkSf("MuonSF", out, 0.0, 5.0);

    return out;
}

// ---------------- Public SF getters ----------------
double ScaleMuonFunction::getMuonIdSf(double pt, double eta, SystLevel syst) const {
    return getSfFromHist(loader_.idHist(), pt, eta, syst);
}
double ScaleMuonFunction::getMuonIsoSf(double pt, double eta, SystLevel syst) const {
    return getSfFromHist(loader_.isoHist(), pt, eta, syst);
}
double ScaleMuonFunction::getMuonTrigSf(double pt, double eta, SystLevel syst) const {
    return getSfFromHist(loader_.trigHist(), pt, eta, syst);
}

ScaleMuonFunction::MuSf ScaleMuonFunction::getMuonSfs(const SkimTree& skimT,
                                                      int index,
                                                      SystLevel syst) const {
    ScaleFunctionGuard guard(globalFlags_, "ScaleMuonFunction::getMuonSfs",
                             skimT.run, skimT.luminosityBlock, skimT.event);

    guard.checkIndex("Muon", index);

    const double pt  = skimT.Muon_pt[index];
    const double eta = std::abs(skimT.Muon_eta[index]);

    guard.checkPtEta(pt, eta, "muon");

    MuSf out;
    out.id   = getMuonIdSf  (pt, eta, syst);
    out.iso  = getMuonIsoSf (pt, eta, syst);
    out.trig = getMuonTrigSf(pt, eta, syst);
    out.total = out.id * out.iso * out.trig;

    guard.checkFinite("MuonSF_total", out.total);
    guard.checkSf("MuonSF_total", out.total, 0.0, 5.0);

    if (isDebug_) {
        std::cout << "----------------------------\n"
                  << "Muon Scale Factors\n"
                  << "    pt   = " << pt  << "\n"
                  << "    eta  = " << eta << "\n"
                  << "    ID   = " << out.id   << "\n"
                  << "    Iso  = " << out.iso  << "\n"
                  << "    Trig = " << out.trig << "\n"
                  << "    Total= " << out.total << "\n";
    }
    return out;
}

ScaleMuonFunction::MuSf ScaleMuonFunction::getMuonSfs(const SkimTree& skimT,
                                                      int index,
                                                      const std::string& systStr) const {
    return getMuonSfs(skimT, index, parseSyst(systStr));
}

