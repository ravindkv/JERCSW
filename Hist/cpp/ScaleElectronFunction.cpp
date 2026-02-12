#include "ScaleElectronFunction.h"

#include <iostream>
#include <cmath>
#include <algorithm>

#include "ScaleFunctionGuard.hpp"

// ROOT
#include "TH2.h"
#include "TAxis.h"

ScaleElectronFunction::ScaleElectronFunction(const GlobalFlag& globalFlags)
    : loader_(globalFlags),
      globalFlags_(globalFlags),
      isDebug_(globalFlags_.isDebug()),
      isData_(globalFlags_.isData()),
      isMC_(globalFlags_.isMC()) {}

// ---------------- Ss correction (same logic as before) ----------------
double ScaleElectronFunction::getElectronSsCorrection(const SkimTree& skimT,
                                                     int index,
                                                     const std::string& /*syst*/) const {
    ScaleFunctionGuard guard(globalFlags_, "ScaleElectronFunction::getElectronSsCorrection",
                             skimT.run, skimT.luminosityBlock, skimT.event);

    guard.checkIndex("Electron", index);

    double corrEleSs = 1.0;

    const double pt    = skimT.Electron_pt[index];
    const double eta   = skimT.Electron_eta[index];
    const double scEta = eta + skimT.Electron_deltaEtaSC[index];
    const double mass  = skimT.Electron_mass[index];
    const double seedG = skimT.Electron_seedGain[index];
    double eSsCorr       = 1.0; 

    guard.checkPtEta(pt, eta, "electron");
    guard.checkFinite("scEta", scEta);
    guard.checkFinite("Electron_mass", mass);

    const double energy = std::sqrt(std::pow(pt * std::cosh(scEta), 2) + std::pow(mass, 2));
    if(globalFlags_.getNanoVersion()== GlobalFlag::NanoVersion::V15){
        //const double r9    = skimT.Electron_r9[index]; //FIXME
        //eSsCorr = loadedEleSsRef_->evaluate({pt, r9, scEta});
        eSsCorr = loader_.eleSsRef()->evaluate({seedG, scEta});
    }else{
        eSsCorr = skimT.Electron_eCorr[index];
    }
    guard.checkFinite("eSsCorr", eSsCorr);
    double corrE = eSsCorr * energy;
    const double newPt  = std::sqrt((std::pow(corrE, 2) - std::pow(mass, 2)) / std::pow(std::cosh(scEta), 2));
    corrEleSs           = (pt > 0.0) ? (newPt / pt) : 1.0;

    guard.checkFinite("energy", energy);
    guard.checkFinite("newPt", newPt);
    guard.checkFinite("corrEleSs", corrEleSs);
    guard.checkSf("corrEleSs", corrEleSs, 0.5, 1.5);

    if (isDebug_) {
        std::cout << "[EleSs] idx=" << index
                  << ", pt=" << pt
                  << ", scEta=" << scEta
                  << ", E=" << energy
                  << ", eSsCorr=" << eSsCorr 
                  << ", newPt=" << newPt
                  << ", corr=" << corrEleSs
                  << '\n';
    }
    return corrEleSs;
}

// ---------------- Helpers ----------------
ScaleElectronFunction::SystLevel ScaleElectronFunction::parseSyst(const std::string& s) {
    if (s == "down" || s == "Down" || s == "DOWN") return SystLevel::Down;
    if (s == "up"   || s == "Up"   || s == "UP")   return SystLevel::Up;
    return SystLevel::Nominal;
}

double ScaleElectronFunction::getSfFromHist(const TH2* h2, double pt, double eta, SystLevel syst) const {
    ScaleFunctionGuard guard(globalFlags_, "ScaleElectronFunction::getSfFromHist");

    if (!h2) {
        guard.noteDefaultedToOne("ElectronSF", "missing TH2");
        return 1.0;
    }

    guard.checkPtEta(pt, eta, "electron_sf");

    const TAxis* x = h2->GetXaxis(); // SuperCluster η (signed)
    const TAxis* y = h2->GetYaxis(); // pT (GeV)

    const double xVal = std::clamp(eta, x->GetXmin() + 1e-6, x->GetXmax() - 1e-6);
    const double yVal = std::clamp(pt,  y->GetXmin() + 1e-6, y->GetXmax() - 1e-6);

    guard.noteClamp("ElectronSF_scEta", eta, xVal, "hist x-axis clamp");
    guard.noteClamp("ElectronSF_pt",    pt,  yVal, "hist y-axis clamp");

    const int binX = x->FindBin(xVal);
    const int binY = y->FindBin(yVal);

    const double sf  = h2->GetBinContent(binX, binY);
    const double err = h2->GetBinError  (binX, binY);

    if (sf == 0.0 && err == 0.0) {
        guard.noteDefaultedToOne("ElectronSF", "bin content and error are 0");
        return 1.0;
    }

    const int lvl = static_cast<int>(syst); // Down=0, Nominal=1, Up=2
    const double out = sf + (lvl - 1) * err;

    guard.checkFinite("ElectronSF", out);
    guard.checkSf("ElectronSF", out, 0.0, 5.0);

    return out;
}

// ---------------- Public SF getters ----------------
double ScaleElectronFunction::getElectronIdSf(double pt, double eta, SystLevel syst) const {
    return getSfFromHist(loader_.idHist(), pt, eta, syst);
}
double ScaleElectronFunction::getElectronRecoSf(double pt, double eta, SystLevel syst) const {
    return getSfFromHist(loader_.recoHist(), pt, eta, syst);
}
double ScaleElectronFunction::getElectronTrigSf(double pt, double eta, SystLevel syst) const {
    return getSfFromHist(loader_.trigHist(), pt, eta, syst);
}

ScaleElectronFunction::EleSf ScaleElectronFunction::getElectronSfs(const SkimTree& skimT,
                                                                   int index,
                                                                   SystLevel syst) const {
    ScaleFunctionGuard guard(globalFlags_, "ScaleElectronFunction::getElectronSfs",
                             skimT.run, skimT.luminosityBlock, skimT.event);

    guard.checkIndex("Electron", index);

    const double pt    = skimT.Electron_pt[index];
    const double eta   = skimT.Electron_eta[index];
    const double scEta = eta + skimT.Electron_deltaEtaSC[index];

    guard.checkPtEta(pt, eta, "electron");
    guard.checkFinite("scEta", scEta);

    EleSf out;
    out.id   = getElectronIdSf  (pt, scEta, syst);
    out.reco = getElectronRecoSf(pt, scEta, syst);
    out.trig = getElectronTrigSf(pt, scEta, syst);
    out.total = out.id * out.reco * out.trig;

    guard.checkFinite("ElectronSF_total", out.total);
    guard.checkSf("ElectronSF_total", out.total, 0.0, 5.0);

    if (isDebug_) {
        std::cout << "----------------------------\n"
                  << "Electron Scale Factors\n"
                  << "    pt     = " << pt  << "\n"
                  << "    scEta  = " << scEta << "\n"
                  << "    ID     = " << out.id   << "\n"
                  << "    Reco   = " << out.reco << "\n"
                  << "    Trig   = " << out.trig << "\n"
                  << "    Total  = " << out.total << "\n";
    }
    return out;
}

ScaleElectronFunction::EleSf ScaleElectronFunction::getElectronSfs(const SkimTree& skimT,
                                                                   int index,
                                                                   const std::string& systStr) const {
    return getElectronSfs(skimT, index, parseSyst(systStr));
}

