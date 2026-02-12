#include "ScaleElectron.h"

#include <iostream>
#include <iomanip>

ScaleElectron::ScaleElectron(const GlobalFlag& globalFlags)
    : functions_(globalFlags),
      globalFlags_(globalFlags),
      isDebug_(globalFlags_.isDebug()),
      isData_(globalFlags_.isData()),
      isMC_(globalFlags_.isMC()) {

    initializeP4Map();
}

double ScaleElectron::getElectronSsCorrection(const SkimTree& skimT, int index, const std::string& syst) const {
    return functions_.getElectronSsCorrection(skimT, index, syst);
}

double ScaleElectron::getElectronIdSf(double pt, double eta, SystLevel syst) const {
    return functions_.getElectronIdSf(pt, eta, syst);
}
double ScaleElectron::getElectronRecoSf(double pt, double eta, SystLevel syst) const {
    return functions_.getElectronRecoSf(pt, eta, syst);
}
double ScaleElectron::getElectronTrigSf(double pt, double eta, SystLevel syst) const {
    return functions_.getElectronTrigSf(pt, eta, syst);
}

ScaleElectron::EleSf ScaleElectron::getElectronSfs(const SkimTree& skimT, int index, SystLevel syst) const {
    return functions_.getElectronSfs(skimT, index, syst);
}
ScaleElectron::EleSf ScaleElectron::getElectronSfs(const SkimTree& skimT, int index, const std::string& systStr) const {
    return functions_.getElectronSfs(skimT, index, systStr);
}

// ---------------- Kinematic corrections (p4) ----------------
void ScaleElectron::initializeP4Map() {
    p4MapElectron1_.clear();
    for (const auto& name : std::vector<std::string>{"Nano", "Corr"}) {
        p4MapElectron1_[name] = TLorentzVector();
    }
}

void ScaleElectron::applyCorrections(std::shared_ptr<SkimTree>& skimT) {
    initializeP4Map();

    TLorentzVector p4Electron;
    for (int j = 0; j < skimT->nElectron; ++j) {
        p4Electron.SetPtEtaPhiM(skimT->Electron_pt[j], skimT->Electron_eta[j],
                                skimT->Electron_phi[j], skimT->Electron_mass[j]);
        if (j == 0) p4MapElectron1_["Nano"] = p4Electron;

        double corr = 1.0;
        if (globalFlags_.getNanoVersion() == GlobalFlag::NanoVersion::V15) {
            corr = functions_.getElectronSsCorrection(*skimT, j, "nom");
        }

        skimT->Electron_pt[j]   *= corr;
        skimT->Electron_mass[j] *= corr;

        p4Electron.SetPtEtaPhiM(skimT->Electron_pt[j], skimT->Electron_eta[j],
                                skimT->Electron_phi[j], skimT->Electron_mass[j]);
        if (j == 0) p4MapElectron1_["Corr"] = p4Electron;
    }
}

// ---------------- Print ----------------
void ScaleElectron::print() const {
    auto printCorrections = [](const std::unordered_map<std::string, TLorentzVector>& corrections,
                               const std::string& header) {
        std::cout << header << '\n';
        for (const auto& kv : corrections) {
            std::cout << "  " << kv.first
                      << " Pt: " << kv.second.Pt()
                      << ", Mass: " << kv.second.M()
                      << '\n';
        }
        std::cout << '\n';
    };

    std::cout << std::fixed << std::setprecision(3);
    printCorrections(p4MapElectron1_, "Electron1 Corrections:");
}

