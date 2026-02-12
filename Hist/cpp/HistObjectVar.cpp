#include "HistObjectVar.h"
#include "HelperDir.hpp"
#include "VarBin.h"
#include "SkimTree.h"

#include <TDirectory.h>
#include <TH2D.h>
#include <TH1D.h>

#include <vector>
#include <iostream>

HistObjectVar::~HistObjectVar() = default;

/**
 * @brief Constructs a HistObjectVar object and initializes histograms.
 * 
 * @param origDir Pointer to the original ROOT directory.
 * @param directoryName Name of the directory within the ROOT file to store histograms.
 * @param varBin Object defining variable binning.
 */
HistObjectVar::HistObjectVar(TDirectory* origDir, const std::string& directoryName, const VarBin& varBin)
{
    InitializeHistograms(origDir, directoryName, varBin);
}

/**
 * @brief Initializes all histograms and sets up the internal structures.
 * 
 * @param origDir Pointer to the original ROOT directory.
 * @param directoryName Name of the directory within the ROOT file to store histograms.
 * @param varBin Object defining variable binning.
 */
void HistObjectVar::InitializeHistograms(TDirectory* origDir, const std::string& directoryName, const VarBin& varBin)
{
    // Create a subdirectory for HistObjectVar histograms
    std::string dirName = directoryName + "/HistObjectVar";
    TDirectory* newDir = HelperDir::createTDirectory(origDir, dirName);
    newDir->cd();

    // Define binning from VarBin
    const std::vector<double> binsPt  = varBin.getBinsPt();
    const std::vector<double> binsEta  = varBin.getBinsEta();
    const std::vector<double> binsPhi  = varBin.getBinsPhi();
    const int nPt   = binsPt.size()  - 1;
    const int nEta  = binsEta.size()  - 1;
    const int nPhi  = binsPhi.size()  - 1;

    std::vector<double> rangeCountRef = varBin.getRangeCountRef();
    int countN   = rangeCountRef.at(0);
    double countMin = rangeCountRef.at(1);
    double countMax = rangeCountRef.at(2);

    std::vector<double> rangeR9 = varBin.getRangeResponse();
    int r9N   = rangeR9.at(0);
    double r9Min = rangeR9.at(1);
    double r9Max = rangeR9.at(2);

    //Ele
    histEle_.h2EventInElePtEleEta = std::make_unique<TH2D>(
        "h2EventInElePtEleEta", "", nEta, binsEta.data(), nPt, binsPt.data());

    //Photon
    histPho_.h2EventInPhoEtaPhoPt = std::make_unique<TH2D>(
        "h2EventInPhoEtaPhoPt", "", nPt, binsPt.data(), nEta, binsEta.data());
    histPho_.h2EventInPhoCountPhoPt = std::make_unique<TH2D>(
        "h2EventInPhoCountPhoPt", "", nPt, binsPt.data(), countN, countMin, countMax);
    histPho_.h2EventInPhoR9PhoPt = std::make_unique<TH2D>(
        "h2EventInPhoR9PhoPt", "", nPt, binsPt.data(), r9N, r9Min, r9Max);
    histPho_.h2EventInPhoHoePhoPt = std::make_unique<TH2D>(
        "h2EventInPhoHoePhoPt", "", nPt, binsPt.data(), r9N, r9Min, r9Max/100);
    histPho_.h2EventInPhoIdPhoPt = std::make_unique<TH2D>(
        "h2EventInPhoIdPhoPt", "", nPt, binsPt.data(), countN, countMin, countMax);

    //Muon
    histMu_.h2EventInMuPtMuEta = std::make_unique<TH2D>(
        "h2EventInMuPtMuEta", "", nEta, binsEta.data(), nPt, binsPt.data());

    //Energy fraction (ef)
    std::vector<double> rangeEf = varBin.getRangeFraction();
    int efN   = rangeEf.at(0);
    double efMin = rangeEf.at(1);
    double efMax = rangeEf.at(2);
    //Jet
    histJet_.h2EventInJetPtJetEta = std::make_unique<TH2D>(
        "h2EventInJetPtJetEta", "", nEta, binsEta.data(), nPt, binsPt.data());
    //All jets
    histJet_.h1EventInJetChHef = std::make_unique<TH1D>("h1EventInJetChHef", "", efN, efMin, efMax);
    histJet_.h1EventInJetNeHef = std::make_unique<TH1D>("h1EventInJetNeHef", "", efN, efMin, efMax);
    histJet_.h1EventInJetNeEmef = std::make_unique<TH1D>("h1EventInJetNeEmef", "", efN, efMin, efMax);
    histJet_.h1EventInJetChEmef = std::make_unique<TH1D>("h1EventInJetChEmef", "", efN, efMin, efMax);
    histJet_.h1EventInJetMuef = std::make_unique<TH1D>("h1EventInJetMuef", "", efN, efMin, efMax);

    //Leading jet
    histJet_.h2EventInJet1PtJetEta = std::make_unique<TH2D>(
        "h2EventInJet1PtJetEta", "", nEta, binsEta.data(), nPt, binsPt.data());
    histJet_.h1EventInJet1ChHef = std::make_unique<TH1D>("h1EventInJet1ChHef", "", efN, efMin, efMax);
    histJet_.h1EventInJet1NeHef = std::make_unique<TH1D>("h1EventInJet1NeHef", "", efN, efMin, efMax);
    histJet_.h1EventInJet1NeEmef = std::make_unique<TH1D>("h1EventInJet1NeEmef", "", efN, efMin, efMax);
    histJet_.h1EventInJet1ChEmef = std::make_unique<TH1D>("h1EventInJet1ChEmef", "", efN, efMin, efMax);
    histJet_.h1EventInJet1Muef = std::make_unique<TH1D>("h1EventInJet1Muef", "", efN, efMin, efMax);

    //Met
    histMet_.h2EventInMetPtMetPhi = std::make_unique<TH2D>(
        "h2EventInMetPtMetPhi", "", nPhi, binsPhi.data(), nPt, binsPt.data());

    std::cout << "Initialized HistObjectVar histograms in directory: " << dirName << std::endl;
    origDir->cd();
}


// Fill
void HistObjectVar::FillElectron(const SkimTree& skimT, double weight)
{
    for (int ind = 0; ind < skimT.nElectron; ++ind) {
        double pt = skimT.Electron_pt[ind];
        double eta = skimT.Electron_eta[ind];
        histEle_.h2EventInElePtEleEta->Fill(eta, pt, weight);
    }
}

void HistObjectVar::FillPhoton(const SkimTree& skimT, double weight)
{
    for (int ind = 0; ind < skimT.nPhoton; ++ind) {
        double pt = skimT.Photon_pt[ind];
        double eta = skimT.Photon_eta[ind];
        histPho_.h2EventInPhoEtaPhoPt->Fill(pt, eta, weight);
        histPho_.h2EventInPhoCountPhoPt->Fill(pt, skimT.nPhoton, weight);
        histPho_.h2EventInPhoR9PhoPt->Fill(pt, skimT.Photon_r9[ind], weight);
        histPho_.h2EventInPhoHoePhoPt->Fill(pt, skimT.Photon_hoe[ind], weight);
        histPho_.h2EventInPhoIdPhoPt->Fill(pt, skimT.Photon_cutBased[ind], weight);
    }
}

void HistObjectVar::FillMuon(const SkimTree& skimT, double weight)
{
    for (int ind = 0; ind < skimT.nMuon; ++ind) {
        histMu_.h2EventInMuPtMuEta->Fill(skimT.Muon_eta[ind], skimT.Muon_pt[ind], weight);
    }
}

void HistObjectVar::FillJet(const SkimTree& skimT, double weight, int iJet1)
{
    for (int ind = 0; ind < skimT.nJet; ++ind) {
        histJet_.h2EventInJetPtJetEta->Fill(skimT.Jet_eta[ind], skimT.Jet_pt[ind], weight);
        histJet_.h1EventInJetChHef->Fill(skimT.Jet_chHEF[ind]  , weight);
        histJet_.h1EventInJetNeHef->Fill(skimT.Jet_neHEF[ind]  , weight);
        histJet_.h1EventInJetNeEmef->Fill(skimT.Jet_neEmEF[ind] , weight);
        histJet_.h1EventInJetChEmef->Fill(skimT.Jet_chEmEF[ind] , weight);
        histJet_.h1EventInJetMuef->Fill(skimT.Jet_muEF[ind]   , weight);
    }
    if(skimT.nJet >= iJet1 +1 && iJet1 != -1 ){
        histJet_.h2EventInJet1PtJetEta->Fill(skimT.Jet_eta[iJet1], skimT.Jet_pt[iJet1], weight);
        histJet_.h1EventInJet1ChHef->Fill(skimT.Jet_chHEF[iJet1]  , weight);
        histJet_.h1EventInJet1NeHef->Fill(skimT.Jet_neHEF[iJet1]  , weight);
        histJet_.h1EventInJet1NeEmef->Fill(skimT.Jet_neEmEF[iJet1] , weight);
        histJet_.h1EventInJet1ChEmef->Fill(skimT.Jet_chEmEF[iJet1] , weight);
        histJet_.h1EventInJet1Muef->Fill(skimT.Jet_muEF[iJet1]   , weight);
    }
}

void HistObjectVar::FillMet(const SkimTree& skimT, double weight)
{
    histMet_.h2EventInMetPtMetPhi->Fill(skimT.MET_phi, skimT.MET_pt, weight);
}
