#pragma once

#include <memory>
#include <string>
#include <TLorentzVector.h>

// forward decls
class TDirectory;
class TH1D;
class TProfile;
class TProfile2D;
class VarBin;

struct JERHistograms {
    // TH1
    std::unique_ptr<TH1D> h1EventInRecoObjPt;
    std::unique_ptr<TH1D> h1EventInGenObjPt;
    std::unique_ptr<TH1D> h1EventInRecoObjPtOverGenObjPt;


    // Profile 1D
    std::unique_ptr<TProfile> p1RecoObjPtOverGenObjPtInRho;

    //In RecoObj
    std::unique_ptr<TProfile> p1RecoObjPtOverGenObjPtInRecoObjPt;
    std::unique_ptr<TProfile> p1RecoObjPtOverGenObjPtInRecoObjEta;
    std::unique_ptr<TProfile> p1RecoObjPtOverGenObjPtInRecoObjPhi;
    // Profile 2D: Eta
    std::unique_ptr<TProfile2D> p2RecoObjPtOverGenObjPtInRecoObjPtRecoObjEta;
    std::unique_ptr<TProfile2D> p2RecoObjPtOverGenObjPtInRecoObjPhiRecoObjEta;
    std::unique_ptr<TProfile2D> p2RecoObjPtOverGenObjPtInRhoRecoObjEta;
    // Profile 2D: Pt
    std::unique_ptr<TProfile2D> p2RecoObjPtOverGenObjPtInRecoObjEtaRecoObjPt;
    std::unique_ptr<TProfile2D> p2RecoObjPtOverGenObjPtInRecoObjPhiRecoObjPt;
    std::unique_ptr<TProfile2D> p2RecoObjPtOverGenObjPtInRhoRecoObjPt;

    //In GenObj
    std::unique_ptr<TProfile> p1RecoObjPtOverGenObjPtInGenObjPt;
    std::unique_ptr<TProfile> p1RecoObjPtOverGenObjPtInGenObjEta;
    std::unique_ptr<TProfile> p1RecoObjPtOverGenObjPtInGenObjPhi;
    // Profile 2D: Eta
    std::unique_ptr<TProfile2D> p2RecoObjPtOverGenObjPtInGenObjPtGenObjEta;
    std::unique_ptr<TProfile2D> p2RecoObjPtOverGenObjPtInGenObjPhiGenObjEta;
    std::unique_ptr<TProfile2D> p2RecoObjPtOverGenObjPtInRhoGenObjEta;
    // Profile 2D: Pt
    std::unique_ptr<TProfile2D> p2RecoObjPtOverGenObjPtInGenObjEtaGenObjPt;
    std::unique_ptr<TProfile2D> p2RecoObjPtOverGenObjPtInGenObjPhiGenObjPt;
    std::unique_ptr<TProfile2D> p2RecoObjPtOverGenObjPtInRhoGenObjPt;

    // Profile 2D: GenPt vs RecoEta
    std::unique_ptr<TProfile2D> p2RecoObjPtOverGenObjPtInGenObjPtRecoObjEta;
};

class HistObjJER {
public:
    HistObjJER(TDirectory *origDir,
                      const std::string& directoryName,
                      const VarBin& varBin,
                      const std::string& objKey);

    ~HistObjJER();

    /// Fill with Gen//Reco pT of the chosen object and event weight
    void Fill(TLorentzVector p4Gen, TLorentzVector p4Reco, double rho, double weight);

private:
    JERHistograms hist_;
    std::string obj_;        // "Jet", "Pho", "Ele", or any other
    std::string dirTag_;     

    void InitializeHistograms(TDirectory *origDir,
                              const std::string& baseDir,
                              const VarBin& varBin);

    // helper to build long names consistently
    static std::string name4(const std::string& a,
                             const std::string& b,
                             const std::string& c,
                             const std::string& d);
};

