#include "ScalePhotonFunction.h"

#include <iostream>
#include <algorithm>
#include <cmath>

#include "ScaleFunctionGuard.hpp"

// ROOT
#include "TH2.h"
#include "TH1F.h"
#include "TAxis.h"

ScalePhotonFunction::ScalePhotonFunction(const GlobalFlag& globalFlags)
    : loader_(globalFlags),
      globalFlags_(globalFlags),
      isDebug_(globalFlags_.isDebug()),
      isData_(globalFlags_.isData()),
      isMC_(globalFlags_.isMC()) {}

// ---------------- correctionlib (scale/smear) ----------------
double ScalePhotonFunction::getPhotonScaleCorrection(const SkimTree& skimT,
                                                    const std::string& nomOrSyst,
                                                    int index) const {
    double phoScaleSf = 1.0;
    try {
        phoScaleSf = loader_.phoSsRef()->evaluate({
            nomOrSyst,
            skimT.Photon_seedGain[index],
            static_cast<Float_t>(skimT.run),
            skimT.Photon_eta[index],
            skimT.Photon_r9[index],
            skimT.Photon_pt[index]
        });

        ScaleFunctionGuard guard(globalFlags_, "ScalePhotonFunction::getPhotonScaleCorrection",
                                 skimT.run, skimT.luminosityBlock, skimT.event);

        guard.checkIndex("Photon", index);
        guard.checkPtEta(skimT.Photon_pt[index], skimT.Photon_eta[index], "photon");
        guard.checkFinite("Photon_r9", skimT.Photon_r9[index]);
        guard.checkFinite("phoScaleSf", phoScaleSf);
        guard.checkSf("phoScaleSf", phoScaleSf, 0.5, 1.5);

        if (isDebug_) {
            std::cout << "[PhotonScale] "
                      << "nomOrSyst=" << nomOrSyst
                      << ", seedGain=" << skimT.Photon_seedGain[index]
                      << ", run=" << skimT.run
                      << ", eta=" << skimT.Photon_eta[index]
                      << ", r9=" << skimT.Photon_r9[index]
                      << ", pt=" << skimT.Photon_pt[index]
                      << ", sf=" << phoScaleSf
                      << '\n';
        }
    } catch (const std::exception& e) {
        std::cout << "\nEXCEPTION: in ScalePhotonFunction::getPhotonScaleCorrection(): "
                  << e.what() << '\n';
        throw std::runtime_error("Error calculating Photon Scale Correction.");
    }
    return phoScaleSf;
}

double ScalePhotonFunction::getPhotonSmearCorrection(const SkimTree& skimT,
                                                    const std::string& nomOrSyst,
                                                    int index) const {
    double phoSmearSf = 1.0;
    try {
        phoSmearSf = loader_.phoSsRef()->evaluate({
            nomOrSyst,
            skimT.Photon_eta[index],
            skimT.Photon_r9[index]
        });

        ScaleFunctionGuard guard(globalFlags_, "ScalePhotonFunction::getPhotonSmearCorrection",
                                 skimT.run, skimT.luminosityBlock, skimT.event);

        guard.checkIndex("Photon", index);
        guard.checkFinite("phoSmearSf", phoSmearSf);
        guard.checkSf("phoSmearSf", phoSmearSf, 0.7, 1.3);

        if (isDebug_) {
            std::cout << "[PhotonSmear] "
                      << "nomOrSyst=" << nomOrSyst
                      << ", eta=" << skimT.Photon_eta[index]
                      << ", r9=" << skimT.Photon_r9[index]
                      << ", sf=" << phoSmearSf
                      << '\n';
        }
    } catch (const std::exception& e) {
        std::cout << "\nEXCEPTION: in ScalePhotonFunction::getPhotonSmearCorrection(): "
                  << e.what() << '\n';
        throw std::runtime_error("Error calculating Photon Smear Correction.");
    }
    return phoSmearSf;
}

// ---------------- SF helpers ----------------
ScalePhotonFunction::SystLevel ScalePhotonFunction::parseSyst(const std::string& s) {
    if (s == "down" || s == "Down" || s == "DOWN") return SystLevel::Down;
    if (s == "up"   || s == "Up"   || s == "UP")   return SystLevel::Up;
    return SystLevel::Nominal;
}

double ScalePhotonFunction::getSfFromHist2D(const TH2* h2, double pt, double eta, SystLevel syst) const {
    ScaleFunctionGuard guard(globalFlags_, "ScalePhotonFunction::getSfFromHist2D");

    if (!h2) {
        guard.noteDefaultedToOne("PhotonSF2D", "missing TH2");
        return 1.0;
    }

    guard.checkPtEta(pt, eta, "photon_sf2d");

    const TAxis* x = h2->GetXaxis(); // SuperCluster η (signed)
    const TAxis* y = h2->GetYaxis(); // pT (GeV)

    const double xVal = std::clamp(eta, x->GetXmin() + 1e-6, x->GetXmax() - 1e-6);
    const double yVal = std::clamp(pt,  y->GetXmin() + 1e-6, y->GetXmax() - 1e-6);

    guard.noteClamp("PhotonSF2D_eta", eta, xVal, "hist x-axis clamp");
    guard.noteClamp("PhotonSF2D_pt",  pt,  yVal, "hist y-axis clamp");

    const int binX = x->FindBin(xVal);
    const int binY = y->FindBin(yVal);

    const double sf  = h2->GetBinContent(binX, binY);
    const double err = h2->GetBinError  (binX, binY);

    if (sf == 0.0 && err == 0.0) {
        guard.noteDefaultedToOne("PhotonSF2D", "bin content and error are 0");
        return 1.0;
    }

    const int lvl = static_cast<int>(syst); // Down=0, Nominal=1, Up=2
    const double out = sf + (lvl - 1) * err;

    guard.checkFinite("PhotonSF2D", out);
    guard.checkSf("PhotonSF2D", out, 0.0, 5.0);

    return out;
}

// 1D: choose bin by |eta| (as in ReaderPhoton: <1.5 -> bin 1, else bin 4)
double ScalePhotonFunction::getSfFromHist1D(const TH1F* h1, double /*pt*/, double eta, SystLevel syst) const {
    ScaleFunctionGuard guard(globalFlags_, "ScalePhotonFunction::getSfFromHist1D");

    if (!h1) {
        guard.noteDefaultedToOne("PhotonSF1D", "missing TH1");
        return 1.0;
    }

    guard.checkFinite("eta", eta);

    const int bin = (std::abs(eta) < 1.5) ? 1 : 4;
    const double sf  = h1->GetBinContent(bin);
    const double err = h1->GetBinError(bin);
    const int lvl = static_cast<int>(syst);
    const double out = sf + (lvl - 1) * err;

    guard.checkFinite("PhotonSF1D", out);
    guard.checkSf("PhotonSF1D", out, 0.0, 5.0);

    return out;
}

// ---------------- Public SF getters ----------------
double ScalePhotonFunction::getPhotonIdSf(double pt, double eta, SystLevel syst) const {
    return getSfFromHist2D(loader_.idHist(), pt, eta, syst);
}
double ScalePhotonFunction::getPhotonPsSf(double pt, double eta, SystLevel syst) const {
    return getSfFromHist1D(loader_.psHist(), pt, eta, syst);
}
double ScalePhotonFunction::getPhotonCsSf(double pt, double eta, SystLevel syst) const {
    return getSfFromHist1D(loader_.csHist(), pt, eta, syst);
}

ScalePhotonFunction::PhotonSf ScalePhotonFunction::getPhotonSfs(const SkimTree& skimT,
                                                                int index,
                                                                SystLevel syst) const {
    ScaleFunctionGuard guard(globalFlags_, "ScalePhotonFunction::getPhotonSfs",
                             skimT.run, skimT.luminosityBlock, skimT.event);

    guard.checkIndex("Photon", index);

    const double pt  = skimT.Photon_pt[index];
    const double eta = skimT.Photon_eta[index];

    guard.checkPtEta(pt, eta, "photon");

    PhotonSf out;
    out.id = getPhotonIdSf(pt, eta, syst);
    out.ps = getPhotonPsSf(pt, eta, syst);
    out.cs = getPhotonCsSf(pt, eta, syst);
    out.total = out.id * out.ps * out.cs;

    guard.checkFinite("PhotonSF_total", out.total);
    guard.checkSf("PhotonSF_total", out.total, 0.0, 5.0);

    if (isDebug_) {
        std::cout << "----------------------------\n"
                  << "Photon Scale Factors\n"
                  << "    pt   = " << pt  << "\n"
                  << "    eta  = " << eta << "\n"
                  << "    ID   = " << out.id << "\n"
                  << "    PS   = " << out.ps << "\n"
                  << "    CS   = " << out.cs << "\n"
                  << "    Total= " << out.total << "\n";
    }
    return out;
}

ScalePhotonFunction::PhotonSf ScalePhotonFunction::getPhotonSfs(const SkimTree& skimT,
                                                                int index,
                                                                const std::string& systStr) const {
    return getPhotonSfs(skimT, index, parseSyst(systStr));
}

