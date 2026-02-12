#include "HistMultiJet.h"
#include "HelperDir.hpp"
#include "HelperDelta.hpp"
#include "VarBin.h"
#include "SkimTree.h"

#include <TDirectory.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TProfile.h>

#include <vector>
#include <iostream>
#include <cmath>

// out‑of‑line destructor (empty)
HistMultiJet::~HistMultiJet() = default;

// Constructor
HistMultiJet::HistMultiJet(TDirectory *origDir, const std::string& directoryName, const VarBin& varBin) {
    InitializeHistograms(origDir, directoryName, varBin);
}

// Method to initialize histograms
void HistMultiJet::InitializeHistograms(TDirectory *origDir, const std::string& directoryName, const VarBin& varBin) {
    // Use the HelperDir method to get or create the directory
    std::string dirName = directoryName + "/HistMultiJet";
    TDirectory* newDir = HelperDir::createTDirectory(origDir, dirName);
    newDir->cd();

    std::vector<double> binsPt  = varBin.getBinsPt();
    int nPt = binsPt.size() - 1;
    std::vector<double> binsEta  = varBin.getBinsEta();
    int nEta = binsEta.size() - 1;
    // Initialize histograms and profiles
    h1EventInCrecoil   = new TH1D("h1EventInCrecoil", "", 100, 0.0, 1.0);
    p1CrecoilInProbePt = new TProfile("p1CrecoilInProbePt", "", nPt, binsPt.data());

    h1EventInRecoilPt = new TH1D("h1EventInRecoilPt", "", nPt, binsPt.data());
    h1EventInProbePt = new TH1D("h1EventInProbePt", "", nPt, binsPt.data());
    h1EventInMetPt = new TH1D("h1EventInMetPt", "", nPt, binsPt.data());
    h1EventInOtherPt = new TH1D("h1EventInOtherPt", "", nPt, binsPt.data());
    h1EventInUnclusteredPt = new TH1D("h1EventInUnclusteredPt", "", nPt, binsPt.data());
    h1EventInAvgProjPt = new TH1D("h1EventInAvgProjPt", "", nPt, binsPt.data());
    h1EventInAvgPt = new TH1D("h1EventInAvgPt", "", nPt, binsPt.data());
    h1EventInCosDeltaPhiProbeRecoil = new TH1D("h1EventInCosDeltaPhiProbeRecoil", "", 102, -1.01, 1.01);

    // responses: choose a safe generic range (adjust later if you add VarBin ranges)
    constexpr int    nRespBins = 50;
    constexpr double rMin   = 0.0;
    constexpr double rMax   = 2.0;
    h1EventInMjbResp = new TH1D("h1EventInMjbResp", "", nRespBins, rMin, rMax);
    h1EventInMpfResp = new TH1D("h1EventInMpfResp", "", nRespBins, rMin, rMax);
    h1EventInRespOther = new TH1D("h1EventInRespOther", "", nRespBins, rMin, rMax);
    h1EventInRespUnclustered = new TH1D("h1EventInRespUnclustered","", nRespBins, rMin, rMax);

    p1RecoilPtInProbePt      = new TProfile("p1RecoilPtInProbePt", "", nPt, binsPt.data());
    p1ProbePtInProbePt        = new TProfile("p1ProbePtInProbePt",   "", nPt, binsPt.data());
    p1MetPtInProbePt         = new TProfile("p1MetPtInProbePt",    "", nPt, binsPt.data());
    p1OtherPtInProbePt       = new TProfile("p1OtherPtInProbePt",  "", nPt, binsPt.data());
    p1UnclusteredPtInProbePt = new TProfile("p1UnclusteredPtInProbePt", "", nPt, binsPt.data());
    p1ProbePtInAvgProjPt = new TProfile("p1ProbePtInAvgProjPt", "", nPt, binsPt.data());
    p1ProbePtInAvgPt = new TProfile("p1ProbePtInAvgPt", "", nPt, binsPt.data());
    p1CrecoilInAvgPt = new TProfile("p1CrecoilInAvgPt", "", nPt, binsPt.data());

    p1MjbRespInAvgProjPt = new TProfile("p1MjbRespInAvgProjPt", "", nPt, binsPt.data());
    p1CrecoilInAvgProjPt = new TProfile("p1CrecoilInAvgProjPt", "", nPt, binsPt.data());

    p1MpfRespInAvgProjPt = new TProfile("p1MpfRespInAvgProjPt", "", nPt, binsPt.data());
    p1RespSumProbeRecoilInAvgProjPt = new TProfile("p1RespSumProbeRecoilInAvgProjPt", "", nPt, binsPt.data());
    p1RespNegProbeInAvgProjPt = new TProfile("p1RespNegProbeInAvgProjPt", "", nPt, binsPt.data());
    p1RespRecoilInAvgProjPt = new TProfile("p1RespRecoilInAvgProjPt", "", nPt, binsPt.data());
    p1RespOtherInAvgProjPt = new TProfile("p1RespOtherInAvgProjPt", "", nPt, binsPt.data());
    p1RespUnclusteredInAvgProjPt = new TProfile("p1RespUnclusteredInAvgProjPt", "", nPt, binsPt.data());

    p1MjbRespInAvgPt = new TProfile("p1MjbRespInAvgPt", "", nPt, binsPt.data());

    p1MpfRespInAvgPt = new TProfile("p1MpfRespInAvgPt", "", nPt, binsPt.data());
    p1RespSumProbeRecoilInAvgPt = new TProfile("p1RespSumProbeRecoilInAvgPt", "", nPt, binsPt.data());
    p1RespNegProbeInAvgPt = new TProfile("p1RespNegProbeInAvgPt", "", nPt, binsPt.data());
    p1RespRecoilInAvgPt = new TProfile("p1RespRecoilInAvgPt", "", nPt, binsPt.data());
    p1RespOtherInAvgPt = new TProfile("p1RespOtherInAvgPt", "", nPt, binsPt.data());
    p1RespUnclusteredInAvgPt = new TProfile("p1RespUnclusteredInAvgPt", "", nPt, binsPt.data());


    p1MjbRespInProbePt = new TProfile("p1MjbRespInProbePt", "", nPt, binsPt.data());
    p1MjbRespInProbeEta = new TProfile("p1MjbRespInProbeEta", "", nEta, binsEta.data());

    p1MpfRespInProbePt = new TProfile("p1MpfRespInProbePt", "", nPt, binsPt.data());
    p1RespSumProbeRecoilInProbePt = new TProfile("p1RespSumProbeRecoilInProbePt", "", nPt, binsPt.data());
    p1RespNegProbeInProbePt = new TProfile("p1RespNegProbeInProbePt", "", nPt, binsPt.data());
    p1RespRecoilInProbePt = new TProfile("p1RespRecoilInProbePt", "", nPt, binsPt.data());
    p1RespOtherInProbePt = new TProfile("p1RespOtherInProbePt", "", nPt, binsPt.data());
    p1RespUnclusteredInProbePt = new TProfile("p1RespUnclusteredInProbePt", "", nPt, binsPt.data());

    p1MjbRespInRecoilPt = new TProfile("p1MjbRespInRecoilPt", "", nPt, binsPt.data());
    p1ProbePtInRecoilPt = new TProfile("p1ProbePtInRecoilPt", "", nPt, binsPt.data());
    p1CrecoilInRecoilPt = new TProfile("p1CrecoilInRecoilPt", "", nPt, binsPt.data());

    p1MpfRespInRecoilPt = new TProfile("p1MpfRespInRecoilPt", "", nPt, binsPt.data());
    p1RespSumProbeRecoilInRecoilPt = new TProfile("p1RespSumProbeRecoilInRecoilPt", "", nPt, binsPt.data());
    p1RespNegProbeInRecoilPt = new TProfile("p1RespNegProbeInRecoilPt", "", nPt, binsPt.data());
    p1RespRecoilInRecoilPt = new TProfile("p1RespRecoilInRecoilPt", "", nPt, binsPt.data());
    p1RespOtherInRecoilPt = new TProfile("p1RespOtherInRecoilPt", "", nPt, binsPt.data());
    p1RespUnclusteredInRecoilPt = new TProfile("p1RespUnclusteredInRecoilPt", "", nPt, binsPt.data());

    // Control histograms
    h2MpfRespInAvgProjPt = new TH2D("h2MpfRespInAvgProjPt", "", nPt, binsPt.data(), 200, -1, 3);
    h2RespSumProbeRecoilInAvgProjPt = new TH2D("h2RespSumProbeRecoilInAvgProjPt", "", nPt, binsPt.data(), 200, -1, 3);
    h2RespNegProbeInAvgProjPt = new TH2D("h2RespNegProbeInAvgProjPt", "", nPt, binsPt.data(), 200, -1, 3);
    h2RespRecoilInAvgProjPt = new TH2D("h2RespRecoilInAvgProjPt", "", nPt, binsPt.data(), 200, -1, 3);

    h2RecoilJetsPtInAvgProjPt = new TH2D("h2RecoilJetsPtInAvgProjPt", "", nPt, binsPt.data(), nPt, binsPt.data());
    h2RecoilJetsPtInAvgPt = new TH2D("h2RecoilJetsPtInAvgPt", "", nPt, binsPt.data(), nPt, binsPt.data());
    h2RecoilJetsPtInProbePt = new TH2D("h2RecoilJetsPtInProbePt", "", nPt, binsPt.data(), nPt, binsPt.data());
    h2RecoilJetsPtInRecoilPt = new TH2D("h2RecoilJetsPtInRecoilPt", "", nPt, binsPt.data(), nPt, binsPt.data());

    p1RhoInAvgProjPt = new TProfile("p1RhoInAvgProjPt", "", nPt, binsPt.data());
    p1RecoilJetsPtInAvgProjPt = new TProfile("p1RecoilJetsPtInAvgProjPt", "", nPt, binsPt.data());
    p1RecoilJetsChfInAvgProjPt = new TProfile("p1RecoilJetsChfInAvgProjPt", "", nPt, binsPt.data());
    p1RecoilJetsNhfInAvgProjPt = new TProfile("p1RecoilJetsNhfInAvgProjPt", "", nPt, binsPt.data());
    p1RecoilJetsNefInAvgProjPt = new TProfile("p1RecoilJetsNefInAvgProjPt", "", nPt, binsPt.data());
    p1RecoilJetsCefInAvgProjPt = new TProfile("p1RecoilJetsCefInAvgProjPt", "", nPt, binsPt.data());
    p1RecoilJetsMufInAvgProjPt = new TProfile("p1RecoilJetsMufInAvgProjPt", "", nPt, binsPt.data());


    p1RhoInAvgProjPtForProbeEta1p3 = new TProfile("p1RhoInAvgProjPtForProbeEta1p3", "", nPt, binsPt.data());
    p1Jet1PtInAvgProjPtForProbeEta1p3 = new TProfile("p1Jet1PtInAvgProjPtForProbeEta1p3", "", nPt, binsPt.data());
    p1Jet1ChfInAvgProjPtForProbeEta1p3 = new TProfile("p1Jet1ChfInAvgProjPtForProbeEta1p3", "", nPt, binsPt.data());
    p1Jet1NhfInAvgProjPtForProbeEta1p3 = new TProfile("p1Jet1NhfInAvgProjPtForProbeEta1p3", "", nPt, binsPt.data());
    p1Jet1NefInAvgProjPtForProbeEta1p3 = new TProfile("p1Jet1NefInAvgProjPtForProbeEta1p3", "", nPt, binsPt.data());
    p1Jet1CefInAvgProjPtForProbeEta1p3 = new TProfile("p1Jet1CefInAvgProjPtForProbeEta1p3", "", nPt, binsPt.data());
    p1Jet1MufInAvgProjPtForProbeEta1p3 = new TProfile("p1Jet1MufInAvgProjPtForProbeEta1p3", "", nPt, binsPt.data());


    h1EventInAvgProjPt->Sumw2();
    h1EventInAvgPt->Sumw2();
    h1EventInProbePt->Sumw2();
    h1EventInRecoilPt->Sumw2();
    h1EventInMetPt->Sumw2();
    h1EventInOtherPt->Sumw2();
    h1EventInUnclusteredPt->Sumw2();

    h1EventInMjbResp->Sumw2();
    h1EventInMpfResp->Sumw2();
    h1EventInRespOther->Sumw2();
    h1EventInRespUnclustered->Sumw2();
    h1EventInCrecoil->Sumw2();
    h1EventInCosDeltaPhiProbeRecoil->Sumw2();
    std::cout << "Initialized HistMultiJet histograms in directory: " << dirName << std::endl;
    origDir->cd();
}

void HistMultiJet::setInputs(const HistMultiJetInputs& inputs){
    fInputs_ = inputs;
}

void HistMultiJet::fillEventLevelHistos(SkimTree* skimT, const int& iJet1, const double& trigPt)
{
    // Here, read from fInputs_ and fill all relevant histos.
    // This is the code that used to be repeated in your big loops.
    const double& ptAvgProj = fInputs_.ptAvgProj;
    const double& ptAverage = fInputs_.ptAverage;
    const double& ptProbe    = fInputs_.ptProbe;
    const double& ptRecoil  = fInputs_.ptRecoil;
    const double& cRecoil   = fInputs_.cRecoil;
    const double& ptMet = fInputs_.ptMet;
    const double& ptOther = fInputs_.ptOther;
    const double& ptUnclustered = fInputs_.ptUnclustered;
    const double& mjbResp  = fInputs_.mjbResp;
    const double& weight    = fInputs_.weight;

    h1EventInAvgProjPt->Fill(ptAvgProj, weight);
    h1EventInAvgPt->Fill(ptAverage, weight);
    h1EventInProbePt->Fill(ptProbe, weight);
    h1EventInRecoilPt->Fill(ptRecoil, weight);
    h1EventInMetPt->Fill(ptMet, weight);
    h1EventInOtherPt->Fill(ptOther, weight);
    h1EventInUnclusteredPt->Fill(ptUnclustered, weight);

    h1EventInMjbResp->Fill(mjbResp, weight);
    h1EventInMpfResp->Fill(fInputs_.m0m, weight);
    h1EventInRespOther->Fill(fInputs_.mnm, weight);
    h1EventInRespUnclustered->Fill(fInputs_.mul, weight);

    p1MjbRespInAvgProjPt->Fill(ptAvgProj, mjbResp, weight);
    p1MjbRespInAvgPt->Fill(ptAverage, mjbResp, weight);
    p1MjbRespInProbePt->Fill(ptProbe, mjbResp, weight);
    p1MjbRespInProbeEta->Fill(fInputs_.etaProbe, mjbResp, weight);
    p1MjbRespInProbePt->Fill(ptProbe, mjbResp, weight);
    p1MjbRespInRecoilPt->Fill(ptRecoil, mjbResp, weight);

    p1ProbePtInAvgProjPt->Fill(ptAvgProj, ptProbe, weight);
    p1ProbePtInAvgPt->Fill(ptAverage, ptProbe, weight);
    p1ProbePtInRecoilPt->Fill(ptRecoil, ptProbe, weight);

    p1ProbePtInProbePt        ->Fill(ptProbe, ptProbe, weight);
    p1RecoilPtInProbePt      ->Fill(ptProbe, ptRecoil, weight);
    p1MetPtInProbePt         ->Fill(ptProbe, ptMet, weight);
    p1OtherPtInProbePt       ->Fill(ptProbe, ptOther, weight);
    p1UnclusteredPtInProbePt ->Fill(ptProbe, ptUnclustered, weight);

    p1CrecoilInAvgProjPt->Fill(ptAvgProj, cRecoil, weight);
    p1CrecoilInAvgPt->Fill(ptAverage, cRecoil, weight);
    h1EventInCrecoil->Fill(cRecoil, weight);
    p1CrecoilInProbePt->Fill(ptProbe, cRecoil, weight);
    p1CrecoilInRecoilPt->Fill(ptRecoil, cRecoil, weight);

    p1MpfRespInAvgProjPt->Fill(ptAvgProj, fInputs_.m0b, weight);
    p1RespSumProbeRecoilInAvgProjPt->Fill(ptAvgProj, fInputs_.mlrb, weight);
    p1RespNegProbeInAvgProjPt->Fill(ptAvgProj, fInputs_.mlb, weight);
    p1RespRecoilInAvgProjPt->Fill(ptAvgProj, fInputs_.mrb, weight);
    p1RespOtherInAvgProjPt->Fill(ptAvgProj, fInputs_.mnb, weight);
    p1RespUnclusteredInAvgProjPt->Fill(ptAvgProj, fInputs_.mub, weight);

    p1MpfRespInAvgPt->Fill(ptAverage, fInputs_.m0m, weight);
    p1RespSumProbeRecoilInAvgPt->Fill(ptAverage, fInputs_.mlrm, weight);
    p1RespNegProbeInAvgPt->Fill(ptAverage, fInputs_.mlm, weight);
    p1RespRecoilInAvgPt->Fill(ptAverage, fInputs_.mrm, weight);
    p1RespOtherInAvgPt->Fill(ptAverage, fInputs_.mnm, weight);
    p1RespUnclusteredInAvgPt->Fill(ptAverage, fInputs_.mum, weight);

    p1MpfRespInProbePt->Fill(ptProbe, fInputs_.m0l, weight);
    p1RespSumProbeRecoilInProbePt->Fill(ptProbe, fInputs_.mlrl, weight);
    p1RespNegProbeInProbePt->Fill(ptProbe, fInputs_.mll, weight);
    p1RespRecoilInProbePt->Fill(ptProbe, fInputs_.mrl, weight);
    p1RespOtherInProbePt->Fill(ptProbe, fInputs_.mnl, weight);
    p1RespUnclusteredInProbePt->Fill(ptProbe, fInputs_.mul, weight);

    p1MpfRespInRecoilPt->Fill(ptRecoil, fInputs_.m0r, weight);
    p1RespSumProbeRecoilInRecoilPt->Fill(ptRecoil, fInputs_.mlrr, weight);
    p1RespNegProbeInRecoilPt->Fill(ptRecoil, fInputs_.mlr, weight);
    p1RespRecoilInRecoilPt->Fill(ptRecoil, fInputs_.mrr, weight);
    p1RespOtherInRecoilPt->Fill(ptRecoil, fInputs_.mnr, weight);
    p1RespUnclusteredInRecoilPt->Fill(ptRecoil, fInputs_.mur, weight);

    h2MpfRespInAvgProjPt->Fill(ptAvgProj, fInputs_.m0b, weight);
    h2RespSumProbeRecoilInAvgProjPt->Fill(ptAvgProj, fInputs_.mlrb, weight);
    h2RespNegProbeInAvgProjPt->Fill(ptAvgProj, fInputs_.mlb, weight);
    h2RespRecoilInAvgProjPt->Fill(ptAvgProj, fInputs_.mrb, weight);
    if (iJet1 != -1 && std::abs(fInputs_.etaProbe) < 1.3){
        p1RhoInAvgProjPtForProbeEta1p3->Fill(ptAvgProj, skimT->Rho, weight);
        p1Jet1PtInAvgProjPtForProbeEta1p3->Fill(ptAvgProj, skimT->Jet_pt[iJet1], weight);
        p1Jet1ChfInAvgProjPtForProbeEta1p3->Fill(ptAvgProj, skimT->Jet_chHEF[iJet1], weight);
        p1Jet1NhfInAvgProjPtForProbeEta1p3->Fill(ptAvgProj, skimT->Jet_neHEF[iJet1], weight);
        p1Jet1NefInAvgProjPtForProbeEta1p3->Fill(ptAvgProj, skimT->Jet_neEmEF[iJet1], weight);
        p1Jet1CefInAvgProjPtForProbeEta1p3->Fill(ptAvgProj, skimT->Jet_chEmEF[iJet1], weight);
        p1Jet1MufInAvgProjPtForProbeEta1p3->Fill(ptAvgProj, skimT->Jet_muEF[iJet1], weight);
    }
    if (ptAverage > 1.25 *trigPt){
        h1EventInCosDeltaPhiProbeRecoil->Fill(
        cos(HelperDelta::DELTAPHI(fInputs_.phiProbe, fInputs_.phiRecoil)), weight);
    }
}

void HistMultiJet::fillJetLevelHistos(SkimTree* skimT, const int& iJet, const double& weightFi){
    
    double pt_i = skimT->Jet_pt[iJet];
    const double& ptAverage = fInputs_.ptAverage;
    const double& ptAvgProj = fInputs_.ptAvgProj;
    const double& ptProbe    = fInputs_.ptProbe;
    const double& ptRecoil  = fInputs_.ptRecoil;

    h2RecoilJetsPtInAvgProjPt->Fill(ptAvgProj, pt_i, weightFi);
    h2RecoilJetsPtInAvgPt->Fill(ptAverage, pt_i, weightFi);
    h2RecoilJetsPtInProbePt->Fill(ptProbe, pt_i, weightFi);
    h2RecoilJetsPtInRecoilPt->Fill(ptRecoil, pt_i, weightFi);

    // Fill PF composition histograms
    p1RhoInAvgProjPt->Fill(ptAvgProj, skimT->Rho, weightFi);
    p1RecoilJetsPtInAvgProjPt->Fill(ptAvgProj, skimT->Jet_pt[iJet], weightFi);
    p1RecoilJetsChfInAvgProjPt->Fill(ptAvgProj, skimT->Jet_chHEF[iJet], weightFi);
    p1RecoilJetsNhfInAvgProjPt->Fill(ptAvgProj, skimT->Jet_neHEF[iJet], weightFi);
    p1RecoilJetsNefInAvgProjPt->Fill(ptAvgProj, skimT->Jet_neEmEF[iJet], weightFi);
    p1RecoilJetsCefInAvgProjPt->Fill(ptAvgProj, skimT->Jet_chEmEF[iJet], weightFi);
    p1RecoilJetsMufInAvgProjPt->Fill(ptAvgProj, skimT->Jet_muEF[iJet], weightFi);
}

void HistMultiJet::printInputs(std::ostream& os) const {
    const auto& v = fInputs_;
    os 
      << "=== HistMultiJetInputs ===\n"
      << "ptAvgProj:      " << v.ptAvgProj   << "\n"
      << "ptAverage:      " << v.ptAverage   << "\n"
      << "ptProbe:         " << v.ptProbe      << "\n"
      << "ptMet:          " << v.ptMet       << "\n"
      << "ptOther:        " << v.ptOther     << "\n"
      << "ptUnclustered:  " << v.ptUnclustered << "\n"
      << "etaProbe:        " << v.etaProbe     << "\n"
      << "phiProbe:        " << v.phiProbe     << "\n"
      << "ptRecoil:       " << v.ptRecoil    << "\n"
      << "phiRecoil:      " << v.phiRecoil   << "\n"
      << "cRecoil:        " << v.cRecoil     << "\n"
      << "mjbResp:        " << v.mjbResp     << "\n"
      << "\n"
      // “b” branch
      << "m0b:            " << v.m0b         << "\n"
      << "mlrb:           " << v.mlrb        << "\n"
      << "mlb:            " << v.mlb         << "\n"
      << "mrb:            " << v.mrb         << "\n"
      << "mnb:            " << v.mnb         << "\n"
      << "mub:            " << v.mub         << "\n"
      << "\n"
      // “m” branch
      << "m0m:            " << v.m0m         << "\n"
      << "mlrm:           " << v.mlrm        << "\n"
      << "mlm:            " << v.mlm         << "\n"
      << "mrm:            " << v.mrm         << "\n"
      << "mnm:            " << v.mnm         << "\n"
      << "mum:            " << v.mum         << "\n"
      << "\n"
      // “l” branch
      << "m0l:            " << v.m0l         << "\n"
      << "mlrl:           " << v.mlrl        << "\n"
      << "mll:            " << v.mll         << "\n"
      << "mrl:            " << v.mrl         << "\n"
      << "mnl:            " << v.mnl         << "\n"
      << "mul:            " << v.mul         << "\n"
      << "\n"
      // “r” branch
      << "m0r:            " << v.m0r         << "\n"
      << "mlrr:           " << v.mlrr        << "\n"
      << "mlr:            " << v.mlr         << "\n"
      << "mrr:            " << v.mrr         << "\n"
      << "mnr:            " << v.mnr         << "\n"
      << "mur:            " << v.mur         << "\n"
      << "\n"
      << "weight:         " << v.weight      << "\n"
      << "==========================\n";
}
