#include "ScaleBtagFunction.h"

#include <iostream>
#include <algorithm>
#include <cmath>

#include "ScaleFunctionGuard.hpp"

// ROOT
#include "TH2.h"
#include "TAxis.h"

ScaleBtagFunction::ScaleBtagFunction(const GlobalFlag& globalFlags)
    : loader_(globalFlags),
      globalFlags_(globalFlags),
      isDebug_(globalFlags_.isDebug()),
      isData_(globalFlags_.isData()),
      isMC_(globalFlags_.isMC()) {}

void ScaleBtagFunction::parseSyst(const std::string& syst, std::string& b_syst, std::string& l_syst) {
    b_syst = "central";
    l_syst = "central";
    if      (syst == "b_up")   b_syst = "up";
    else if (syst == "b_down") b_syst = "down";
    else if (syst == "l_up")   l_syst = "up";
    else if (syst == "l_down") l_syst = "down";
}

double ScaleBtagFunction::clampInsideAxis_(const TAxis* ax, double x) {
    const double xmin = ax->GetXmin();
    const double xmax = ax->GetXmax();
    const double hi   = std::nextafter(xmax, xmin);
    return std::clamp(x, xmin, hi);
}

double ScaleBtagFunction::getBinContent2D_(const TH2* h2, double x, double y) {
    const TAxis* axX = h2->GetXaxis();
    const TAxis* axY = h2->GetYaxis();
    const double xx = clampInsideAxis_(axX, x);
    const double yy = clampInsideAxis_(axY, y);
    const int binX = axX->FindBin(xx);
    const int binY = axY->FindBin(yy);
    return h2->GetBinContent(binX, binY);
}

double ScaleBtagFunction::clampInside_(double v, double lo, double hi_inclusive) {
    const double hi = std::nextafter(hi_inclusive, lo);
    return std::clamp(v, lo, hi);
}

double ScaleBtagFunction::getPerJetEff_(int flavor, double pt, double abseta) const {
    // loader guarantees lazy-load
    const TH2* lEff = loader_.lEff();
    const TH2* cEff = loader_.cEff();
    const TH2* bEff = loader_.bEff();

    double e = 0.0;
    if (flavor == 5)      e = getBinContent2D_(bEff, pt, abseta);
    else if (flavor == 4) e = getBinContent2D_(cEff, pt, abseta);
    else                  e = getBinContent2D_(lEff, pt, abseta);

    if (!std::isfinite(e)) e = 0.0;
    if (e < 0.0) e = 0.0;
    if (e > 1.0) e = 1.0;
    return e;
}

double ScaleBtagFunction::getPerJetSF_(int flavor, double pt, double eta,
                                      const std::string& b_syst, const std::string& l_syst) const {
    const std::string& sys = (flavor == 5 || flavor == 4) ? b_syst : l_syst;

    const double aeta_raw = std::abs(eta);
    const double spt_raw  = pt;

    const double aeta = clampInside_(aeta_raw, 0.0, loader_.sfAbsEtaMax());
    const double spt  = clampInside_(spt_raw,  loader_.sfPtMin(), loader_.sfPtMax());

    // Optional clamp notification (useful to catch systematic over-clamping)
    {
        ScaleFunctionGuard guard(globalFlags_, "ScaleBtagFunction::getPerJetSF_");
        guard.noteClamp("|eta|", aeta_raw, aeta, "corrlib absEta range");
        guard.noteClamp("pt",    spt_raw,  spt,  "corrlib pt range");
    }

    try {
        if (flavor == 5 || flavor == 4) {
            return loader_.corrMujets()->evaluate({sys, loader_.wp(), flavor, aeta, spt});
        } else {
            return loader_.corrIncl()->evaluate({sys, loader_.wp(), flavor, aeta, spt});
        }
    } catch (const std::exception& e) {
        const double aeta2 = std::nextafter(aeta, 0.0);
        const double spt2  = std::nextafter(spt,  0.0);
        try {
            if (flavor == 5 || flavor == 4) {
                return loader_.corrMujets()->evaluate({sys, loader_.wp(), flavor, aeta2, spt2});
            } else {
                return loader_.corrIncl()->evaluate({sys, loader_.wp(), flavor, aeta2, spt2});
            }
        } catch (...) {
            if (isDebug_) {
                std::cerr << "[ScaleBtagFunction] SF evaluate failed after clamp/retry: "
                          << "flav=" << flavor << " |eta|=" << aeta << " pt=" << spt
                          << " sys=" << sys << " what=" << e.what() << '\n';
            }
            return 1.0;
        }
    }
}

double ScaleBtagFunction::getEventWeight(const SkimTree& skimT,
                                        const std::vector<int>& jetIdx,
                                        const std::string& syst,
                                        double btagCut) const {
    if (isData_) return 1.0;

    ScaleFunctionGuard guard(globalFlags_, "ScaleBtagFunction::getEventWeight",
                             skimT.run, skimT.luminosityBlock, skimT.event);

    std::string b_syst, l_syst;
    parseSyst(syst, b_syst, l_syst);

    const double defaultCut = (loader_.wpCut() >= 0.0 ? loader_.wpCut() : -1.0);
    const double cut = (btagCut >= 0.0 ? btagCut : defaultCut);

    double pMC = 1.0;
    double pData = 1.0;

    for (int j : jetIdx) {
        guard.checkIndex("Jet", j);

        const double pt     = skimT.Jet_pt[j];
        const double eta    = skimT.Jet_eta[j];
        const int    flav   = std::abs(skimT.Jet_hadronFlavour[j]);
        const double score  = skimT.Jet_btagDeepFlavB[j];

        guard.checkPtEta(pt, eta, "jet_for_btag");
        guard.checkFinite("btagScore", score);

        const double eff = getPerJetEff_(flav, pt, std::abs(eta));
        const double SF  = getPerJetSF_(flav, pt, eta, b_syst, l_syst);

        guard.checkFinite("btagEff", eff);
        guard.checkSf("btagEff", eff, 0.0, 1.0);

        guard.checkFinite("btagSF", SF);
        guard.checkSf("btagSF", SF, 0.0, 5.0);

        const bool tagged = (cut < 0.0) ? (score > -1e9) : (score > cut);

        if (tagged) {
            pMC   *= eff;
            pData *= eff * SF;
        } else {
            pMC   *= (1.0 - eff);
            pData *= (1.0 - eff * SF);
        }

        if (isDebug_) {
            std::cout << "[Btag] j=" << j
                      << " pt=" << pt
                      << " eta=" << eta
                      << " flav=" << flav
                      << " score=" << score
                      << " tagged=" << tagged
                      << " eff=" << eff
                      << " SF=" << SF
                      << " pMC=" << pMC
                      << " pData=" << pData
                      << '\n';
        }
    }

    if (!std::isfinite(pMC) || pMC == 0.0) {
        guard.warn("pMC is non-finite or zero; returning 1.0 to avoid blow-up");
        return 1.0;
    }
    if (!std::isfinite(pData)) {
        guard.warn("pData is non-finite; returning 1.0");
        return 1.0;
    }

    const double w = (pData / pMC);

    guard.checkFinite("btagEventWeight", w);
    guard.checkSf("btagEventWeight", w, 0.0, 10.0);

    return w;
}

