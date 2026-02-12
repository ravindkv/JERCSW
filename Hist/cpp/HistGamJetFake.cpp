#include "HistGamJetFake.h"
#include "HelperDir.hpp"
#include "VarBin.h"

#include <iostream>
#include <cmath>
#include <vector>

// ROOT
#include <TH1D.h>
#include <TH2D.h>
#include <TProfile.h>
#include <TLorentzVector.h>

HistGamJetFake::~HistGamJetFake() = default;

HistGamJetFake::HistGamJetFake(TDirectory *origDir, const std::string& directoryName, const VarBin& varBin) {
    InitializeHistograms(origDir, directoryName, varBin);
}

static TH1D* makeH1(const char* name, int nb, double lo, double hi) {
    auto* h = new TH1D(name, "", nb, lo, hi);
    h->Sumw2();
    return h;
}
static TH2D* makeH2(const char* name, int nbx, double xlo, double xhi, int nby, double ylo, double yhi) {
    auto* h = new TH2D(name, "", nbx, xlo, xhi, nby, ylo, yhi);
    h->Sumw2();
    return h;
}
static TProfile* makeP(const char* name, int nb, const double* edges) {
    auto* p = new TProfile(name, "", nb, edges);
    p->Sumw2();
    return p;
}

void HistGamJetFake::InitializeHistograms(TDirectory *origDir, const std::string& directoryName, const VarBin& varBin)
{
    std::string dirName = directoryName + "/HistGamJetFake";
    TDirectory* newDir = HelperDir::createTDirectory(origDir, dirName);
    if (!newDir) {
        std::cerr << "Error: Failed to create directory: " << dirName << std::endl;
        return;
    }
    newDir->cd();

    // Var bins along GenJet pT
    std::vector<double> binsPt = varBin.getBinsPt();
    const int nPt = static_cast<int>(binsPt.size()) - 1;

    // --- Profiles vs GenJet pT (photon-based only)
    hist_.p1RecoPhoPtOverGenJetPtInGenJetPt    = std::unique_ptr<TProfile>(makeP("p1RecoPhoPtOverGenJetPtInGenJetPt",    nPt, binsPt.data()));
    hist_.p1RecoPhoPtOverGenPhoPtInGenJetPt = std::unique_ptr<TProfile>(makeP("p1RecoPhoPtOverGenPhoPtInGenJetPt", nPt, binsPt.data()));
    hist_.p1GenJetPtOverGenPhoPtInGenJetPt= std::unique_ptr<TProfile>(makeP("p1GenJetPtOverGenPhoPtInGenJetPt", nPt, binsPt.data()));
    hist_.p1GenJetPtOverGenPhoPtInRecoPhoPt= std::unique_ptr<TProfile>(makeP("p1GenJetPtOverGenPhoPtInRecoPhoPt", nPt, binsPt.data()));

    // --- Core 1D responses (photon-based)
    hist_.h1EventInRecoPhoPtOverGenPhoPt = std::unique_ptr<TH1D>(makeH1("h1EventInRecoPhoPtOverGenPhoPt",   120, 0.0, 2.0));
    hist_.h1EventInRecoPhoPtOverGenJetPt = std::unique_ptr<TH1D>(makeH1("h1EventInRecoPhoPtOverGenJetPt",   120, 0.0, 2.0));
    hist_.h1EventInGenJetPtOverGenPhoPt= std::unique_ptr<TH1D>(makeH1("h1EventInGenJetPtOverGenPhoPt",   120, 0.0, 2.0));

    // --- Angular diagnostics
    hist_.h1DRmatch   = std::unique_ptr<TH1D>(makeH1("h1DRmatch",    120, 0.0, 0.3));
    hist_.h1DEtaMatch = std::unique_ptr<TH1D>(makeH1("h1DEtaMatch",  120, -0.2, 0.2));
    hist_.h1DPhiMatch = std::unique_ptr<TH1D>(makeH1("h1DPhiMatch",  120, -0.3, 0.3));

    // --- Categories & daughters
    hist_.h1MatchTag  = std::unique_ptr<TH1D>(makeH1("h1MatchTag",   12, -0.5, 11.5)); // integer-coded
    hist_.h1NDauBest  = std::unique_ptr<TH1D>(makeH1("h1NDauBest",    6, -1.5, 4.5));  // -1,0,1,2,3,4(=≥4)

    // --- Profiles vs photon reco pT and R9 (fixed bins)
    hist_.p1RecoPhoPtOverGenPhoPtInRecoPhoPt = std::unique_ptr<TProfile>(new TProfile("p1RecoPhoPtOverGenPhoPtInRecoPhoPt","", 30, 0.0, 600.0));
    hist_.p1RecoPhoPtOverGenPhoPtInR9     = std::unique_ptr<TProfile>(new TProfile("p1RecoPhoPtOverGenPhoPtInR9","",    20, 0.0, 1.05));
    hist_.p1RecoPhoPtOverGenJetPtInRecoPhoPt = std::unique_ptr<TProfile>(new TProfile("p1RecoPhoPtOverGenJetPtInRecoPhoPt","",30, 0.0, 600.0));
    hist_.p1RecoPhoPtOverGenJetPtInR9     = std::unique_ptr<TProfile>(new TProfile("p1RecoPhoPtOverGenJetPtInR9","",   20, 0.0, 1.05));

    // --- Heatmaps to guide “magic numbers” (photon-based)
    hist_.h2PhoOverGenPhoVsRecoPt = std::unique_ptr<TH2D>(makeH2("h2PhoOverGenPhoVsRecoPt", 30, 0.0, 600.0, 60, 0.0, 2.0));
    hist_.h2PhoOverGenPhoVsR9     = std::unique_ptr<TH2D>(makeH2("h2PhoOverGenPhoVsR9",     20, 0.0, 1.05,  60, 0.0, 2.0));
    hist_.h2DRmatchVsRecoPt       = std::unique_ptr<TH2D>(makeH2("h2DRmatchVsRecoPt",       30, 0.0, 600.0, 60, 0.0, 0.3));
    hist_.h2RespVsEta             = std::unique_ptr<TH2D>(makeH2("h2RespVsEta",             24, 0.0, 2.4,   60, 0.0, 2.0)); // |eta|
    hist_.h2RespVsRcore           = std::unique_ptr<TH2D>(makeH2("h2RespVsRcore",           24, 0.0, 0.12,  60, 0.0, 2.0));
    hist_.h2RespVsRadapt          = std::unique_ptr<TH2D>(makeH2("h2RespVsRadapt",          24, 0.0, 0.20,  60, 0.0, 2.0));
    hist_.h2RespVsPhiMax          = std::unique_ptr<TH2D>(makeH2("h2RespVsPhiMax",          24, 0.0, 0.24,  60, 0.0, 2.0));
    hist_.h2RespVsEtaMax          = std::unique_ptr<TH2D>(makeH2("h2RespVsEtaMax",          24, 0.0, 0.10,  60, 0.0, 2.0));

    std::cout << "Initialized HistGamJetFake histograms in directory: " << dirName << std::endl;
    origDir->cd();
}

// -----------------------------
// Fill (jet args removed)
// -----------------------------
void HistGamJetFake::Fill(double ptGenJet, double ptRecoJet,
                          double ptGenPho, double ptRecoPho,
                          double weight)
{
    if (ptGenJet <= 0.0) return;

    // Profiles vs GenJet pT
    hist_.p1RecoPhoPtOverGenJetPtInGenJetPt->Fill(ptGenJet, ptRecoPho / ptGenJet, weight);
    if (ptGenPho > 0.0){
        hist_.p1RecoPhoPtOverGenPhoPtInGenJetPt->Fill(ptGenJet, ptRecoPho / ptGenPho, weight);
        hist_.p1GenJetPtOverGenPhoPtInGenJetPt->Fill(ptGenJet, ptGenJet / ptGenPho, weight);
        hist_.p1GenJetPtOverGenPhoPtInRecoPhoPt->Fill(ptRecoPho, ptGenJet / ptGenPho, weight);
    }

    // 1D responses
    if (ptGenPho > 0.0){
        hist_.h1EventInRecoPhoPtOverGenPhoPt->Fill(ptRecoPho / ptGenPho, weight);
        hist_.h1EventInGenJetPtOverGenPhoPt->Fill(ptGenJet / ptGenPho, weight);
    }
    hist_.h1EventInRecoPhoPtOverGenJetPt->Fill(ptRecoPho / ptGenJet, weight);
}

// -----------------------------
// Extended Fill (with monitoring payload)
// -----------------------------
void HistGamJetFake::Fill(double ptGenJet, double ptRecoJet,
                          double ptGenPho, double ptRecoPho,
                          const GamMatchMon& mon,
                          double weight)
{
    // Populate base plots
    Fill(ptGenJet, ptRecoJet, ptGenPho, ptRecoPho, weight);

    // Short-circuit if we lack genPho (avoid NaNs in photon/genPho)
    if (ptGenPho <= 0.0) return;

    // Pho-based responses we’ll reuse
    const double respPhoGenPho = ptRecoPho / ptGenPho;
    const double respPhoGenJet = (ptGenJet > 0.0 ? ptRecoPho / ptGenJet : 0.0);

    // Angular diagnostics (if provided)
    if (mon.dRmatch < 900.) {
        hist_.h1DRmatch->Fill(mon.dRmatch, weight);
        hist_.h1DEtaMatch->Fill(mon.dEta,  weight);
        hist_.h1DPhiMatch->Fill(mon.dPhi,  weight);
        if (mon.recoPt > 0.0)
            hist_.h2DRmatchVsRecoPt->Fill(mon.recoPt, mon.dRmatch, weight);
    }

    // Category & daughters
    hist_.h1MatchTag->Fill(static_cast<int>(mon.tag), weight);
    int nd = mon.nDauForBest;
    if (nd >= 3) nd = 3;
    hist_.h1NDauBest->Fill(nd, weight);

    // Profiles vs photon reco pT and R9
    if (mon.recoPt > 0.0) {
        hist_.p1RecoPhoPtOverGenPhoPtInRecoPhoPt->Fill(mon.recoPt, respPhoGenPho, weight);
        hist_.p1RecoPhoPtOverGenJetPtInRecoPhoPt->Fill(mon.recoPt, respPhoGenJet, weight);
        hist_.h2PhoOverGenPhoVsRecoPt->Fill(mon.recoPt, respPhoGenPho, weight);
    }
    if (mon.r9 >= 0.0) {
        hist_.p1RecoPhoPtOverGenPhoPtInR9->Fill(mon.r9, respPhoGenPho, weight);
        hist_.p1RecoPhoPtOverGenJetPtInR9->Fill(mon.r9, respPhoGenJet, weight);
        hist_.h2PhoOverGenPhoVsR9->Fill(mon.r9, respPhoGenPho, weight);
    }

    // |eta| trends (photon reco eta)
    hist_.h2RespVsEta->Fill(std::abs(mon.recoEta), respPhoGenPho, weight);

    // Magic-number tuners
    if (mon.R_core  > 0.0) hist_.h2RespVsRcore ->Fill(mon.R_core,  respPhoGenPho, weight);
    if (mon.R_adapt > 0.0) hist_.h2RespVsRadapt->Fill(mon.R_adapt, respPhoGenPho, weight);
    if (mon.phi_max > 0.0) hist_.h2RespVsPhiMax->Fill(mon.phi_max, respPhoGenPho, weight);
    if (mon.eta_max > 0.0) hist_.h2RespVsEtaMax->Fill(mon.eta_max, respPhoGenPho, weight);
}

