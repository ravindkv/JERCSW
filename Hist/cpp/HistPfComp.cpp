#include "HistPfComp.h"
#include "HelperDir.hpp"
#include "SkimTree.h"
#include "VarBin.h"

#include <TDirectory.h>
#include <TProfile.h>

#include <vector>
#include <iostream>

HistPfComp::~HistPfComp() = default;


HistPfComp::HistPfComp(TDirectory *origDir, const std::string& directoryName, const VarBin& varBin, const std::string& ofJet, const std::string& inObj)
    : ofJet_(ofJet),
    inObj_(inObj)
{
    InitializeHistograms(origDir, directoryName, varBin);
}

void HistPfComp::InitializeHistograms(TDirectory *origDir, const std::string& directoryName, const VarBin& varBin)
{

    // Use the HelperDir method to get or create the directory
    std::string dirName = directoryName + "/HistPfComp_"+ofJet_+inObj_;
    TDirectory* newDir = HelperDir::createTDirectory(origDir, dirName);
    if (!newDir) {
        std::cerr << "Error: Failed to create directory " << dirName << std::endl;
        return; // or handle the error appropriately
    }
    newDir->cd();

    const double* binsPt  = varBin.getBinsPt().data();
    const int nPt  = varBin.getBinsPt().size()  - 1;
    
    const double* binsEta  = varBin.getBinsEta().data();
    const int nEta  = varBin.getBinsEta().size()  - 1;

    // Response in Pt 
    hist_.p1ChfInPt = std::make_unique<TProfile>(("p1"+ ofJet_+"Chf"+inObj_+"Pt").c_str(), "", nPt, binsPt);
    hist_.p1NhfInPt = std::make_unique<TProfile>(("p1"+ ofJet_+"Nhf"+inObj_+"Pt").c_str(), "", nPt, binsPt);
    hist_.p1NefInPt = std::make_unique<TProfile>(("p1"+ ofJet_+"Nef"+inObj_+"Pt").c_str(), "", nPt, binsPt);
    hist_.p1CefInPt = std::make_unique<TProfile>(("p1"+ ofJet_+"Cef"+inObj_+"Pt").c_str(), "", nPt, binsPt);
    hist_.p1MufInPt = std::make_unique<TProfile>(("p1"+ ofJet_+"Muf"+inObj_+"Pt").c_str(), "", nPt, binsPt);
    
    // Response in Eta 
    hist_.p1ChfInEta = std::make_unique<TProfile>(("p1"+ofJet_+"Chf"+inObj_+"Eta").c_str(), "", nEta, binsEta);
    hist_.p1NhfInEta = std::make_unique<TProfile>(("p1"+ofJet_+"Nhf"+inObj_+"Eta").c_str(), "", nEta, binsEta);
    hist_.p1NefInEta = std::make_unique<TProfile>(("p1"+ofJet_+"Nef"+inObj_+"Eta").c_str(), "", nEta, binsEta);
    hist_.p1CefInEta = std::make_unique<TProfile>(("p1"+ofJet_+"Cef"+inObj_+"Eta").c_str(), "", nEta, binsEta);
    hist_.p1MufInEta = std::make_unique<TProfile>(("p1"+ofJet_+"Muf"+inObj_+"Eta").c_str(), "", nEta, binsEta);

    std::cout << "Initialized HistPfComp histograms in directory: " << dirName << std::endl;
    origDir->cd();
}

void HistPfComp::Fill(SkimTree* skimT, int iOfJet, double inObjPt, double inObjEta, double weight)
{
    if (!skimT) {
    std::cerr << "Error: Invalid SkimTree pointer passed to HistPfComp::Fill." << std::endl;
    return;
    }
    double chHEF  =  skimT->Jet_chHEF[iOfJet];
    double neHEF  =  skimT->Jet_neHEF[iOfJet];
    double neEmEF =  skimT->Jet_neEmEF[iOfJet];
    double chEmEF =  skimT->Jet_chEmEF[iOfJet];
    double muEF   =  skimT->Jet_muEF[iOfJet];
    
    hist_.p1ChfInPt->Fill(inObjPt, chHEF, weight);
    hist_.p1NhfInPt->Fill(inObjPt, neHEF, weight);
    hist_.p1NefInPt->Fill(inObjPt, neEmEF, weight);
    hist_.p1CefInPt->Fill(inObjPt, chEmEF, weight);
    hist_.p1MufInPt->Fill(inObjPt, muEF, weight);

    hist_.p1ChfInEta->Fill(inObjEta, chHEF, weight);
    hist_.p1NhfInEta->Fill(inObjEta, neHEF, weight);
    hist_.p1NefInEta->Fill(inObjEta, neEmEF, weight);
    hist_.p1CefInEta->Fill(inObjEta, chEmEF, weight);
    hist_.p1MufInEta->Fill(inObjEta, muEF, weight);
}

