#include "ScaleMet.h"
#include <iostream>
#include <cmath>

ScaleMet::ScaleMet(const GlobalFlag& globalFlags)
    : functions_(globalFlags),
      globalFlags_(globalFlags),
      level_(globalFlags_.getJecApplicationLevel()),
      isDebug_(globalFlags_.isDebug()),
      isData_(globalFlags_.isData()),
      isBookKeep_(globalFlags_.isBookKeep()),
      jetAlgo_(globalFlags_.getJetAlgo()),
      applyJer_(globalFlags_.applyJer() && !isData_),
      p4CorrectedMet_(0,0,0,0)
{
}

void ScaleMet::applyCorrection(const std::shared_ptr<SkimTree>& skimT,
                               const std::vector<double>& jetPtRaw) {
    if (!skimT) {
        std::cerr << "ScaleMet::applyCorrection: nullptr SkimTree\n";
        return;
    }
    if (static_cast<int>(jetPtRaw.size()) != skimT->nJet) {
        std::cerr << "ScaleMet::applyCorrection: jetPtRaw size mismatch\n";
    }

    if (isDebug_) std::cout << "\n[ScaleMet::applyCorrection]\n";

    // reset per-event state
    p4MapMet_.clear();
    p4CorrectedMet_.SetPtEtaPhiM(0,0,0,0);

    // Store Raw/Nano MET for comparison
    TLorentzVector p4MetRaw;  p4MetRaw.SetPtEtaPhiM(skimT->RawMET_pt, 0, skimT->RawMET_phi, 0);
    TLorentzVector p4MetNano; p4MetNano.SetPtEtaPhiM(skimT->MET_pt,   0, skimT->MET_phi,   0);

    if (isBookKeep_) {
        p4MapMet_["Raw"]  = p4MetRaw;
        p4MapMet_["Nano"] = p4MetNano;
    }

    if (isDebug_) std::cout << "\n ====> Met pT Nano = " << skimT->MET_pt << "\n";

    // Start from RAW MET
    double met_px = skimT->RawMET_pt * std::cos(skimT->RawMET_phi);
    double met_py = skimT->RawMET_pt * std::sin(skimT->RawMET_phi);

    // Loop corrected jets and apply Type-1
    for (int i = 0; i < skimT->nJet; ++i) {
        if (isDebug_) std::cout << "\n ---> Jet Index = " << i << "\n";

        const double eta  = skimT->Jet_eta[i];
        const double phi  = skimT->Jet_phi[i];
        const float  area = skimT->Jet_area[i];

        const double pt_raw = (i < static_cast<int>(jetPtRaw.size()))
                              ? jetPtRaw[i]
                              : skimT->Jet_pt[i]; // fallback
        const double pt_raw_minusMuon = pt_raw * (1 - skimT->Jet_muonSubtrFactor[i]);
        double pt_corr = pt_raw_minusMuon;

        if(pt_raw_minusMuon < 10) continue; //FIXME

        if (isDebug_) {
            std::cout << " pt_raw = " << pt_raw
                      << ", pt_raw_minusMuon = " << pt_raw_minusMuon << "\n";
        }

        if (level_ >= GlobalFlag::JecApplicationLevel::L1Rc && jetAlgo_ == GlobalFlag::JetAlgo::AK4Chs) {
            const double c1 = functions_.getL1FastJetCorrection(area, eta, pt_corr, skimT->Rho);
            pt_corr *= c1;
        }
        // L1 RC reference point
        const double pt_corr_l1rc = pt_corr;

        if (level_ >= GlobalFlag::JecApplicationLevel::L2Rel) {
            const double c2 = functions_.getL2RelativeCorrection(eta, pt_corr);
            pt_corr *= c2;
        }

        if (isData_ && (level_ >= GlobalFlag::JecApplicationLevel::L2L3Res)) {
            const double cR = functions_.getL2L3ResidualCorrection(eta, pt_corr);
            pt_corr *= cR;
        } else if (isData_ && (level_ >= GlobalFlag::JecApplicationLevel::L2Res)) {
            const double cR = functions_.getL2ResidualCorrection(eta, pt_corr);
            pt_corr *= cR;
        }

        if (applyJer_) {
            const double cJER = functions_.getJerCorrection(*skimT, i, "nom", pt_corr);
            pt_corr *= cJER;
        }

        // selection for propagation (same as original)
        const bool passSel = (pt_corr > 15.0
                              && std::abs(eta) < 5.2
                              && (skimT->Jet_neEmEF[i] + skimT->Jet_chEmEF[i]) < 0.9);
        if (!passSel) continue;

        const double dpt = (pt_corr - pt_corr_l1rc);
        met_px -= dpt * std::cos(phi);
        met_py -= dpt * std::sin(phi);

        if (isDebug_) std::cout << " -> Jet Index Added to MET = " << i << "\n";
        if (isDebug_) std::cout << " -> dpt Added to MET = " << dpt << "\n";
    }

    // finalize MET
    const double met_pt  = std::hypot(met_px, met_py);
    const double met_phi = std::atan2(met_py, met_px);
    TLorentzVector p4MetCorr; p4MetCorr.SetPtEtaPhiM(met_pt, 0, met_phi, 0);

    if (isDebug_) std::cout << "\n ===> Met pT Type-1 Corrected = " << met_pt << "\n";

    if (isBookKeep_) {
        p4MapMet_["Corr"] = p4MetCorr;
    }
    p4CorrectedMet_ = p4MetCorr;
}

// Optional debug helper if you want to compare with jets externally
void ScaleMet::printDebug(const std::unordered_map<std::string, TLorentzVector>& p4MapJetSum) const {
    if (!isDebug_ || !isBookKeep_) return;

    auto printCorrections = [](const std::unordered_map<std::string, TLorentzVector>& m, const std::string& h){
        std::cout << h << '\n';
        for (const auto& kv : m) {
            const auto& label = kv.first;
            const auto& p4    = kv.second;
            std::cout << "  " << label << " Pt: " << p4.Pt() << ", Mass: " << p4.M() << '\n';
        }
        std::cout << '\n';
    };

    if (!p4MapMet_.empty()) {
        printCorrections(p4MapMet_, "Met Corrections:");
    }

    if (p4MapMet_.count("Nano") && p4MapMet_.count("Corr") &&
        p4MapJetSum.count("Nano") && p4MapJetSum.count("Corr")) {
        std::cout << "Momentum check (scalar diagnostic only):\n";
        std::cout << "  At Nano   : " << (p4MapMet_.at("Nano") - p4MapJetSum.at("Nano")).Pt() << '\n';
        std::cout << "  After JEC : " << (p4MapMet_.at("Corr") - p4MapJetSum.at("Corr")).Pt() << '\n';
    }
}

