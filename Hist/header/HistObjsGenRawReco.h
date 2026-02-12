#pragma once

#include <memory>
#include <string>

// fwd decls
class TDirectory;
class TH1D;
class TProfile;
class VarBin;

struct ObjsGenRawRecoHists {
    // ---------------------------
    // TProfiles: X-axis = Gen{ObjB} pT
    // ---------------------------
    std::unique_ptr<TProfile> p1GenObjAPtOverGenObjBPtInGenObjBPt;
    std::unique_ptr<TProfile> p1RawObjAPtOverRawObjBPtInGenObjBPt;
    std::unique_ptr<TProfile> p1RecoObjAPtOverRecoObjBPtInGenObjBPt;
    std::unique_ptr<TProfile> p1GenObjAPtOverRawObjBPtInGenObjBPt;
    std::unique_ptr<TProfile> p1GenObjAPtOverRecoObjBPtInGenObjBPt;
    std::unique_ptr<TProfile> p1RecoObjAPtMinusRecoObjBPtInGenObjBPt;

    // ---------------------------
    // TProfiles: X-axis = Reco{ObjB} pT
    // ---------------------------
    std::unique_ptr<TProfile> p1GenObjAPtOverGenObjBPtInRecoObjBPt;
    std::unique_ptr<TProfile> p1RawObjAPtOverRawObjBPtInRecoObjBPt;
    std::unique_ptr<TProfile> p1RecoObjAPtOverRecoObjBPtInRecoObjBPt;
    std::unique_ptr<TProfile> p1GenObjAPtOverRawObjBPtInRecoObjBPt;
    std::unique_ptr<TProfile> p1GenObjAPtOverRecoObjBPtInRecoObjBPt;
    std::unique_ptr<TProfile> p1RecoObjAPtMinusRecoObjBPtInRecoObjBPt;

    // ---------------------------
    // TH1D spectra (fixed binning; adjust if desired)
    // ---------------------------
    std::unique_ptr<TH1D> h1GenObjAPtOverGenObjBPt;    // genA/genB
    std::unique_ptr<TH1D> h1RawObjAPtOverRawObjBPt;    // rawA/rawB
    std::unique_ptr<TH1D> h1RecoObjAPtOverRecoObjBPt;  // recoA/recoB
    std::unique_ptr<TH1D> h1GenObjAPtOverRawObjBPt;    // genA/rawB
    std::unique_ptr<TH1D> h1GenObjAPtOverRecoObjBPt;   // genA/recoB
    std::unique_ptr<TH1D> h1RecoObjAPtMinusRecoObjBPt; // recoA - recoB
};

class HistObjsGenRawReco {
public:
    HistObjsGenRawReco(TDirectory* origDir,
                       const std::string& directoryName,
                       const VarBin& varBin,
                       const std::string& objA,
                       const std::string& objB);
    ~HistObjsGenRawReco();

    // Fill with (A: gen/raw/reco) and (B: gen/raw/reco), plus weight
    void Fill(double genA, double rawA, double recoA,
              double genB, double rawB, double recoB,
              double weight);

private:
    ObjsGenRawRecoHists hist_;
    std::string objA_;      // e.g., "Pho"/"Ele"/"Jet"
    std::string objB_;      // e.g., "Jet"/"Pho"/"Ele"
    std::string dirTag_;    // "HistObjsGenRawReco_{A}Vs{B}"

    void Initialize(TDirectory* origDir,
                    const std::string& baseDir,
                    const VarBin& varBin);

    // helpers to build explicit names
    static std::string join4(const std::string& a,
                             const std::string& b,
                             const std::string& c,
                             const std::string& d);
};

