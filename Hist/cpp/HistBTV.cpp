#include "HistBTV.h"
#include "HelperDir.hpp"

// bring in the full definitions for our forward‐declared types
#include "VarBin.h"
#include "SkimTree.h"

#include <cassert>           // assert()
#include <cmath>             // std::abs
#include <iostream>          // std::cerr
#include <vector>            // std::vector
#include <unordered_map>     // std::unordered_map

// ROOT
#include <TH1D.h>
#include <TProfile.h>
#include <TDirectory.h>

HistBTV::~HistBTV() = default;

HistBTV::HistBTV(TDirectory *origDir, const std::string& directoryName, const VarBin& varBin, const GlobalFlag& globalFlags)
    : 
     threshBtagDeepFlavB_(1.0),
     threshBtagDeepFlavCvL_(1.0),
     threshBtagDeepFlavCvB_(1.0),
     threshBtagDeepFlavG_(1.0),
     threshBtagDeepFlavQG_(1.0),
     threshBtagDeepFlavUDS_(1.0),
     bal_(1.0),
     mpf_(1.0),
     mpf1_(1.0), 
     mpfn_(1.0), 
     mpfu_(1.0), 
     year_(globalFlags.getYear()),
     isMC_(globalFlags.isMC()),
     isDebug_(globalFlags.isDebug())
{
    if (year_ == GlobalFlag::Year::Year2016Pre) {
        threshBtagDeepFlavB_    = 0.6502;
        threshBtagDeepFlavCvL_  = 0.270;
        threshBtagDeepFlavCvB_  = 0.256;
        threshBtagDeepFlavQG_    = 0.5;
        threshBtagDeepFlavG_    = 0.5;
    }
    if (year_ == GlobalFlag::Year::Year2016Post) {
        threshBtagDeepFlavB_    = 0.6377;
        threshBtagDeepFlavCvL_  = 0.269;
        threshBtagDeepFlavCvB_  = 0.247;
        threshBtagDeepFlavQG_    = 0.5;
        threshBtagDeepFlavG_    = 0.5;
    }
    if (year_ == GlobalFlag::Year::Year2017) {
        threshBtagDeepFlavB_    = 0.7476;
        threshBtagDeepFlavCvL_  = 0.520;
        threshBtagDeepFlavCvB_  = 0.050; 
        threshBtagDeepFlavQG_    = 0.5;
        threshBtagDeepFlavG_    = 0.5;
    }
    if (year_ == GlobalFlag::Year::Year2018) {
        threshBtagDeepFlavB_    = 0.7100;
        threshBtagDeepFlavCvL_  = 0.282;
        threshBtagDeepFlavCvB_  = 0.267;
        threshBtagDeepFlavQG_    = 0.5;
        threshBtagDeepFlavG_    = 0.5;
    }

    // Initialize histograms and the mvar_ map
    InitializeHistograms(origDir, directoryName, varBin);
}

void HistBTV::SetResponse(const double &bal, const double &mpf, const double &mpf1, const double &mpfn, const double &mpfu){
    bal_ = bal;
    mpf_ = mpf;
    mpf1_ = mpf1;
    mpfn_ = mpfn;
    mpfu_ = mpfu;
}

void HistBTV::InitializeHistograms(TDirectory *origDir, const std::string& directoryName, const VarBin& varBin) {

    // Use the HelperDir method to get or create the directory
    std::string dirName = directoryName + "/HistBTV";
    TDirectory* newDir = HelperDir::createTDirectory(origDir, dirName);
    newDir->cd();

    std::vector<double> binsPt  = varBin.getBinsPt();
    const int nPt  = binsPt.size()  - 1;
    // Define the list of variables to initialize
    std::vector<std::string> variables = {
        "h1EventInRefPt",
        "h1EventInGenJet1Pt",
        "p1MpfRespInRefPt",
        "p1DbRespInRefPt",
        "p1MpfResp1InRefPt",
        "p1MpfRespNInRefPt",
        "p1MpfRespUInRefPt",
        "p1RhoInRefPt",
        "p1Jet1PtOverRefPtInRefPt",
        "p1GenJet1PtOverRefPtInRefPt",
        "p1Jet1PtOverGenJet1PtInGenJet1Pt",
        "p1Jet1ChHefInRefPt", 
        "p1Jet1NeHefInRefPt", 
        "p1Jet1NeEmefInRefPt", 
        "p1Jet1ChEmefInRefPt", 
        "p1Jet1MuefInRefPt" 
    };

    // Initialize tag and flavor identifiers
    //atag_ = {"i", "b", "c", "q", "g", "n"};
    //aflv_ = {"i", "b", "c", "q", "s", "ud", "g", "n"};

    atag_ = {"i", "b"};
    aflv_ = {"i"};
    // Loop over all combinations of variables, tags, and flavors to initialize histograms
    for (const auto& var : variables) {
        mvar_[var] = 1.0;//initialize
        for (const auto& tag : atag_) {
            for (const auto& flv : aflv_) {
                // Construct the histogram name based on the naming convention
                std::string histName = var + "_" + tag + flv;
                
                // Initialize TH1D or TProfile based on the variable type
                if (var == "h1EventInGenJet1Pt" || var == "h1EventInRefPt") {
                    varTagFlvMap_[var][tag][flv] = std::make_unique<TH1D>(
                        histName.c_str(),
                        ";#it{p}_{T} (GeV);Events",
                        nPt,
                        binsPt.data()
                    );
                } else {
                    varTagFlvMap_[var][tag][flv] = std::make_unique<TProfile>(
                        histName.c_str(),
                        ";#it{p}_{T} (GeV);Value",
                        nPt,
                        binsPt.data()
                    );
                }
            }//aflv_
        }//atag_
    }//variables
    origDir->cd();
}

void HistBTV::FillHistograms(
    SkimTree* skimT,
    double ptRef,
    int iJet1,
    int iGenJet,
    double weight
) {
   double genJetPt = skimT->GenJet_pt[iGenJet]; 
   double jetPt = skimT->Jet_pt[iJet1]; 
   double chHEF  =  skimT->Jet_chHEF[iJet1];
   double neHEF  =  skimT->Jet_neHEF[iJet1];
   double neEmEF =  skimT->Jet_neEmEF[iJet1];
   double chEmEF =  skimT->Jet_chEmEF[iJet1];
   double muEF   =  skimT->Jet_muEF[iJet1];
   mvar_["h1EventInRefPt"] = 1.0;
   mvar_["p1DbRespInRefPt"] = bal_;
   mvar_["p1MpfRespInRefPt"] = mpf_;
   mvar_["p1MpfResp1InRefPt"] = mpf1_;
   mvar_["p1MpfRespNInRefPt"] = mpfn_;
   mvar_["p1MpfRespUInRefPt"] = mpfu_;
   mvar_["p1RhoInRefPt"] = skimT->Rho;
   mvar_["p1Jet1PtOverRefPtInRefPt"] = jetPt/ ptRef;
   mvar_["p1GenJet1PtOverRefPtInRefPt"] = (genJetPt / ptRef);
   mvar_["p1Jet1PtOverGenJet1PtInGenJet1Pt"] = (genJetPt != 0 ? jetPt / genJetPt : 0.0);
   mvar_["p1Jet1ChHefInRefPt"]  = chHEF;
   mvar_["p1Jet1NeHefInRefPt"]  = neHEF;
   mvar_["p1Jet1NeEmefInRefPt"] = neEmEF;
   mvar_["p1Jet1ChEmefInRefPt"] = chEmEF;
   mvar_["p1Jet1MuefInRefPt" ]  = muEF;

    // Determine the flavor of the jet
    int flv = (isMC_ ? skimT->GenJet_partonFlavour[iGenJet] : 99);

    // Evaluate tagging conditions based on jet properties
    bool isb = (skimT->Jet_btagDeepFlavB[iJet1] > threshBtagDeepFlavB_);
    bool isc = (skimT->Jet_btagDeepFlavCvL[iJet1] > threshBtagDeepFlavCvL_ 
                && skimT->Jet_btagDeepFlavCvB[iJet1] > threshBtagDeepFlavCvB_ 
                && !isb);
    bool isq = (skimT->Jet_btagDeepFlavQG[iJet1] > threshBtagDeepFlavQG_ 
                && !isb 
                && !isc);
    bool isg = (skimT->Jet_btagDeepFlavQG[iJet1] < threshBtagDeepFlavG_ 
                && !isb 
                && !isc);
    bool isn = (!isb && !isc && !isq && !isg);

    // Loop over all variables to fill the corresponding histograms
    for (const auto& var_pair : mvar_) {
        const std::string& var = var_pair.first;
        double val = var_pair.second;

        for (const auto& tag : atag_) {
            for (const auto& flv_str : aflv_) {
                // Apply the selection criteria based on flavor and tag
                if (((flv_str == "i") ||
                    (flv_str == "b" && abs(flv) == 5) ||
                    (flv_str == "c" && abs(flv) == 4) ||
                    (flv_str == "q" && abs(flv) <= 3 && flv != 0) ||
                    (flv_str == "s" && abs(flv) == 3) ||
                    (flv_str == "ud" && abs(flv) <= 2 && flv != 0) ||
                    (flv_str == "g" && flv == 21) ||
                    (flv_str == "n" && flv == 0)) &&
                    ((tag == "i") ||
                    (tag == "b" && isb) ||
                    (tag == "c" && isc) ||
                    (tag == "q" && isq) ||
                    (tag == "g" && isg) ||
                    (tag == "n" && isn))) {

                    // Determine the x-axis value based on the variable
                    double x = ptRef;
                    if (var == "p1Jet1PtOverGenJet1PtInGenJet1Pt" && genJetPt != 0) {
                        x = genJetPt;
                    }

                    // Retrieve the histogram from the map
                    TH1* h = varTagFlvMap_[var][tag][flv_str].get();
                    if (!h) {
                        std::cerr << "Error: Missing histogram for var: " << var << ", tag: " << tag 
                                  << ", flv: " << flv_str << std::endl;
                        assert(h);
                        continue; // Skip filling if histogram is missing
                    }

                    // Fill the histogram based on its type
                    if (var == "h1EventInRefPt" || var == "h1EventInGenJet1Pt") {
                        // Fill TH1D histograms
                        static_cast<TH1D*>(h)->Fill(x, weight);
                    } else {
                        // Fill TProfile histograms
                        static_cast<TProfile*>(h)->Fill(x, val, weight);
                    }
                }
            }//aflv_
        }//atag_
    }//mvar_
}

