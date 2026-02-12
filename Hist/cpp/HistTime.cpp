#include "HistTime.h"
#include "HelperDir.hpp"

// full definitions for our forward‑declared types
#include "VarBin.h"
#include "SkimTree.h"

#include <iostream>   // std::cout, std::cerr
#include <string>     // std::to_string

// ROOT
#include <TH1D.h>
#include <TProfile.h>
#include <TDirectory.h>


HistTime::~HistTime() = default;

HistTime::HistTime(TDirectory *origDir,  const std::string& directoryName, const VarBin& varBin, const std::vector<int>& pTProbes)
    : pTProbes_(pTProbes), runN_(200), runMin_(271030), runMax_(325200)
{
    InitializeHistograms(origDir, directoryName, varBin);
}

void HistTime::InitializeHistograms(TDirectory *origDir, const std::string& directoryName, const VarBin& varBin) {

    // Use the HelperDir method to get or create the directory
    std::string dirName = directoryName + "/HistTime";
    TDirectory* newDir = HelperDir::createTDirectory(origDir, dirName);
    newDir->cd();

    std::vector<double> rangeRun  = varBin.getRangeRun();
    runN_ = rangeRun.at(0);
    runMin_ = rangeRun.at(1);
    runMax_ = rangeRun.at(2);

    std::cout<<"HistTime: Run range = "<<runMin_<<", "<<runMax_<<'\n';
    
    // Initialize histograms for each pTProbe threshold
    for (const auto& pTProbe : pTProbes_) {
        // Create RunHistograms struct
        RunHistograms rh;
        
        // Define histogram suffix based on pTProbe
        std::string suffix = "ProbePt" + std::to_string(pTProbe);
        
        // Initialize TH1D histograms
        std::string h1Name = "h1EventInRunFor" + suffix;
        rh.h1EventInRun = std::make_unique<TH1D>(
            h1Name.c_str(),
            ";Run Number;Events",
            runN_,
            runMin_,
            runMax_
        );
        
        // Initialize TProfile histograms
        std::string p1DbRespName = "p1DbRespInRunFor" + suffix;
        rh.p1DbRespInRun = std::make_unique<TProfile>(
            p1DbRespName.c_str(),
            ";Run Number; DB Balance",
            runN_,
            runMin_,
            runMax_
        );
        
        std::string p1MpfRespName = "p1MpfRespInRunFor" + suffix;
        rh.p1MpfRespInRun = std::make_unique<TProfile>(
            p1MpfRespName.c_str(),
            ";Run Number; MPF Response",
            runN_,
            runMin_,
            runMax_
        );
        
        std::string p1Jet1ChfName = "p1Jet1ChfInRunFor" + suffix;
        rh.p1Jet1ChfInRun = std::make_unique<TProfile>(
            p1Jet1ChfName.c_str(),
            ";Run Number;Jet chHEF",
            runN_,
            runMin_,
            runMax_
        );
        
        std::string p1Jet1NhfName = "p1Jet1NhfInRunFor" + suffix;
        rh.p1Jet1NhfInRun = std::make_unique<TProfile>(
            p1Jet1NhfName.c_str(),
            ";Run Number;Jet neHEF",
            runN_,
            runMin_,
            runMax_
        );
        
        std::string p1Jet1NefName = "p1Jet1NefInRunFor" + suffix;
        rh.p1Jet1NefInRun = std::make_unique<TProfile>(
            p1Jet1NefName.c_str(),
            ";Run Number;Jet neEmEF",
            runN_,
            runMin_,
            runMax_
        );

        std::string p1Jet1CefName = "p1Jet1CefInRunFor" + suffix;
        rh.p1Jet1CefInRun = std::make_unique<TProfile>(
            p1Jet1CefName.c_str(),
            ";Run Number;Jet chEmEF",
            runN_,
            runMin_,
            runMax_
        );

        std::string p1Jet1MufName = "p1Jet1MufInRunFor" + suffix;
        rh.p1Jet1MufInRun = std::make_unique<TProfile>(
            p1Jet1MufName.c_str(),
            ";Run Number;Jet muEF",
            runN_,
            runMin_,
            runMax_
        );
        
        // Add to histMap_
        histMap_[pTProbe] = std::move(rh);
        
        // Optionally, print initialization confirmation
        std::cout << "Initialized HistTime histograms for pTProbe" << pTProbe <<" in "<<dirName << std::endl;
    }
    origDir->cd();
}

void HistTime::Fill(SkimTree* skimT, int iJet1, double bal, double mpf, double ptProbe, double weight) {
    // Loop over each pTProbe threshold
    for (const auto& pTProbe : pTProbes_) {
        if (ptProbe > pTProbe) {
            // Retrieve the corresponding RunHistograms
            auto it = histMap_.find(pTProbe);
            if (it == histMap_.end()) {
                std::cerr << "Error: pTProbe " << pTProbe << " not found in histMap_." << std::endl;
                continue;
            }
            RunHistograms& rh = it->second;
            
            // Fill h1EventInRun
            rh.h1EventInRun->Fill(skimT->run, weight);
            
            // Fill p1DbRespInRun
            rh.p1DbRespInRun->Fill(skimT->run, bal, weight);
            
            // Fill p1MpfRespInRun
            rh.p1MpfRespInRun->Fill(skimT->run, mpf, weight);
            
            // Fill p1Jet1ChfInRun
            rh.p1Jet1ChfInRun->Fill(skimT->run, skimT->Jet_chHEF[iJet1], weight);
            
            // Fill p1Jet1NhfInRun
            rh.p1Jet1NhfInRun->Fill(skimT->run, skimT->Jet_neHEF[iJet1], weight);
            
            // Fill p1Jet1NefInRun
            rh.p1Jet1NefInRun->Fill(skimT->run, skimT->Jet_neEmEF[iJet1], weight);

            // Fill p1Jet1CefInRun
            rh.p1Jet1CefInRun->Fill(skimT->run, skimT->Jet_chEmEF[iJet1], weight);

            // Fill p1Jet1MufInRun
            rh.p1Jet1MufInRun->Fill(skimT->run, skimT->Jet_muEF[iJet1], weight);
        }
    }
}

