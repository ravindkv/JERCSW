#pragma once

#include <memory>
#include <string>

// forward decls
class TDirectory;
class TH1D;
class TProfile;
class TProfile2D;
class VarBin;

struct GenRecoHistograms {
    // vs Gen{Obj}Pt
    std::unique_ptr<TProfile> p1RecoObjPtOverGenObjPtInGenObjPt;

    // vs Reco{Obj}Pt
    std::unique_ptr<TProfile> p1RecoObjPtOverGenObjPtInRecoObjPt;

    // Event counters
    std::unique_ptr<TH1D> h1EventInGenObjPt;
    std::unique_ptr<TH1D> h1EventInRecoObjPt;

    // Ratios (1D)
    std::unique_ptr<TH1D> h1EventInRecoObjPtOverGenObjPt;
};

class HistObjGenReco {
public:
    HistObjGenReco(TDirectory *origDir,
                      const std::string& directoryName,
                      const VarBin& varBin,
                      const std::string& objKey);

    ~HistObjGenReco();

    /// Fill with Gen//Reco pT of the chosen object and event weight
    void Fill(double ptGen, double ptReco, double weight);

private:
    GenRecoHistograms hist_;
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

