// cpp/HemVeto.cpp
#include "HemVeto.h"
#include <iostream>

HemVeto::HemVeto(const GlobalFlag& globalFlags)
  : globalFlags_(globalFlags)
  , isMC_(globalFlags_.isMC())
{
    loadConfig_("config/HemVeto.json");
}

void HemVeto::loadConfig_(const std::string& filename) {
    ReadConfig cfg(filename);
    auto year = globalFlags_.getYearStr();

    applyHemVeto_  = cfg.getValue<bool>   ({"applyHemVeto"});
    runThreshold_  = cfg.getValue<int>    ({"runThreshold"});
    mcWeight_  = cfg.getValue<double>   ({"mcWeight"});

    eleCuts_.ptMin   = cfg.getValue<double>({"elePtMin"});
    eleCuts_.etaMin  = cfg.getValue<double>({"eleEtaMin"});
    eleCuts_.etaMax  = cfg.getValue<double>({"eleEtaMax"});
    eleCuts_.phiMin  = cfg.getValue<double>({"elePhiMin"});
    eleCuts_.phiMax  = cfg.getValue<double>({"elePhiMax"});

    phoCuts_.ptMin   = cfg.getValue<double>({"phoPtMin"});
    phoCuts_.etaMin  = cfg.getValue<double>({"phoEtaMin"});
    phoCuts_.etaMax  = cfg.getValue<double>({"phoEtaMax"});
    phoCuts_.phiMin  = cfg.getValue<double>({"phoPhiMin"});
    phoCuts_.phiMax  = cfg.getValue<double>({"phoPhiMax"});

    // jets have no pT cut in your snippet
    jetCuts_.ptMin   = cfg.getValue<double>({"jetPtMin"});
    jetCuts_.etaMin  = cfg.getValue<double>({"jetEtaMin"});
    jetCuts_.etaMax  = cfg.getValue<double>({"jetEtaMax"});
    jetCuts_.phiMin  = cfg.getValue<double>({"jetPhiMin"});
    jetCuts_.phiMax  = cfg.getValue<double>({"jetPhiMax"});

    if (globalFlags_.isDebug()) {
        std::cout << "[HemVeto] loaded for year=" << year
                  << ", apply=" << applyHemVeto_
                  << ", mcWeight=" << mcWeight_
                  << ", runThr=" << runThreshold_ << "\n";
    }
}

bool HemVeto::isHemVeto(const SkimTree& skimT) const {
    // master switch / year logic
    if (!applyHemVeto_ || globalFlags_.getYearStr() != "2018") return false;

    // require data-run ≥ threshold
    if (!isMC_ && skimT.run < runThreshold_) return false;

    // count electrons in HEM region
    int nEle = 0;
    for (int i = 0; i < skimT.nElectron; ++i) {
        double pt  = skimT.Electron_pt[i];
        double η   = skimT.Electron_eta[i];
        double φ   = skimT.Electron_phi[i];
        if (pt >= eleCuts_.ptMin
         && η  >  eleCuts_.etaMin && η  < eleCuts_.etaMax
         && φ  >  eleCuts_.phiMin && φ  < eleCuts_.phiMax)
        {
            ++nEle;
        }
    }

    // count photons in HEM region
    int nPho = 0;
    for (int i = 0; i < skimT.nPhoton; ++i) {
        double pt  = skimT.Photon_pt[i];
        double η   = skimT.Photon_eta[i];
        double φ   = skimT.Photon_phi[i];
        if (pt >= phoCuts_.ptMin
         && η  >  phoCuts_.etaMin && η  < phoCuts_.etaMax
         && φ  >  phoCuts_.phiMin && φ  < phoCuts_.phiMax)
        {
            ++nPho;
        }
    }

    // count jets in HEM region
    int nJet = 0;
    for (int i = 0; i < skimT.nJet; ++i) {
        double pt = skimT.Jet_pt[i];
        double η = skimT.Jet_eta[i];
        double φ = skimT.Jet_phi[i];
        if (pt >= jetCuts_.ptMin
         && jetCuts_.etaMin && η < jetCuts_.etaMax
         && φ > jetCuts_.phiMin && φ < jetCuts_.phiMax)
        {
            ++nJet;
        }
    }

    bool veto = (nEle > 0 || nPho > 0 || nJet > 0);
    if (globalFlags_.isDebug()) {
        std::cout << "[HemVeto] Run=" << skimT.run
                  << " nEle=" << nEle
                  << " nPho=" << nPho
                  << " nJet=" << nJet
                  << " => veto=" << veto << "\n";
    }
    return veto;
}

