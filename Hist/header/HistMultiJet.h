#pragma once

#include <string>
#include <iostream>   // for std::ostream

// forward declarations of ROOT and user types
class TH1D;
class TH2D;
class TProfile;
class TDirectory;
class VarBin;
class SkimTree;

/// A struct to hold all per-event quantities that we want to fill into HistMultiJet.
/// Add or remove fields as needed for your analysis.
struct HistMultiJetInputs {
    double ptAvgProj   = 0.0;
    double ptAverage   = 0.0;
    double ptProbe      = 0.0;
    double ptMet   = 0.0;
    double ptOther   = 0.0;
    double ptUnclustered   = 0.0;
    double etaProbe     = 0.0;
    double phiProbe     = 0.0;
    double ptRecoil    = 0.0;
    double phiRecoil   = 0.0;
    double cRecoil     = 0.0;
    double mjbResp    = 0.0;
    double m0b         = 0.0;
    double mlrb         = 0.0;
    double mlb         = 0.0;
    double mrb         = 0.0;
    double mnb         = 0.0;
    double mub         = 0.0;

    double m0m         = 0.0;
    double mlrm         = 0.0;
    double mlm         = 0.0;
    double mrm         = 0.0;
    double mnm         = 0.0;
    double mum         = 0.0;

    double m0l         = 0.0;
    double mlrl         = 0.0;
    double mll         = 0.0;
    double mrl         = 0.0;
    double mnl         = 0.0;
    double mul         = 0.0;

    double m0r         = 0.0;
    double mlrr         = 0.0;
    double mlr         = 0.0;
    double mrr         = 0.0;
    double mnr         = 0.0;
    double mur         = 0.0;
    double weight      = 1.0;
};

class HistMultiJet {
public:
    // Constructor
    HistMultiJet(TDirectory *origDir, const std::string& directoryName, const VarBin& varBin);

    // Destructor
    ~HistMultiJet();

    // 1) A method to store all user inputs:
    void setInputs(const HistMultiJetInputs& inputs);

    // 2) A method to fill the histograms, using the stored inputs:
    void fillHistos();

    // Basic information about the trigger
    std::string triggerName;
    int trigPt;
    double ptMin, ptMax, absEtaMin, absEtaMax;

    // Histograms and profiles
    TH1D* h1EventInAvgProjPt;
    TH1D* h1EventInAvgPt;
    TH1D* h1EventInProbePt;
    TH1D* h1EventInRecoilPt;
    TH1D* h1EventInMetPt;
    TH1D* h1EventInOtherPt;
    TH1D* h1EventInUnclusteredPt;

    TH1D* h1EventInMjbResp;
    TH1D* h1EventInMpfResp;
    TH1D* h1EventInRespOther;
    TH1D* h1EventInRespUnclustered;

    TProfile* p1MjbRespInAvgProjPt;
    TProfile* p1MjbRespInAvgPt;
    TProfile* p1MjbRespInProbePt;
    TProfile* p1MjbRespInProbeEta;
    TProfile* p1MjbRespInRecoilPt;

    TProfile* p1ProbePtInProbePt;
    TProfile* p1RecoilPtInProbePt;
    TProfile* p1MetPtInProbePt;
    TProfile* p1OtherPtInProbePt;
    TProfile* p1UnclusteredPtInProbePt;

    TProfile* p1ProbePtInAvgProjPt;
    TProfile* p1ProbePtInAvgPt;
    TProfile* p1ProbePtInRecoilPt;

    TProfile* p1CrecoilInAvgProjPt;
    TProfile* p1CrecoilInAvgPt;
    TProfile* p1CrecoilInProbePt;
    TH1D* h1EventInCrecoil;
    TProfile* p1CrecoilInRecoilPt;

    TProfile* p1MpfRespInAvgProjPt;
    TProfile* p1RespSumProbeRecoilInAvgProjPt;
    TProfile* p1RespNegProbeInAvgProjPt;
    TProfile* p1RespRecoilInAvgProjPt;
    TProfile* p1RespOtherInAvgProjPt;
    TProfile* p1RespUnclusteredInAvgProjPt;

    TProfile* p1MpfRespInAvgPt;
    TProfile* p1RespSumProbeRecoilInAvgPt;
    TProfile* p1RespNegProbeInAvgPt;
    TProfile* p1RespRecoilInAvgPt;
    TProfile* p1RespOtherInAvgPt;
    TProfile* p1RespUnclusteredInAvgPt;

    TProfile* p1MpfRespInProbePt;
    TProfile* p1RespSumProbeRecoilInProbePt;
    TProfile* p1RespNegProbeInProbePt;
    TProfile* p1RespRecoilInProbePt;
    TProfile* p1RespOtherInProbePt;
    TProfile* p1RespUnclusteredInProbePt;

    TProfile* p1MpfRespInRecoilPt;
    TProfile* p1RespSumProbeRecoilInRecoilPt;
    TProfile* p1RespNegProbeInRecoilPt;
    TProfile* p1RespRecoilInRecoilPt;
    TProfile* p1RespOtherInRecoilPt;
    TProfile* p1RespUnclusteredInRecoilPt;

    // 2D recoils
    TH2D* h2RecoilJetsPtInAvgProjPt;
    TH2D* h2RecoilJetsPtInAvgPt;
    TH2D* h2RecoilJetsPtInProbePt;
    TH2D* h2RecoilJetsPtInRecoilPt;

    // PF composition plots
    TProfile* p1RhoInAvgProjPt;
    TProfile* p1RecoilJetsPtInAvgProjPt;
    TProfile* p1RecoilJetsChfInAvgProjPt;
    TProfile* p1RecoilJetsNhfInAvgProjPt;
    TProfile* p1RecoilJetsNefInAvgProjPt;
    TProfile* p1RecoilJetsCefInAvgProjPt;
    TProfile* p1RecoilJetsMufInAvgProjPt;

    TProfile* p1RhoInAvgProjPtForProbeEta1p3;
    TProfile* p1Jet1PtInAvgProjPtForProbeEta1p3;
    TProfile* p1Jet1ChfInAvgProjPtForProbeEta1p3;
    TProfile* p1Jet1NhfInAvgProjPtForProbeEta1p3;
    TProfile* p1Jet1NefInAvgProjPtForProbeEta1p3;
    TProfile* p1Jet1CefInAvgProjPtForProbeEta1p3;
    TProfile* p1Jet1MufInAvgProjPtForProbeEta1p3;


    // Controls
    TH2D* h2MpfRespInAvgProjPt;
    TH2D* h2RespSumProbeRecoilInAvgProjPt;
    TH2D* h2RespNegProbeInAvgProjPt;
    TH2D* h2RespRecoilInAvgProjPt;
    TH1D* h1EventInCosDeltaPhiProbeRecoil;

    // Method to initialize histograms
    void InitializeHistograms(TDirectory *origDir, const std::string& directoryName, const VarBin& varBin);
    void fillJetLevelHistos(SkimTree* skimT, const int& iJet,  const double& weightFi);
    void fillEventLevelHistos(SkimTree* skimT, const int& iJet1, const double& trigPt);
    /// Print all of the currently stored inputs to the given stream (default = std::cout)
    void printInputs(std::ostream& os = std::cout) const;

private:
    // Add a private data member to hold the current event’s inputs:
    HistMultiJetInputs fInputs_;
};

