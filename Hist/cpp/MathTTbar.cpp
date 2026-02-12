#include "MathTTbar.h"
#include "TLorentzVector.h"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <limits> // for std::numeric_limits
#include "MathMetPz.h"

MathTTbar::MathTTbar(const GlobalFlag& globalFlags, double bTagThresh)
    : globalFlags_(globalFlags),
      bTagThresh_(bTagThresh),
      chiSqr_(9e9),
      goodCombo_(false),
      resLep_(0.0),
      resMet_(0.0),
      resHadW_(0.0),
      resHadT_(0.0),
      resLepT_(0.0),
      useResolutions_(false),
      massLep_(0.0),
      massW_(80.4),
      massT_(172.0),
      pzMet_(0.0)
{
    // Initialize four-vectors to something safe
    p4Met_.SetPxPyPzE(0,0,0,0);
    p4HadB_.SetPxPyPzE(0,0,0,0);
    p4Jet1_.SetPxPyPzE(0,0,0,0);
    p4Jet2_.SetPxPyPzE(0,0,0,0);
    p4LepB_.SetPxPyPzE(0,0,0,0);
    p4Lep_.SetPxPyPzE(0,0,0,0);
    p4FullMet_.SetPxPyPzE(0,0,0,0);

    p4HadT_.SetPxPyPzE(0,0,0,0);
    p4HadW_.SetPxPyPzE(0,0,0,0);
    p4LepT_.SetPxPyPzE(0,0,0,0);
}

void MathTTbar::setEventObjects(const TLorentzVector& p4Lep, 
                                const TLorentzVector& p4Met, 
                                const std::vector<int>& indexJets)
{
    p4Lep_  = p4Lep;
    p4Met_  = p4Met;
    indexJets_ = indexJets;

    // We won't separate b-jets / non-b-jets here. We'll do that
    // in minimizeChiSqr() after we look at the bTag values.
}

void MathTTbar::setMass(double massLep, double massW, double massT) 
{
    massLep_ = massLep;
    massW_ = massW;
    massT_ = massT;

}

void MathTTbar::setResolution(double resLep, double resMet, double resHadW, double resHadT, 
                              double resLepT, const std::vector<double>& resJets, bool useRes)
{
    resLep_ = resLep;
    resMet_ = resMet;
    resHadW_ = resHadW;
    resHadT_ = resHadT;
    resLepT_ = resLepT;
    resJets_ = resJets;
    useResolutions_ = useRes;
}

double MathTTbar::calcChiSqr(const SkimTree& skimT,
                             int iHadB, int iLepB,
                             int iJet1, int iJet2,
                             double pzMetHypo)
{
    // Work on a local copy of the MET four-vector so that p4Met_ is not overwritten.
    TLorentzVector metHyp = p4Met_;
    metHyp.SetXYZM(metHyp.Px(), metHyp.Py(), pzMetHypo, 0.0); // neutrino massless

    // Build the jet four-vectors
    TLorentzVector p4HadB, p4LepB, p4Jet1, p4Jet2;
    p4HadB.SetPtEtaPhiM( skimT.Jet_pt[iHadB],  skimT.Jet_eta[iHadB],
                         skimT.Jet_phi[iHadB], skimT.Jet_mass[iHadB]); 
    p4LepB.SetPtEtaPhiM( skimT.Jet_pt[iLepB],  skimT.Jet_eta[iLepB],
                         skimT.Jet_phi[iLepB], skimT.Jet_mass[iLepB]); 
    p4Jet1.SetPtEtaPhiM( skimT.Jet_pt[iJet1],  skimT.Jet_eta[iJet1],
                         skimT.Jet_phi[iJet1], skimT.Jet_mass[iJet1]); 
    p4Jet2.SetPtEtaPhiM( skimT.Jet_pt[iJet2],  skimT.Jet_eta[iJet2],
                         skimT.Jet_phi[iJet2], skimT.Jet_mass[iJet2]); 

    // For now the uncertainties are fixed; real analyses might do more dynamic resolution usage.
    double sigma2HadW = resHadW_ * resHadW_;
    double sigma2HadT = resHadT_ * resHadT_;
    double sigma2LepT = resLepT_ * resLepT_;

    if (useResolutions_) {
        // Example (commented out):
        // sigma2HadT = sigmaJet1*sigmaJet1 + sigmaJet2*sigmaJet2 + sigmaHadB*sigmaHadB;
        // sigma2HadW = sigmaJet1*sigmaJet1 + sigmaJet2*sigmaJet2;
        // sigma2LepT = sigmaLepB*sigmaLepB + resMet_*resMet_ + resLep_*resLep_;
    }

    // Reconstruct the candidate four-vectors
    TLorentzVector p4HadT = p4HadB + p4Jet1 + p4Jet2;
    TLorentzVector p4HadW = p4Jet1 + p4Jet2;
    TLorentzVector p4LepT = p4LepB + p4Lep_ + metHyp;

    // Compute the χ² using the differences from the expected masses.
    double chiSq = std::pow(p4HadT.M() - massT_, 2) / sigma2HadT +
                   std::pow(p4HadW.M() - massW_, 2) / sigma2HadW +
                   std::pow(p4LepT.M() - massT_, 2) / sigma2LepT;

    return chiSq;
}

int MathTTbar::minimizeChiSqr(const SkimTree& skimT)
{
    // Reset some variables at start
    goodCombo_ = false;
    chiSqr_ = 9e9;
    indexJetsB_.clear();
    indexJetsNonB_.clear();

    // We require at least 4 jets total to form t->(bjj) and the other t->(blep).
    if (indexJets_.size() < 4) {
        if (globalFlags_.isDebug()) {
            std::cerr << "[MathTTbar] Not enough jets overall (<4). Returning -1.\n";
        }
        return -1;
    }

    // 1) Find all jets passing bTagThresh_
    std::vector<std::pair<double,int>> bJetCandidates; // (bVal, idx)
    std::vector<int> otherJets; // jets that fail threshold

    for (int jIdx : indexJets_) {
        double bVal = skimT.Jet_btagDeepFlavB[jIdx];
        if (bVal > bTagThresh_) {
            bJetCandidates.emplace_back(bVal, jIdx);
        } else {
            otherJets.push_back(jIdx);
        }
    }

    // 2) If more than 2 pass, pick top 2 in bTagVal. The rest go to otherJets.
    std::sort(bJetCandidates.begin(), bJetCandidates.end(),
              [](auto &a, auto &b){ return a.first > b.first; }); // descending by bVal

    if (bJetCandidates.size() >= 2) {
        // pick the first 2
        indexJetsB_.push_back( bJetCandidates[0].second );
        indexJetsB_.push_back( bJetCandidates[1].second );

        // all others become "nonB" 
        for (size_t i = 2; i < bJetCandidates.size(); ++i) {
            otherJets.push_back(bJetCandidates[i].second);
        }
    }
    else {
        // fewer than 2 jets pass threshold => we can't form both hadronic b and leptonic b
        if (globalFlags_.isDebug()) {
            std::cerr << "[MathTTbar] Not enough bTag>Thresh jets. Returning -1.\n";
        }
        return -1;
    }

    // Now indexJetsNonB_ is a combination of all jets that fail the threshold
    // plus any "excess" b-tagged jets beyond the top 2
    indexJetsNonB_ = otherJets;

    // Need at least 2 non-b jets to form the hadronic W
    if (indexJetsNonB_.size() < 2) {
        if (globalFlags_.isDebug()) {
            std::cerr << "[MathTTbar] Not enough non-b jets to build hadronic W. -1.\n";
        }
        return -1;
    }

    // 3) Compute neutrino pz solutions from (lep + MET).
    MathMetPz mathMetPz;
    mathMetPz.setInput(p4Met_, p4Lep_, massLep_, massW_);
    mathMetPz.calcMetPz();

    double pzBase = mathMetPz.getSelectedPz();
    double pzOther = mathMetPz.getAlternatePz();
    std::vector<double> pzMets;
    pzMets.push_back(pzBase);
    if (pzOther != pzBase) {
        pzMets.push_back(pzOther);
    }

    // 4) Try all combinations:
    //    indexJetsB_ has exactly 2 entries: iHadB, iLepB
    //    indexJetsNonB_ might have many. We pick 2 for the hadronic W.
    //    We also permute which b is hadronic vs. leptonic.
    chiSqr_ = std::numeric_limits<double>::max();
    double comboChi2 = 9e9;

    // indexJetsB_.size()==2 by construction, but let's do a small loop to consider permutations
    //   iHadB = indexJetsB_[0], iLepB = indexJetsB_[1]
    //   OR iHadB = indexJetsB_[1], iLepB = indexJetsB_[0]
    // Then for the non-b jets, pick any distinct pair (i < j).
    for (int pass=0; pass<2; pass++){
        int iHadB = (pass==0 ? indexJetsB_[0] : indexJetsB_[1]);
        int iLepB = (pass==0 ? indexJetsB_[1] : indexJetsB_[0]);

        for (size_t i = 0; i < indexJetsNonB_.size(); ++i) {
            for (size_t j = i+1; j < indexJetsNonB_.size(); ++j) {
                int iJet1 = indexJetsNonB_[i];
                int iJet2 = indexJetsNonB_[j];

                // Loop over neutrino pz solutions
                for (double testNuPz : pzMets) {
                    comboChi2 = calcChiSqr(skimT, iHadB, iLepB, iJet1, iJet2, testNuPz);
                    if (comboChi2 < chiSqr_) {
                        chiSqr_ = comboChi2;
                        indexHadB_ = iHadB;
                        indexLepB_ = iLepB;
                        index1ForW_ = iJet1;
                        index2ForW_ = iJet2;
                        pzMet_ = testNuPz;
                        goodCombo_ = true;
                    }
                }
            }
        }
    }

    setFinalP4s(skimT);
    if (globalFlags_.isDebug()) {
      mathMetPz.print(); 
      print();
    }
    return (goodCombo_ ? 0 : -1);
}

void MathTTbar::setFinalP4s(const SkimTree& skimT)
{

    p4FullMet_.SetXYZM(p4Met_.Px(), p4Met_.Py(), pzMet_, 0.0); // neutrino massless

    // Build the jet four-vectors
    p4HadB_.SetPtEtaPhiM( skimT.Jet_pt[indexHadB_],  skimT.Jet_eta[indexHadB_],
                         skimT.Jet_phi[indexHadB_], skimT.Jet_mass[indexHadB_]); 
    p4LepB_.SetPtEtaPhiM( skimT.Jet_pt[indexLepB_],  skimT.Jet_eta[indexLepB_],
                         skimT.Jet_phi[indexLepB_], skimT.Jet_mass[indexLepB_]); 
    p4Jet1_.SetPtEtaPhiM( skimT.Jet_pt[index1ForW_],  skimT.Jet_eta[index1ForW_],
                         skimT.Jet_phi[index1ForW_], skimT.Jet_mass[index1ForW_]); 
    p4Jet2_.SetPtEtaPhiM( skimT.Jet_pt[index2ForW_],  skimT.Jet_eta[index2ForW_],
                         skimT.Jet_phi[index2ForW_], skimT.Jet_mass[index2ForW_]); 

    // Reconstruct the candidate four-vectors
    p4HadT_ = p4HadB_ + p4Jet1_ + p4Jet2_;
    p4HadW_ = p4Jet1_ + p4Jet2_;
    p4LepT_ = p4LepB_ + p4Lep_ + p4FullMet_;

}

void MathTTbar::print() const
{
    std::cout << "=== MathTTbar Debug Info ===" << std::endl;
    std::cout << "  bTagThreshold = " << bTagThresh_ << std::endl;
    std::cout << "  Chi2 TT       = " << chiSqr_ << std::endl;
    std::cout << "  Good combo    = " << std::boolalpha << goodCombo_ << std::noboolalpha << std::endl;
    std::cout << std::endl;

    std::cout << "Mases : " << std::endl;
    std::cout << "massLep : " << massLep_<< std::endl;
    std::cout << "massW : " << massW_<< std::endl;
    std::cout << "massT : " << massT_<< std::endl;

    std::cout << "Lepton p4:" << std::endl;
    std::cout << "  E  : " << p4Lep_.E()  << std::endl;
    std::cout << "  Px : " << p4Lep_.Px() << std::endl;
    std::cout << "  Py : " << p4Lep_.Py() << std::endl;
    std::cout << "  Pz : " << p4Lep_.Pz() << std::endl;
    std::cout << "Lepton resolution: " << resLep_ << std::endl;
    std::cout << std::endl;

    std::cout << "MET p4:" << std::endl;
    std::cout << "  Px : " << p4Met_.Px() << std::endl;
    std::cout << "  Py : " << p4Met_.Py() << std::endl;
    std::cout << "  Pz : " << p4Met_.Pz() << std::endl;
    std::cout << "  Pt : " << p4Met_.Pt() << std::endl;
    std::cout << "  Eta : " << p4Met_.Eta() << std::endl;
    std::cout << "Full MET p4:" << std::endl;
    std::cout << "  Px : " << p4FullMet_.Px() << std::endl;
    std::cout << "  Py : " << p4FullMet_.Py() << std::endl;
    std::cout << "  Pz : " << p4FullMet_.Pz() << std::endl;
    std::cout << "  Pt : " << p4FullMet_.Pt() << std::endl;
    std::cout << "  Eta : " << p4FullMet_.Eta() << std::endl;
    std::cout << "  (Selected neutrino Pz : " << pzMet_ << ")" << std::endl;
    std::cout << "MET resolution : " << resMet_ << std::endl;
    std::cout << "HadW resolution : " << resHadW_ << std::endl;
    std::cout << "HadT resolution : " << resHadT_ << std::endl;
    std::cout << "LepT resolution : " << resLepT_ << std::endl;
    std::cout << std::endl;

    std::cout << "All jets (indexJets_) size = " << indexJets_.size() << std::endl;

    std::cout << "Chosen 2 b-jets (indexJetsB_): ";
    for (auto bIdx : indexJetsB_) {
        std::cout << bIdx << " ";
    }
    std::cout << std::endl;

    std::cout << "Non-b jets (indexJetsNonB_): ";
    for (auto nbIdx : indexJetsNonB_) {
        std::cout << nbIdx << " ";
    }
    std::cout << std::endl;

    std::cout << "Selected combo indices:" << std::endl;
    std::cout << "  iHadB  : " << indexHadB_ << std::endl;
    std::cout << "  iLepB  : " << indexLepB_ << std::endl;
    std::cout << "  iJet1  : " << index1ForW_ << std::endl;
    std::cout << "  iJet2  : " << index2ForW_ << std::endl;
    std::cout << "=== End MathTTbar Debug Info ===" << std::endl;
}

