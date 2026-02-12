#pragma once

#include <memory>
#include <string>
#include <vector>

class TDirectory;
class TH1D;
class TH2D;
class TProfile;
class TLorentzVector;
class VarBin;

// --- Matching category for monitoring
enum class GamMatchTag : int {
    MotherPairSum = 0,
    Mother4Vec    = 1,
    MotherPrefer  = 2,
    MotherSingle  = 3,
    SumCore       = 4,
    SumAdapt      = 5,
    SumSCwindow   = 6,
    NearestAnyGen = 7,
    NearestGenIso = 8,
    None          = 9
};

// --- Optional debug payload from the matcher
struct GamMatchMon {
    double recoPt   {0.0};   // photon reco pT
    double recoEta  {0.0};   // photon reco eta
    double recoPhi  {0.0};   // photon reco phi
    double r9       {0.0};

    // chosen match kinematics (gen-side)
    double dRmatch  {999.};
    double dEta     {999.};
    double dPhi     {999.};
    double relPtDiff{999.}; // |pTgen - pTreco| / pTreco

    // catchment parameters
    double R_core   {0.0};
    double R_adapt  {0.0};
    double eta_max  {0.0};
    double phi_max  {0.0};

    int    nDauForBest {-1};      // 0/1/2 if mother-based, otherwise -1
    GamMatchTag tag {GamMatchTag::None};
};

struct GamJetFakeHistograms {
    // --- Profiles vs GenJet pT (photon-based only)
    std::unique_ptr<TProfile> p1RecoPhoPtOverGenJetPtInGenJetPt;
    std::unique_ptr<TProfile> p1RecoPhoPtOverGenPhoPtInGenJetPt;
    std::unique_ptr<TProfile> p1GenJetPtOverGenPhoPtInGenJetPt;
    std::unique_ptr<TProfile> p1GenJetPtOverGenPhoPtInRecoPhoPt;
    // --- Profiles vs photon reco pT and R9
    std::unique_ptr<TProfile> p1RecoPhoPtOverGenPhoPtInRecoPhoPt;
    std::unique_ptr<TProfile> p1RecoPhoPtOverGenJetPtInRecoPhoPt;
    std::unique_ptr<TProfile> p1RecoPhoPtOverGenPhoPtInR9;
    std::unique_ptr<TProfile> p1RecoPhoPtOverGenJetPtInR9;


    // --- Core 1D responses (photon-based)
    std::unique_ptr<TH1D> h1EventInRecoPhoPtOverGenPhoPt;
    std::unique_ptr<TH1D> h1EventInRecoPhoPtOverGenJetPt;
    std::unique_ptr<TH1D> h1EventInGenJetPtOverGenPhoPt;

    // --- Angular diagnostics
    std::unique_ptr<TH1D> h1DRmatch;
    std::unique_ptr<TH1D> h1DEtaMatch;
    std::unique_ptr<TH1D> h1DPhiMatch;

    // --- Category & daughters
    std::unique_ptr<TH1D> h1MatchTag;   // integer-coded GamMatchTag
    std::unique_ptr<TH1D> h1NDauBest;   // 0/1/2/3 (3=≥3), -1 ⇒ “no-mother”

    // --- Heatmaps to tune magic numbers (photon-based)
    std::unique_ptr<TH2D> h2PhoOverGenPhoVsRecoPt;
    std::unique_ptr<TH2D> h2PhoOverGenPhoVsR9;
    std::unique_ptr<TH2D> h2DRmatchVsRecoPt;
    std::unique_ptr<TH2D> h2RespVsEta;           // pho/genPho vs |eta|
    std::unique_ptr<TH2D> h2RespVsRcore;         // pho/genPho vs R_core
    std::unique_ptr<TH2D> h2RespVsRadapt;        // pho/genPho vs R_adapt
    std::unique_ptr<TH2D> h2RespVsPhiMax;        // pho/genPho vs phi_max
    std::unique_ptr<TH2D> h2RespVsEtaMax;        // pho/genPho vs eta_max
};

class HistGamJetFake {
public:
    HistGamJetFake(TDirectory *origDir, const std::string& directoryName, const VarBin& varBin);
    ~HistGamJetFake();

    // Legacy-style Fill (jet args removed)
    void Fill(double ptGenJet, double ptRecoJet,
              double ptGenPho, double ptRecoPho,
              double weight);

    // Extended Fill with monitoring payload (recommended)
    void Fill(double ptGenJet, double ptRecoJet,
              double ptGenPho, double ptRecoPho,
              const GamMatchMon& mon,
              double weight);

private:
    GamJetFakeHistograms hist_;
    void InitializeHistograms(TDirectory *origDir, const std::string& directoryName, const VarBin& varBin);
};

