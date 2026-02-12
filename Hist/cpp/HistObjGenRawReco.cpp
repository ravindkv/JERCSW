#include "HistObjGenRawReco.h"
#include "HelperDir.hpp"
#include "VarBin.h"

#include <TH1D.h>
#include <TProfile.h>

#include <iostream>
#include <vector>
#include <algorithm>

std::string HistObjGenRawReco::name4(const std::string& a,
                                     const std::string& b,
                                     const std::string& c,
                                     const std::string& d) {
    // join without separators, e.g. "p1Reco" + "Jet" + "PtOverGen" + "JetPtInGenJetPt"
    return a + b + c + d;
}

HistObjGenRawReco::~HistObjGenRawReco() = default;

HistObjGenRawReco::HistObjGenRawReco(TDirectory *origDir,
                                     const std::string& directoryName,
                                     const VarBin& varBin,
                                     const std::string& objKey)
    : obj_(objKey),
      dirTag_("HistObjGenRawReco_"+objKey)
{
    InitializeHistograms(origDir, directoryName, varBin);
}

void HistObjGenRawReco::InitializeHistograms(TDirectory *origDir,
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

    // Shorthand tokens built from obj_
    const std::string Obj = obj_; // "Jet"/"Pho"/"Ele"

    // Profiles vs Gen{Obj}Pt
    hist_.p1RecoObjPtOverGenObjPtInGenObjPt = std::make_unique<TProfile>(
        name4("p1Reco", Obj, "PtOverGen", Obj + "PtInGen" + Obj + "Pt").c_str(), "", nPt, binsPt.data());

    hist_.p1RawObjPtOverGenObjPtInGenObjPt = std::make_unique<TProfile>(
        name4("p1Raw", Obj, "PtOverGen", Obj + "PtInGen" + Obj + "Pt").c_str(), "", nPt, binsPt.data());

    hist_.p1RecoObjPtOverRawObjPtInGenObjPt = std::make_unique<TProfile>(
        name4("p1Reco", Obj, "PtOverRaw", Obj + "PtInGen" + Obj + "Pt").c_str(), "", nPt, binsPt.data());

    // Profiles vs Reco{Obj}Pt
    hist_.p1RecoObjPtOverGenObjPtInRecoObjPt = std::make_unique<TProfile>(
        name4("p1Reco", Obj, "PtOverGen", Obj + "PtInReco" + Obj + "Pt").c_str(), "", nPt, binsPt.data());

    hist_.p1RawObjPtOverGenObjPtInRecoObjPt = std::make_unique<TProfile>(
        name4("p1Raw", Obj, "PtOverGen", Obj + "PtInReco" + Obj + "Pt").c_str(), "", nPt, binsPt.data());

    hist_.p1RecoObjPtOverRawObjPtInRecoObjPt = std::make_unique<TProfile>(
        name4("p1Reco", Obj, "PtOverRaw", Obj + "PtInReco" + Obj + "Pt").c_str(), "", nPt, binsPt.data());

    // Event counters (same binning as profiles)
    hist_.h1EventInGenObjPt  = std::make_unique<TH1D>(
        ("h1EventInGen"  + Obj + "Pt").c_str(), "", nPt, binsPt.data());
    hist_.h1EventInRawObjPt  = std::make_unique<TH1D>(
        ("h1EventInRaw"  + Obj + "Pt").c_str(), "", nPt, binsPt.data());
    hist_.h1EventInRecoObjPt = std::make_unique<TH1D>(
        ("h1EventInReco" + Obj + "Pt").c_str(), "", nPt, binsPt.data());

    hist_.h1EventInGenObjPt->Sumw2();
    hist_.h1EventInRawObjPt->Sumw2();
    hist_.h1EventInRecoObjPt->Sumw2();

    // Ratio histograms with fixed [0,2] range (adjust if you prefer VarBin-based)
    hist_.h1EventInRecoObjPtOverGenObjPt = std::make_unique<TH1D>(
        ("h1EventInReco" + Obj + "PtOverGen" + Obj + "Pt").c_str(), "", 100, 0., 2.);
    hist_.h1EventInRawObjPtOverGenObjPt = std::make_unique<TH1D>(
        ("h1EventInRaw" + Obj + "PtOverGen" + Obj + "Pt").c_str(), "", 100, 0., 2.);
    hist_.h1EventInRecoObjPtOverRawObjPt = std::make_unique<TH1D>(
        ("h1EventInReco" + Obj + "PtOverRaw" + Obj + "Pt").c_str(), "", 100, 0., 2.);

    hist_.h1EventInRecoObjPtOverGenObjPt->Sumw2();
    hist_.h1EventInRawObjPtOverGenObjPt->Sumw2();
    hist_.h1EventInRecoObjPtOverRawObjPt->Sumw2();

    std::cout << "Initialized " << dirTag_ << " histograms in directory: " << dirName << std::endl;
    origDir->cd();
}

void HistObjGenRawReco::Fill(double ptGen, double ptRaw, double ptReco, double weight)
{
    if (ptGen > 0.0 && ptRaw > 0.0 && ptReco > 0.0) {
        // vs Gen{Obj}Pt
        hist_.p1RecoObjPtOverGenObjPtInGenObjPt->Fill(ptGen,  ptReco/ptGen, weight);
        hist_.p1RecoObjPtOverRawObjPtInGenObjPt->Fill(ptGen,  ptReco/ptRaw, weight);
        hist_.p1RawObjPtOverGenObjPtInGenObjPt ->Fill(ptGen,  ptRaw /ptGen, weight);

        // vs Reco{Obj}Pt
        hist_.p1RecoObjPtOverGenObjPtInRecoObjPt->Fill(ptReco, ptReco/ptGen, weight);
        hist_.p1RecoObjPtOverRawObjPtInRecoObjPt->Fill(ptReco, ptReco/ptRaw, weight);
        hist_.p1RawObjPtOverGenObjPtInRecoObjPt ->Fill(ptReco, ptRaw /ptGen, weight);

        // Event counters
        hist_.h1EventInGenObjPt ->Fill(ptGen,  weight);
        hist_.h1EventInRawObjPt ->Fill(ptRaw,  weight);
        hist_.h1EventInRecoObjPt->Fill(ptReco, weight);

        // Ratios (names now match definitions)
        hist_.h1EventInRecoObjPtOverGenObjPt->Fill(ptReco/ptGen, weight);
        hist_.h1EventInRawObjPtOverGenObjPt ->Fill(ptRaw /ptGen, weight);
        hist_.h1EventInRecoObjPtOverRawObjPt->Fill(ptReco/ptRaw, weight);
    }
}

