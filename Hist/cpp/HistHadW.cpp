#include "HistHadW.h"
#include "HelperDir.hpp"
#include "VarBin.h"            // full definition of VarBin

#include <TDirectory.h>
#include <TH1D.h>
#include <TProfile.h>
#include <TLorentzVector.h>

#include <vector>
#include <iostream>

// out‑of‑line destructor requires full types
HistHadW::~HistHadW() = default;


HistHadW::HistHadW(TDirectory *origDir, const std::string& directoryName, const VarBin& varBin)
{
    InitializeHistograms(origDir, directoryName, varBin);
}

void HistHadW::InitializeHistograms(TDirectory *origDir, const std::string& directoryName, const VarBin& varBin)
{
    // Use the HelperDir method to get or create the directory
    std::string dirName = directoryName + "/HistHadW";
    TDirectory* newDir = HelperDir::createTDirectory(origDir, dirName);
    newDir->cd();

    std::vector<double> binsPt  = varBin.getBinsPt();
    std::vector<double> binsEta = varBin.getBinsEta();
    std::vector<double> binsPhi = varBin.getBinsPhi();
    std::vector<double> binsMass = varBin.getBinsMass();

    std::vector<double> rangeMassTop = varBin.getRangeMassTop();
    const int nMassTop= rangeMassTop.at(0); 
    const int minMassTop= rangeMassTop.at(1); 
    const int maxMassTop= rangeMassTop.at(2); 

    const int nPt  = binsPt.size()  - 1;
    const int nEta = binsEta.size() - 1;
    const int nPhi = binsPhi.size() - 1;
    const int nMass= binsMass.size() - 1;

    std::vector<double> rangeChiSqr = varBin.getRangeChiSqr();
    const int nChiSqr= rangeChiSqr.at(0); 
    const int minChiSqr= rangeChiSqr.at(1); 
    const int maxChiSqr= rangeChiSqr.at(2); 

    std::vector<double> rangeRun  = varBin.getRangeRun();
    const int runN = rangeRun.at(0);
    const int runMin = rangeRun.at(1);
    const int runMax = rangeRun.at(2);

    // Initialize 1D Histograms
    hist_.h1EventInHadWPt = std::make_unique<TH1D>("h1EventInHadWPt", "", nPt, binsPt.data());
    hist_.h1EventInAvgHadPt = std::make_unique<TH1D>("h1EventInAvgHadPt", "", nPt, binsPt.data());
    hist_.h1EventInHadWEta = std::make_unique<TH1D>("h1EventInHadWEta", "", nEta, binsEta.data());
    hist_.h1EventInHadWPhi = std::make_unique<TH1D>("h1EventInHadWPhi", "", nPhi, binsPhi.data());
    hist_.h1EventInHadWMass = std::make_unique<TH1D>("h1EventInHadWMass", "", nMassTop, minMassTop, maxMassTop);
    hist_.h1EventInChiSqrW = std::make_unique<TH1D>("h1EventInChiSqrW", "", nChiSqr, minChiSqr, maxChiSqr);

    
    // Initialize TProfile Histograms
    hist_.p1HadWMassInHadWPt = std::make_unique<TProfile>(
        "p1HadWMassInHadWPt", "", nPt, binsPt.data());
    hist_.p1HadWMassInAvgHadPt = std::make_unique<TProfile>(
        "p1HadWMassInAvgHadPt", "", nPt, binsPt.data());
    hist_.p1HadWMassInHadWEta = std::make_unique<TProfile>(
        "p1HadWMassInHadWEta", "", nEta, binsEta.data());
    hist_.p1HadWMassInHadWPhi = std::make_unique<TProfile>(
        "p1HadWMassInHadWPhi", "", nPhi, binsPhi.data());
    hist_.p1HadWMassInRun = std::make_unique<TProfile>(
        "p1HadWMassInRun", "", runN, runMin, runMax);
    
    std::cout << "Initialized HistHadW histograms in directory: " << dirName << std::endl;
    origDir->cd();
}

void HistHadW::Fill(const TLorentzVector& p4HadW, double avgHadPt, double chiSqr, int run,  double weight)
{
    double wPt    = p4HadW.Pt();
    double wEta   = p4HadW.Eta();
    double wPhi   = p4HadW.Phi();
    double wMass  = p4HadW.M();

    // Fill h1EventInHadWPt
    hist_.h1EventInHadWPt->Fill(wPt, weight);
    hist_.h1EventInAvgHadPt->Fill(avgHadPt, weight);
    hist_.h1EventInHadWEta->Fill(wEta, weight);
    hist_.h1EventInHadWPhi->Fill(wPhi, weight);
    hist_.h1EventInHadWMass->Fill(wMass, weight);
    hist_.h1EventInChiSqrW->Fill(chiSqr, weight);

    hist_.p1HadWMassInAvgHadPt->Fill(avgHadPt, wMass, weight);
    hist_.p1HadWMassInHadWPt->Fill(wPt, wMass, weight);
    hist_.p1HadWMassInHadWEta->Fill(wEta, wMass, weight);
    hist_.p1HadWMassInHadWPhi->Fill(wPhi, wMass, weight);
    hist_.p1HadWMassInRun->Fill(run, wMass, weight);
    
}
