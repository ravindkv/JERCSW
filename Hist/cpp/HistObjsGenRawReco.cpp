#include "HistObjsGenRawReco.h"
#include "HelperDir.hpp"
#include "VarBin.h"

#include <TH1D.h>
#include <TProfile.h>

#include <iostream>
#include <vector>
#include <cmath>

std::string HistObjsGenRawReco::join4(const std::string& a,
                                      const std::string& b,
                                      const std::string& c,
                                      const std::string& d) {
    return a + b + c + d;
}

HistObjsGenRawReco::~HistObjsGenRawReco() = default;

HistObjsGenRawReco::HistObjsGenRawReco(TDirectory* origDir,
                                       const std::string& directoryName,
                                       const VarBin& varBin,
                                       const std::string& objA,
                                       const std::string& objB)
    : objA_(objA),
      objB_(objB),
      dirTag_("HistObjsGenRawReco_" + objA + "Vs" + objB)
{
    Initialize(origDir, directoryName, varBin);
}

void HistObjsGenRawReco::Initialize(TDirectory* origDir,
                                    const std::string& baseDir,
                                    const VarBin& varBin)
{
    const std::string dirName = baseDir + "/" + dirTag_;
    TDirectory* newDir = HelperDir::createTDirectory(origDir, dirName);
    if (!newDir) {
        std::cerr << "Error: Failed to create directory: " << dirName << std::endl;
        return;
    }
    newDir->cd();

    std::vector<double> binsPt = varBin.getBinsPt();
    const int nPt = static_cast<int>(binsPt.size()) - 1;

    // ---------- Profiles (vs Gen{ObjB} pT)
    hist_.p1GenObjAPtOverGenObjBPtInGenObjBPt = std::make_unique<TProfile>(
        join4("p1Gen",  objA_, "PtOverGen",  objB_ + "PtInGen"  + objB_ + "Pt").c_str(), "", nPt, binsPt.data());
    hist_.p1RawObjAPtOverRawObjBPtInGenObjBPt = std::make_unique<TProfile>(
        join4("p1Raw",  objA_, "PtOverRaw",  objB_ + "PtInGen"  + objB_ + "Pt").c_str(), "", nPt, binsPt.data());
    hist_.p1RecoObjAPtOverRecoObjBPtInGenObjBPt = std::make_unique<TProfile>(
        join4("p1Reco", objA_, "PtOverReco", objB_ + "PtInGen"  + objB_ + "Pt").c_str(), "", nPt, binsPt.data());
    hist_.p1GenObjAPtOverRawObjBPtInGenObjBPt = std::make_unique<TProfile>(
        join4("p1Gen",  objA_, "PtOverRaw",  objB_ + "PtInGen"  + objB_ + "Pt").c_str(), "", nPt, binsPt.data());
    hist_.p1GenObjAPtOverRecoObjBPtInGenObjBPt = std::make_unique<TProfile>(
        join4("p1Gen",  objA_, "PtOverReco", objB_ + "PtInGen"  + objB_ + "Pt").c_str(), "", nPt, binsPt.data());
    hist_.p1RecoObjAPtMinusRecoObjBPtInGenObjBPt = std::make_unique<TProfile>(
        join4("p1Reco", objA_, "PtMinusReco",objB_ + "PtInGen"  + objB_ + "Pt").c_str(), "", nPt, binsPt.data());

    // ---------- Profiles (vs Reco{ObjB} pT)
    hist_.p1GenObjAPtOverGenObjBPtInRecoObjBPt = std::make_unique<TProfile>(
        join4("p1Gen",  objA_, "PtOverGen",  objB_ + "PtInReco" + objB_ + "Pt").c_str(), "", nPt, binsPt.data());
    hist_.p1RawObjAPtOverRawObjBPtInRecoObjBPt = std::make_unique<TProfile>(
        join4("p1Raw",  objA_, "PtOverRaw",  objB_ + "PtInReco" + objB_ + "Pt").c_str(), "", nPt, binsPt.data());
    hist_.p1RecoObjAPtOverRecoObjBPtInRecoObjBPt = std::make_unique<TProfile>(
        join4("p1Reco", objA_, "PtOverReco", objB_ + "PtInReco" + objB_ + "Pt").c_str(), "", nPt, binsPt.data());
    hist_.p1GenObjAPtOverRawObjBPtInRecoObjBPt = std::make_unique<TProfile>(
        join4("p1Gen",  objA_, "PtOverRaw",  objB_ + "PtInReco" + objB_ + "Pt").c_str(), "", nPt, binsPt.data());
    hist_.p1GenObjAPtOverRecoObjBPtInRecoObjBPt = std::make_unique<TProfile>(
        join4("p1Gen",  objA_, "PtOverReco", objB_ + "PtInReco" + objB_ + "Pt").c_str(), "", nPt, binsPt.data());
    hist_.p1RecoObjAPtMinusRecoObjBPtInRecoObjBPt = std::make_unique<TProfile>(
        join4("p1Reco", objA_, "PtMinusReco",objB_ + "PtInReco" + objB_ + "Pt").c_str(), "", nPt, binsPt.data());

    // ---------- TH1D spectra
    // Ratios in [0,2], difference in [-200,200] GeV (tune as needed)
    hist_.h1GenObjAPtOverGenObjBPt   = std::make_unique<TH1D>(("h1Gen"  + objA_ + "PtOverGen"  + objB_ + "Pt").c_str(), "", 100, 0., 2.);
    hist_.h1RawObjAPtOverRawObjBPt   = std::make_unique<TH1D>(("h1Raw"  + objA_ + "PtOverRaw"  + objB_ + "Pt").c_str(), "", 100, 0., 2.);
    hist_.h1RecoObjAPtOverRecoObjBPt = std::make_unique<TH1D>(("h1Reco" + objA_ + "PtOverReco" + objB_ + "Pt").c_str(), "", 100, 0., 2.);
    hist_.h1GenObjAPtOverRawObjBPt   = std::make_unique<TH1D>(("h1Gen"  + objA_ + "PtOverRaw"  + objB_ + "Pt").c_str(), "", 100, 0., 2.);
    hist_.h1GenObjAPtOverRecoObjBPt  = std::make_unique<TH1D>(("h1Gen"  + objA_ + "PtOverReco" + objB_ + "Pt").c_str(), "", 100, 0., 2.);
    hist_.h1RecoObjAPtMinusRecoObjBPt= std::make_unique<TH1D>(("h1Reco" + objA_ + "PtMinusReco"+ objB_ + "Pt").c_str(), "", 200, -200., 200.);

    hist_.h1GenObjAPtOverGenObjBPt  ->Sumw2();
    hist_.h1RawObjAPtOverRawObjBPt  ->Sumw2();
    hist_.h1RecoObjAPtOverRecoObjBPt->Sumw2();
    hist_.h1GenObjAPtOverRawObjBPt  ->Sumw2();
    hist_.h1GenObjAPtOverRecoObjBPt ->Sumw2();
    hist_.h1RecoObjAPtMinusRecoObjBPt->Sumw2();

    std::cout << "Initialized " << dirTag_ << " in: " << dirName << std::endl;
    origDir->cd();
}

void HistObjsGenRawReco::Fill(double genA, double rawA, double recoA,
                              double genB, double rawB, double recoB,
                              double weight)
{
    const bool okGenA  = (genA  > 0.0);
    const bool okRawA  = (rawA  > 0.0);
    const bool okRecoA = (recoA > 0.0);

    const bool okGenB  = (genB  > 0.0);
    const bool okRawB  = (rawB  > 0.0);
    const bool okRecoB = (recoB > 0.0);

    // ---- TH1D spectra
    if (okGenA && okGenB)   hist_.h1GenObjAPtOverGenObjBPt  ->Fill(genA  / genB,  weight);
    if (okRawA && okRawB)   hist_.h1RawObjAPtOverRawObjBPt  ->Fill(rawA  / rawB,  weight);
    if (okRecoA && okRecoB) hist_.h1RecoObjAPtOverRecoObjBPt->Fill(recoA / recoB, weight);
    if (okGenA && okRawB)   hist_.h1GenObjAPtOverRawObjBPt  ->Fill(genA  / rawB,  weight);
    if (okGenA && okRecoB)  hist_.h1GenObjAPtOverRecoObjBPt ->Fill(genA  / recoB, weight);
    if (okRecoA && okRecoB) hist_.h1RecoObjAPtMinusRecoObjBPt->Fill(recoA - recoB, weight);

    // ---- Profiles vs Gen{ObjB} pT
    if (okGenB) {
        if (okGenA)  hist_.p1GenObjAPtOverGenObjBPtInGenObjBPt  ->Fill(genB,  genA  / genB,  weight);
        if (okRawA)  hist_.p1RawObjAPtOverRawObjBPtInGenObjBPt  ->Fill(genB,  okRawB  ? rawA  / rawB  : NAN, weight);
        if (okRecoA) hist_.p1RecoObjAPtOverRecoObjBPtInGenObjBPt->Fill(genB,  okRecoB ? recoA / recoB : NAN, weight);
        if (okGenA)  hist_.p1GenObjAPtOverRawObjBPtInGenObjBPt  ->Fill(genB,  okRawB  ? genA  / rawB  : NAN, weight);
        if (okGenA)  hist_.p1GenObjAPtOverRecoObjBPtInGenObjBPt ->Fill(genB,  okRecoB ? genA  / recoB : NAN, weight);
        if (okRecoA) hist_.p1RecoObjAPtMinusRecoObjBPtInGenObjBPt->Fill(genB,  okRecoB ? (recoA - recoB) : NAN, weight);
    }

    // ---- Profiles vs Reco{ObjB} pT
    if (okRecoB) {
        if (okGenA && okGenB)   hist_.p1GenObjAPtOverGenObjBPtInRecoObjBPt  ->Fill(recoB, genA  / genB,  weight);
        if (okRawA && okRawB)   hist_.p1RawObjAPtOverRawObjBPtInRecoObjBPt  ->Fill(recoB, rawA  / rawB,  weight);
        if (okRecoA && okRecoB) hist_.p1RecoObjAPtOverRecoObjBPtInRecoObjBPt->Fill(recoB, recoA / recoB, weight);
        if (okGenA && okRawB)   hist_.p1GenObjAPtOverRawObjBPtInRecoObjBPt  ->Fill(recoB, genA  / rawB,  weight);
        if (okGenA && okRecoB)  hist_.p1GenObjAPtOverRecoObjBPtInRecoObjBPt ->Fill(recoB, genA  / recoB, weight);
        if (okRecoA && okRecoB) hist_.p1RecoObjAPtMinusRecoObjBPtInRecoObjBPt->Fill(recoB, recoA - recoB, weight);
    }
}

