#include "HistObjGenReco.h"
#include "HelperDir.hpp"
#include "VarBin.h"

#include <TH1D.h>
#include <TProfile.h>
#include <TProfile2D.h>

#include <iostream>
#include <vector>

std::string HistObjGenReco::name4(const std::string& a,
                                 const std::string& b,
                                 const std::string& c,
                                 const std::string& d) {
    // join without separators, e.g. "p1Reco" + "Jet" + "PtOverGen" + "JetPtInGenJetPt"
    return a + b + c + d;
}

HistObjGenReco::~HistObjGenReco() = default;

HistObjGenReco::HistObjGenReco(TDirectory* origDir,
                               const std::string& directoryName,
                               const VarBin& varBin,
                               const std::string& objKey)
    : obj_(objKey),
      dirTag_("HistObjGenReco_" + objKey)
{
    InitializeHistograms(origDir, directoryName, varBin);
}

void HistObjGenReco::InitializeHistograms(TDirectory* origDir,
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

    // --- Binning ---
    const std::vector<double> binsPt  = varBin.getBinsPt();
    const int nPt = static_cast<int>(binsPt.size()) - 1;

    // Shorthand tokens built from obj_
    const std::string Obj = obj_; // "Jet"/"Pho"/"Ele"/...

    // -------------------------
    // Profiles vs Gen{Obj}Pt
    // -------------------------
    hist_.p1RecoObjPtOverGenObjPtInGenObjPt = std::make_unique<TProfile>(
        name4("p1Reco", Obj, "PtOverGen", Obj + "PtInGen" + Obj + "Pt").c_str(),
        "",
        nPt, binsPt.data()
    );

    // -------------------------
    // Profiles vs Reco{Obj}Pt
    // -------------------------
    hist_.p1RecoObjPtOverGenObjPtInRecoObjPt = std::make_unique<TProfile>(
        name4("p1Reco", Obj, "PtOverGen", Obj + "PtInReco" + Obj + "Pt").c_str(),
        "",
        nPt, binsPt.data()
    );

    // -------------------------
    // Event counters
    // -------------------------
    hist_.h1EventInGenObjPt = std::make_unique<TH1D>(
        ("h1EventInGen" + Obj + "Pt").c_str(), "",
        nPt, binsPt.data()
    );
    hist_.h1EventInRecoObjPt = std::make_unique<TH1D>(
        ("h1EventInReco" + Obj + "Pt").c_str(), "",
        nPt, binsPt.data()
    );

    hist_.h1EventInGenObjPt->Sumw2();
    hist_.h1EventInRecoObjPt->Sumw2();

    // Ratio histogram (ptReco/ptGen)
    hist_.h1EventInRecoObjPtOverGenObjPt = std::make_unique<TH1D>(
        ("h1EventInReco" + Obj + "PtOverGen" + Obj + "Pt").c_str(),
        "",
        100, 0., 2.
    );
    hist_.h1EventInRecoObjPtOverGenObjPt->Sumw2();

    std::cout << "Initialized " << dirTag_
              << " histograms in directory: " << dirName << std::endl;

    origDir->cd();
}

void HistObjGenReco::Fill(double ptGen, double ptReco,
                          double weight)
{
    if (ptGen <= 0.0 || ptReco <= 0.0) return;

    const double ratio = ptReco / ptGen;

    // vs Gen{Obj}Pt
    if (hist_.p1RecoObjPtOverGenObjPtInGenObjPt)
        hist_.p1RecoObjPtOverGenObjPtInGenObjPt->Fill(ptGen, ratio, weight);

    // vs Reco{Obj}Pt
    if (hist_.p1RecoObjPtOverGenObjPtInRecoObjPt)
        hist_.p1RecoObjPtOverGenObjPtInRecoObjPt->Fill(ptReco, ratio, weight);

    // Event counters
    if (hist_.h1EventInGenObjPt)  hist_.h1EventInGenObjPt->Fill(ptGen,  weight);
    if (hist_.h1EventInRecoObjPt) hist_.h1EventInRecoObjPt->Fill(ptReco, weight);

    // Ratio distribution
    if (hist_.h1EventInRecoObjPtOverGenObjPt)
        hist_.h1EventInRecoObjPtOverGenObjPt->Fill(ratio, weight);
}

