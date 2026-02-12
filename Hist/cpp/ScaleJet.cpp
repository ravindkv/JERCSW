#include "ScaleJet.h"
#include <iostream>
#include <cmath>

ScaleJet::ScaleJet(const GlobalFlag& globalFlags)
    : functions_(globalFlags),
      globalFlags_(globalFlags),
      level_(globalFlags_.getJecApplicationLevel()),
      isDebug_(globalFlags_.isDebug()),
      isData_(globalFlags_.isData()),
      isBookKeep_(globalFlags_.isBookKeep()),
      jetAlgo_(globalFlags_.getJetAlgo()),
      applyJer_(globalFlags_.applyJer() && !isData_)
{
}

void ScaleJet::applyCorrection(std::shared_ptr<SkimTree>& skimT) {
    if (!skimT) {
        std::cerr << "ScaleJet::applyCorrection: nullptr SkimTree\n";
        return;
    }

    if (isDebug_) std::cout << "\n[ScaleJet::applyCorrection]\n";

    // reset per-event state
    p4MapJet1_.clear();
    p4MapJetSum_.clear();
    jet_pt_raw_.assign(skimT->nJet, 0.0);

    for (int i = 0; i < skimT->nJet; ++i) {
        if (isDebug_) std::cout << "\n ===> Jet Index = " << i << "\n";

        const float pt_nano   = skimT->Jet_pt[i];
        const float rawFactor = skimT->Jet_rawFactor[i];
        if (isDebug_) std::cout << " pt_nano= " << pt_nano << '\n';
        const double pt_raw   = pt_nano * (1.f - rawFactor);
        jet_pt_raw_[i] = pt_raw;

        const float eta       = skimT->Jet_eta[i];
        const float phi       = skimT->Jet_phi[i];
        const float mass_nano = skimT->Jet_mass[i];
        const float area      = skimT->Jet_area[i];
        const double mass_raw = mass_nano * (1.f - rawFactor);

        if(pt_raw < 10) continue;//FIXME

        // --- Book-keeping: Nano/Raw ---
        if (isBookKeep_) {
            TLorentzVector p4Nano; p4Nano.SetPtEtaPhiM(pt_nano, eta, phi, mass_nano);
            TLorentzVector p4Raw;  p4Raw .SetPtEtaPhiM(pt_raw,  eta, phi, mass_raw);
            if (i == 0) {
                p4MapJet1_["Nano"] += p4Nano;
                p4MapJet1_["Raw"]  += p4Raw;
            }
            p4MapJetSum_["Nano"] += p4Nano;
            p4MapJetSum_["Raw"]  += p4Raw;
        }

        double pt_corr   = pt_raw;
        double mass_corr = mass_raw;

        // L1 RC
        if (level_ >= GlobalFlag::JecApplicationLevel::L1Rc && jetAlgo_ == GlobalFlag::JetAlgo::AK4Chs) {
            const double c1 = functions_.getL1FastJetCorrection(area, eta, pt_corr, skimT->Rho);
            pt_corr   *= c1;
            mass_corr *= c1;

            if (isBookKeep_) {
                TLorentzVector t; t.SetPtEtaPhiM(pt_corr, eta, phi, mass_corr);
                if (i == 0) p4MapJet1_["L1RcCorr"] += t;
                p4MapJetSum_["L1RcCorr"] += t;
            }
        }

        // L2Rel
        if (level_ >= GlobalFlag::JecApplicationLevel::L2Rel) {
            const double c2 = functions_.getL2RelativeCorrection(eta, pt_corr);
            pt_corr   *= c2;
            mass_corr *= c2;

            if (isBookKeep_) {
                TLorentzVector t; t.SetPtEtaPhiM(pt_corr, eta, phi, mass_corr);
                if (i == 0) p4MapJet1_["L2RelCorr"] += t;
                p4MapJetSum_["L2RelCorr"] += t;
            }
        }

        // L2Res / L2L3Res (data only)
        if (isData_ && (level_ >= GlobalFlag::JecApplicationLevel::L2L3Res)) {
            const double cR = functions_.getL2L3ResidualCorrection(eta, pt_corr);
            pt_corr   *= cR;
            mass_corr *= cR;

            if (isBookKeep_) {
                TLorentzVector t; t.SetPtEtaPhiM(pt_corr, eta, phi, mass_corr);
                if (i == 0) p4MapJet1_["L2L3ResCorr"] += t;
                p4MapJetSum_["L2L3ResCorr"] += t;
            }
        } else if (isData_ && (level_ >= GlobalFlag::JecApplicationLevel::L2Res)) {
            const double cR = functions_.getL2ResidualCorrection(eta, pt_corr);
            pt_corr   *= cR;
            mass_corr *= cR;

            if (isBookKeep_) {
                TLorentzVector t; t.SetPtEtaPhiM(pt_corr, eta, phi, mass_corr);
                if (i == 0) p4MapJet1_["L2ResCorr"] += t;
                p4MapJetSum_["L2ResCorr"] += t;
            }
        }

        // JER (MC only)
        if (applyJer_) {
            const double cJER = functions_.getJerCorrection(*skimT, i, "nom", pt_corr);
            pt_corr   *= cJER;
            mass_corr *= cJER;

            if (isBookKeep_) {
                TLorentzVector t; t.SetPtEtaPhiM(pt_corr, eta, phi, mass_corr);
                if (i == 0) p4MapJet1_["JerCorr"] += t;
                p4MapJetSum_["JerCorr"] += t;
            }
        }

        if (isBookKeep_) {
            TLorentzVector p4Corr; p4Corr.SetPtEtaPhiM(pt_corr, eta, phi, mass_corr);
            if (i == 0) p4MapJet1_["Corr"] += p4Corr;
            p4MapJetSum_["Corr"] += p4Corr;
        }

        skimT->Jet_pt[i]   = pt_corr;
        skimT->Jet_mass[i] = mass_corr;
    }
}

