#include "ScalePhoton.h"

#include <iostream>
#include <iomanip>

ScalePhoton::ScalePhoton(const GlobalFlag& globalFlags)
    : functions_(globalFlags),
      globalFlags_(globalFlags),
      isDebug_(globalFlags_.isDebug()),
      isData_(globalFlags_.isData()),
      isMC_(globalFlags_.isMC()) {
    initializeP4Map();
}

double ScalePhoton::getPhotonScaleCorrection(const SkimTree& skimT,
                                            const std::string& nomOrSyst,
                                            int index) const {
    return functions_.getPhotonScaleCorrection(skimT, nomOrSyst, index);
}

double ScalePhoton::getPhotonSmearCorrection(const SkimTree& skimT,
                                            const std::string& nomOrSyst,
                                            int index) const {
    return functions_.getPhotonSmearCorrection(skimT, nomOrSyst, index);
}

double ScalePhoton::getPhotonIdSf(double pt, double eta, SystLevel syst) const {
    return functions_.getPhotonIdSf(pt, eta, syst);
}
double ScalePhoton::getPhotonPsSf(double pt, double eta, SystLevel syst) const {
    return functions_.getPhotonPsSf(pt, eta, syst);
}
double ScalePhoton::getPhotonCsSf(double pt, double eta, SystLevel syst) const {
    return functions_.getPhotonCsSf(pt, eta, syst);
}

ScalePhoton::PhotonSf ScalePhoton::getPhotonSfs(const SkimTree& skimT, int index, SystLevel syst) const {
    return functions_.getPhotonSfs(skimT, index, syst);
}
ScalePhoton::PhotonSf ScalePhoton::getPhotonSfs(const SkimTree& skimT, int index, const std::string& systStr) const {
    return functions_.getPhotonSfs(skimT, index, systStr);
}

// ---------------- p4 corrections ----------------
void ScalePhoton::initializeP4Map() {
    p4MapPhoton1_.clear();
    for (const auto& name : std::vector<std::string>{"Nano", "Corr"}) {
        p4MapPhoton1_[name] = TLorentzVector();
    }
}

void ScalePhoton::applyCorrections(std::shared_ptr<SkimTree>& skimT) {
    initializeP4Map();

    TLorentzVector p4Photon;
    for (int j = 0; j < skimT->nPhoton; ++j) {
        p4Photon.SetPtEtaPhiM(skimT->Photon_pt[j], skimT->Photon_eta[j],
                              skimT->Photon_phi[j], skimT->Photon_mass[j]);
        if (j == 0) p4MapPhoton1_["Nano"] = p4Photon;

        double corr = 1.0;
        // If you enable Ss shifts, derive corr via getPhotonScale/SmearCorrection here for up/down
        skimT->Photon_pt[j]   *= corr;
        skimT->Photon_mass[j] *= corr;

        p4Photon.SetPtEtaPhiM(skimT->Photon_pt[j], skimT->Photon_eta[j],
                              skimT->Photon_phi[j], skimT->Photon_mass[j]);
        if (j == 0) p4MapPhoton1_["Corr"] = p4Photon;
    }
}

// ---------------- Print ----------------
void ScalePhoton::print() const {
    auto printCorrections = [](const std::unordered_map<std::string, TLorentzVector>& corrections,
                               const std::string& header) {
        std::cout << header << '\n';
        for (const auto& [label, p4] : corrections) {
            std::cout << "  " << label
                      << " Pt: " << p4.Pt()
                      << ", Mass: " << p4.M()
                      << '\n';
        }
        std::cout << '\n';
    };

    std::cout << std::fixed << std::setprecision(3);
    printCorrections(p4MapPhoton1_, "Photon1 Corrections:");
}

