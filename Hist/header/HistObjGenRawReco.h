#pragma once

#include <memory>
#include <string>

// forward decls
class TDirectory;
class TH1D;
class TProfile;
class VarBin;

struct GenRawRecoHistograms {
    // vs Gen{Obj}Pt
    std::unique_ptr<TProfile> p1RecoObjPtOverGenObjPtInGenObjPt;
    std::unique_ptr<TProfile> p1RecoObjPtOverRawObjPtInGenObjPt;
    std::unique_ptr<TProfile> p1RawObjPtOverGenObjPtInGenObjPt;

    // vs Reco{Obj}Pt
    std::unique_ptr<TProfile> p1RecoObjPtOverGenObjPtInRecoObjPt;
    std::unique_ptr<TProfile> p1RawObjPtOverGenObjPtInRecoObjPt;
    std::unique_ptr<TProfile> p1RecoObjPtOverRawObjPtInRecoObjPt;

    // Event counters
    std::unique_ptr<TH1D> h1EventInGenObjPt;
    std::unique_ptr<TH1D> h1EventInRawObjPt;
    std::unique_ptr<TH1D> h1EventInRecoObjPt;

    // Ratios (1D)
    std::unique_ptr<TH1D> h1EventInRecoObjPtOverGenObjPt;
    std::unique_ptr<TH1D> h1EventInRawObjPtOverGenObjPt;
    std::unique_ptr<TH1D> h1EventInRecoObjPtOverRawObjPt;
};

class HistObjGenRawReco {
public:
    HistObjGenRawReco(TDirectory *origDir,
                      const std::string& directoryName,
                      const VarBin& varBin,
                      const std::string& objKey);

    ~HistObjGenRawReco();

    /// Fill with Gen/Raw/Reco pT of the chosen object and event weight
    void Fill(double ptGen, double ptRaw, double ptReco, double weight);

private:
    GenRawRecoHistograms hist_;
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

