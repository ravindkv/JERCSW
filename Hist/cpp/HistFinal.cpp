#include "HistFinal.h"
#include "HelperDir.hpp"
#include "VarBin.h"        // full VarBin definition

#include <TDirectory.h>
#include <TH2D.h>
#include <TProfile.h>
#include <TLorentzVector.h>

#include <vector>
#include <iostream>        // for std::cerr, std::cout
#include <cmath>           // for std::fabs

// out‑of‑line destructor so the header can keep ROOT types opaque
HistFinal::~HistFinal() = default;


HistFinal::HistFinal(TDirectory* origDir, const std::string& directoryName, const VarBin& varBin)
{
    InitializeHistograms(origDir, directoryName, varBin);
}

void HistFinal::InitializeHistograms(TDirectory* origDir, const std::string& directoryName, const VarBin& varBin)
{
    // Use the HelperDir method to get or create the directory
    std::string dirName = directoryName + "/HistFinal";
    TDirectory* newDir = HelperDir::createTDirectory(origDir, dirName);
    if (!newDir) {
        std::cerr << "Error: Failed to create directory " << dirName << std::endl;
        return; // Or handle the error appropriately
    }
    newDir->cd();

    // Retrieve binning configurations
    std::vector<double> binsPt  = varBin.getBinsPt();
    const int nPt  = binsPt.size() - 1;
    std::vector<double> rangeResp  = varBin.getRangeResponse();
    int respN = rangeResp.at(0);
    int respMin = rangeResp.at(1);
    int respMax = rangeResp.at(2);

    // Initialize 2D Histograms
    hist_.h2EventInDbRespRefPt = std::make_unique<TH2D>(
        "h2EventInDbRespRefPt", "", nPt, binsPt.data(), respN, respMin, respMax);

    hist_.h2EventInMpfRespRefPt = std::make_unique<TH2D>(
        "h2EventInMpfRespRefPt", "", nPt, binsPt.data(), respN, respMin, respMax);

    // Initialize Additional TProfile Histograms
    hist_.p1GenJet2PtOverRefPtInRefPt = std::make_unique<TProfile>(
        "p1GenJet2PtOverRefPtInRefPt", "", nPt, binsPt.data());

    hist_.p1Jet2PtOverRefPtInRefPt = std::make_unique<TProfile>(
        "p1Jet2PtOverRefPtInRefPt", "", nPt, binsPt.data());

    hist_.p1Jet2PtOverGenJet2PtInGenJet2Pt = std::make_unique<TProfile>(
        "p1Jet2PtOverGenJet2PtInGenJet2Pt", "", nPt, binsPt.data());

    // Initialize Conditional 2D Histograms
    hist_.h2EventInDbRespRefPtForPassMpf = std::make_unique<TH2D>(
        "h2EventInDbRespRefPtForPassMpf", "", nPt, binsPt.data(), respN, respMin, respMax);

    hist_.h2EventInMpfRespRefPtForPassDb = std::make_unique<TH2D>(
        "h2EventInMpfRespRefPtForPassDb", "", nPt, binsPt.data(), respN, respMin, respMax);

    hist_.h2EventInDbRespRefPtForPassBoth = std::make_unique<TH2D>(
        "h2EventInDbRespRefPtForPassBoth", "", nPt, binsPt.data(), respN, respMin, respMax);

    hist_.h2EventInMpfRespRefPtForPassBoth = std::make_unique<TH2D>(
        "h2EventInMpfRespRefPtForPassBoth", "", nPt, binsPt.data(), respN, respMin, respMax);

    std::cout << "Initialized HistFinal histograms in directory: " << dirName << std::endl;

    // Return to the original directory
    origDir->cd();
}

void HistFinal::Fill(double ptRef, double bal, double mpf,
                     const TLorentzVector& p4Jet2, const TLorentzVector& p4GenJet2,
                     int iGenJet2, bool isMC, double weight)
{
    // Fill 2D Histograms
    hist_.h2EventInDbRespRefPt->Fill(ptRef, bal, weight);
    hist_.h2EventInMpfRespRefPt->Fill(ptRef, mpf, weight);

    // Fill TProfile Histograms based on jet conditions
    if (p4Jet2.Pt() > 0) {
        if (iGenJet2 != -1 || !isMC) {
            hist_.p1Jet2PtOverRefPtInRefPt->Fill(ptRef, p4Jet2.Pt() / ptRef, weight);
        }
        if (iGenJet2 != -1) {
            hist_.p1GenJet2PtOverRefPtInRefPt->Fill(ptRef, p4GenJet2.Pt() / ptRef, weight);
            hist_.p1Jet2PtOverGenJet2PtInGenJet2Pt->Fill(p4GenJet2.Pt(), p4Jet2.Pt() / p4GenJet2.Pt(), weight);
        }
    }

    // Define pass criteria
    bool passDbResp = (fabs(1.0 - bal) < 0.7);
    bool passMpfResp = (fabs(1.0 - mpf) < 0.7);

    // Conditional filling based on pass criteria
    if (passMpfResp) {
        hist_.h2EventInDbRespRefPtForPassMpf->Fill(ptRef, bal, weight);
    }
    if (passDbResp) {
        hist_.h2EventInMpfRespRefPtForPassDb->Fill(ptRef, mpf, weight);
    }
    if (passMpfResp && passDbResp) {
        hist_.h2EventInDbRespRefPtForPassBoth->Fill(ptRef, bal, weight);
        hist_.h2EventInMpfRespRefPtForPassBoth->Fill(ptRef, mpf, weight);
    }
}

void HistFinal::FillCommon(double ptRef, double bal, double mpf, bool passDbResp, bool passMpfResp, double weight)
{
    // Fill 2D Histograms
    hist_.h2EventInDbRespRefPt->Fill(ptRef, bal, weight);
    hist_.h2EventInMpfRespRefPt->Fill(ptRef, mpf, weight);

    // Conditional filling based on pass criteria
    if (passMpfResp) {
        hist_.h2EventInDbRespRefPtForPassMpf->Fill(ptRef, bal, weight);
    }
    if (passDbResp) {
        hist_.h2EventInMpfRespRefPtForPassDb->Fill(ptRef, mpf, weight);
    }
    if (passMpfResp && passDbResp) {
        hist_.h2EventInDbRespRefPtForPassBoth->Fill(ptRef, bal, weight);
        hist_.h2EventInMpfRespRefPtForPassBoth->Fill(ptRef, mpf, weight);
    }
}

void HistFinal::FillGen(double ptRef, const TLorentzVector& p4Jet2, const TLorentzVector& p4GenJet2, double weight)
{
    // Fill TProfile histograms based on jet conditions.
    if (p4Jet2.Pt() > 0) {
        if(ptRef > 0){
            hist_.p1Jet2PtOverRefPtInRefPt->Fill(ptRef, p4Jet2.Pt() / ptRef, weight);
            hist_.p1GenJet2PtOverRefPtInRefPt->Fill(ptRef, p4GenJet2.Pt() / ptRef, weight);
        }
        if (p4GenJet2.Pt() > 0) {
            hist_.p1Jet2PtOverGenJet2PtInGenJet2Pt->Fill(p4GenJet2.Pt(), p4Jet2.Pt() / p4GenJet2.Pt(), weight);
        }
    }
}
