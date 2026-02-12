#include "ScaleMuon.h"

#include <iostream>
#include <iomanip>

ScaleMuon::ScaleMuon(const GlobalFlag& globalFlags)
    : functions_(globalFlags),
      globalFlags_(globalFlags),
      isDebug_(globalFlags_.isDebug()),
      isData_(globalFlags_.isData()),
      isMC_(globalFlags_.isMC()) {
    initializeP4Map();
}

double ScaleMuon::getMuonRochCorrection(const SkimTree& skimT, int index, const std::string& syst) const {
    return functions_.getMuonRochCorrection(skimT, index, syst);
}

double ScaleMuon::getMuonIdSf(double pt, double eta, SystLevel syst) const {
    return functions_.getMuonIdSf(pt, eta, syst);
}
double ScaleMuon::getMuonIsoSf(double pt, double eta, SystLevel syst) const {
    return functions_.getMuonIsoSf(pt, eta, syst);
}
double ScaleMuon::getMuonTrigSf(double pt, double eta, SystLevel syst) const {
    return functions_.getMuonTrigSf(pt, eta, syst);
}

ScaleMuon::MuSf ScaleMuon::getMuonSfs(const SkimTree& skimT, int index, SystLevel syst) const {
    return functions_.getMuonSfs(skimT, index, syst);
}
ScaleMuon::MuSf ScaleMuon::getMuonSfs(const SkimTree& skimT, int index, const std::string& systStr) const {
    return functions_.getMuonSfs(skimT, index, systStr);
}

// ---------------- p4 corrections (Rochester) ----------------
void ScaleMuon::initializeP4Map() {
    p4MapMuon1_.clear();
    for (const auto& name : std::vector<std::string>{"Nano", "Corr"}) {
        p4MapMuon1_[name] = TLorentzVector();
    }
}

void ScaleMuon::applyCorrections(std::shared_ptr<SkimTree>& skimT) {
    initializeP4Map();

    TLorentzVector p4Muon;
    for (int j = 0; j < skimT->nMuon; ++j) {
        p4Muon.SetPtEtaPhiM(skimT->Muon_pt[j], skimT->Muon_eta[j],
                            skimT->Muon_phi[j], skimT->Muon_mass[j]);
        if (j == 0) p4MapMuon1_["Nano"] = p4Muon;

        const double corr = functions_.getMuonRochCorrection(*skimT, j, "nom");
        if(corr > 0.0) skimT->Muon_pt[j] *= corr;//FIXME

        p4Muon.SetPtEtaPhiM(skimT->Muon_pt[j], skimT->Muon_eta[j],
                            skimT->Muon_phi[j], skimT->Muon_mass[j]);
        if (j == 0) p4MapMuon1_["Corr"] = p4Muon;
    }
}

// ---------------- Print ----------------
void ScaleMuon::print() const {
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
    printCorrections(p4MapMuon1_, "Muon1 Corrections:");
}

