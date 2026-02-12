#include "HistObjJER.h"
#include "HelperDir.hpp"
#include "VarBin.h"

#include <TH1D.h>
#include <TProfile.h>
#include <TProfile2D.h>

#include <iostream>
#include <vector>
#include <cmath>

std::string HistObjJER::name4(const std::string& a,
                             const std::string& b,
                             const std::string& c,
                             const std::string& d)
{
    return a + b + c + d;
}

HistObjJER::~HistObjJER() = default;

HistObjJER::HistObjJER(TDirectory* origDir,
                       const std::string& directoryName,
                       const VarBin& varBin,
                       const std::string& objKey)
    : obj_(objKey),
      dirTag_("HistObjJER_" + objKey)   // (your old tag said GenReco; JER seems more accurate)
{
    InitializeHistograms(origDir, directoryName, varBin);
}

void HistObjJER::InitializeHistograms(TDirectory* origDir,
                                      const std::string& baseDir,
                                      const VarBin& varBin)
{
    const std::string dirName = baseDir + "/" + dirTag_;
    TDirectory* newDir = HelperDir::createTDirectory(origDir, dirName);
    if (!newDir) {
        std::cerr << "[HistObjJER] ERROR: Failed to create directory: " << dirName << "\n";
        return;
    }
    newDir->cd();

    // --- Binning ---
    const std::vector<double> binsPt  = varBin.getBinsPt();
    const int nPt = static_cast<int>(binsPt.size()) - 1;

    const std::vector<double> binsEta = varBin.getBinsEta();
    const int nEta = static_cast<int>(binsEta.size()) - 1;

    // These are assumed to exist in your VarBin (typical in your framework).
    const std::vector<double> binsPhi = varBin.getBinsPhi();
    const int nPhi = static_cast<int>(binsPhi.size()) - 1;

    const std::vector<double> binsRho = varBin.getBinsRho();
    const int nRho = static_cast<int>(binsRho.size()) - 1;

    const std::string Obj = obj_; // "Jet"/"Pho"/"Ele"/...

    hist_.h1EventInRecoObjPt = std::make_unique<TH1D>(
        ("h1EventInReco" + Obj + "Pt").c_str(), "", nPt, binsPt.data()
    );
    hist_.h1EventInGenObjPt = std::make_unique<TH1D>(
        ("h1EventInGen" + Obj + "Pt").c_str(), "", nPt, binsPt.data()
    );

    // Ratio histogram (ptReco/ptGen)
    hist_.h1EventInRecoObjPtOverGenObjPt = std::make_unique<TH1D>(
        ("h1EventInReco" + Obj + "PtOverGen" + Obj + "Pt").c_str(),
        "",
        100, 0., 2.
    );
    hist_.h1EventInRecoObjPtOverGenObjPt->Sumw2();
    // Small local helpers (avoid repetition)
    auto makeP1 = [](const std::string& name,
                     int nbins, const double* bins) -> std::unique_ptr<TProfile>
    {
        auto p = std::make_unique<TProfile>(name.c_str(), "", nbins, bins);
        return p;
    };

    auto makeP2 = [](const std::string& name,
                     int nx, const double* bx,
                     int ny, const double* by) -> std::unique_ptr<TProfile2D>
    {
        auto p = std::make_unique<TProfile2D>(name.c_str(), "", nx, bx, ny, by);
        return p;
    };

    //---------- RecoObj
    // =========================================================
    // Profile 1D: ratio vs Reco{Obj}{Var}
    // =========================================================
    hist_.p1RecoObjPtOverGenObjPtInRecoObjPt  =
        makeP1(name4("p1Reco", Obj, "PtOverGen", Obj + "PtInReco" + Obj + "Pt"),
               nPt, binsPt.data());

    hist_.p1RecoObjPtOverGenObjPtInRecoObjEta =
        makeP1(name4("p1Reco", Obj, "PtOverGen", Obj + "PtInReco" + Obj + "Eta"),
               nEta, binsEta.data());

    hist_.p1RecoObjPtOverGenObjPtInRecoObjPhi =
        makeP1(name4("p1Reco", Obj, "PtOverGen", Obj + "PtInReco" + Obj + "Phi"),
               nPhi, binsPhi.data());

    // =========================================================
    // Profile 2D: ratio vs (Reco{Obj}{X}, Reco{Obj}{Y})
    // NOTE: TProfile2D::Fill(x, y, z, w)  (z=ratio here)
    // =========================================================

    // (Pt, Eta)
    hist_.p2RecoObjPtOverGenObjPtInRecoObjPtRecoObjEta =
        makeP2(name4("p2Reco", Obj, "PtOverGen",
                     Obj + "PtInReco" + Obj + "PtReco" + Obj + "Eta"),
               nEta, binsEta.data(),
               nPt,  binsPt.data());

    // (Phi, Eta)
    hist_.p2RecoObjPtOverGenObjPtInRecoObjPhiRecoObjEta =
        makeP2(name4("p2Reco", Obj, "PtOverGen",
                     Obj + "PtInReco" + Obj + "PhiReco" + Obj + "Eta"),
               nEta, binsEta.data(),
               nPhi, binsPhi.data());

    // (Rho, Eta)
    hist_.p2RecoObjPtOverGenObjPtInRhoRecoObjEta =
        makeP2(name4("p2Reco", Obj, "PtOverGen",
                     Obj + "PtInRhoReco" + Obj + "Eta"),
               nEta, binsEta.data(),
               nRho, binsRho.data());

    // (Eta, Pt)
    hist_.p2RecoObjPtOverGenObjPtInRecoObjEtaRecoObjPt =
        makeP2(name4("p2Reco", Obj, "PtOverGen",
                     Obj + "PtInReco" + Obj + "EtaReco" + Obj + "Pt"),
               nPt,  binsPt.data(),
               nEta, binsEta.data());

    // (Phi, Pt)
    hist_.p2RecoObjPtOverGenObjPtInRecoObjPhiRecoObjPt =
        makeP2(name4("p2Reco", Obj, "PtOverGen",
                     Obj + "PtInReco" + Obj + "PhiReco" + Obj + "Pt"),
               nPt,  binsPt.data(),
               nPhi, binsPhi.data());

    // (Rho, Pt)
    hist_.p2RecoObjPtOverGenObjPtInRhoRecoObjPt =
        makeP2(name4("p2Reco", Obj, "PtOverGen",
                     Obj + "PtInRhoReco" + Obj + "Pt"),
               nPt,  binsPt.data(),
               nRho, binsRho.data());

    //---------- GenObj
    // =========================================================
    // Profile 1D: ratio vs Reco{Obj}{Var}
    // =========================================================
    hist_.p1RecoObjPtOverGenObjPtInGenObjPt  =
        makeP1(name4("p1Reco", Obj, "PtOverGen", Obj + "PtInGen" + Obj + "Pt"),
               nPt, binsPt.data());

    hist_.p1RecoObjPtOverGenObjPtInGenObjEta =
        makeP1(name4("p1Reco", Obj, "PtOverGen", Obj + "PtInGen" + Obj + "Eta"),
               nEta, binsEta.data());

    hist_.p1RecoObjPtOverGenObjPtInGenObjPhi =
        makeP1(name4("p1Reco", Obj, "PtOverGen", Obj + "PtInGen" + Obj + "Phi"),
               nPhi, binsPhi.data());

    hist_.p1RecoObjPtOverGenObjPtInRho =
        makeP1(name4("p1Reco", Obj, "PtOverGen", Obj + "PtInRho"),
               nRho, binsRho.data());

    // =========================================================
    // Profile 2D: ratio vs (Reco{Obj}{X}, Reco{Obj}{Y})
    // NOTE: TProfile2D::Fill(x, y, z, w)  (z=ratio here)
    // =========================================================

    // (Pt, Eta)
    hist_.p2RecoObjPtOverGenObjPtInGenObjPtGenObjEta =
        makeP2(name4("p2Reco", Obj, "PtOverGen",
                     Obj + "PtInGen" + Obj + "PtGen" + Obj + "Eta"),
               nEta, binsEta.data(),
               nPt,  binsPt.data());

    // (Phi, Eta)
    hist_.p2RecoObjPtOverGenObjPtInGenObjPhiGenObjEta =
        makeP2(name4("p2Reco", Obj, "PtOverGen",
                     Obj + "PtInGen" + Obj + "PhiGen" + Obj + "Eta"),
               nEta, binsEta.data(),
               nPhi, binsPhi.data());

    // (Rho, Eta)
    hist_.p2RecoObjPtOverGenObjPtInRhoGenObjEta =
        makeP2(name4("p2Reco", Obj, "PtOverGen",
                     Obj + "PtInRhoGen" + Obj + "Eta"),
               nEta, binsEta.data(),
               nRho, binsRho.data());

    // (Eta, Pt)
    hist_.p2RecoObjPtOverGenObjPtInGenObjEtaGenObjPt =
        makeP2(name4("p2Reco", Obj, "PtOverGen",
                     Obj + "PtInGen" + Obj + "EtaGen" + Obj + "Pt"),
               nPt,  binsPt.data(),
               nEta, binsEta.data());

    // (Phi, Pt)
    hist_.p2RecoObjPtOverGenObjPtInGenObjPhiGenObjPt =
        makeP2(name4("p2Reco", Obj, "PtOverGen",
                     Obj + "PtInGen" + Obj + "PhiGen" + Obj + "Pt"),
               nPt,  binsPt.data(),
               nPhi, binsPhi.data());

    // (Rho, Pt)
    hist_.p2RecoObjPtOverGenObjPtInRhoGenObjPt =
        makeP2(name4("p2Reco", Obj, "PtOverGen",
                     Obj + "PtInRhoGen" + Obj + "Pt"),
               nPt,  binsPt.data(),
               nRho, binsRho.data());

    // Profile 2D: GenPt vs RecoEta
    hist_.p2RecoObjPtOverGenObjPtInGenObjPtRecoObjEta =
        makeP2(name4("p2Reco", Obj, "PtOverGen",
                     Obj + "PtInGen" + Obj + "PtReco" + Obj + "Eta"),
               nEta, binsEta.data(),
               nPt,  binsPt.data());
    std::cout << "[HistObjJER] Initialized " << dirTag_
              << " in directory: " << dirName << "\n";

    origDir->cd();
}

void HistObjJER::Fill(TLorentzVector p4Gen, TLorentzVector p4Reco, double rho, double weight)
{
    const double ptReco  = p4Reco.Pt();
    const double etaReco = p4Reco.Eta();
    const double phiReco = p4Reco.Phi();

    const double ptGen  = p4Gen.Pt();
    const double etaGen = p4Gen.Eta();
    const double phiGen = p4Gen.Phi();

    if (ptGen <= 0.0)  return;
    if (ptReco <= 0.0) return;

    const double ratio = ptReco / ptGen;
    //const double ratio = ptGen / ptReco;

    if(hist_.h1EventInRecoObjPt) hist_.h1EventInRecoObjPt->Fill(ptReco, weight);
    if(hist_.h1EventInGenObjPt) hist_.h1EventInGenObjPt->Fill(ptGen, weight);

    // Ratio distribution
    if (hist_.h1EventInRecoObjPtOverGenObjPt)
        hist_.h1EventInRecoObjPtOverGenObjPt->Fill(ratio, weight);


    //---------- RecoObj
    // -------------------------
    // 1D profiles
    // -------------------------
    if (hist_.p1RecoObjPtOverGenObjPtInRecoObjPt)
        hist_.p1RecoObjPtOverGenObjPtInRecoObjPt->Fill(ptReco, ratio, weight);

    if (hist_.p1RecoObjPtOverGenObjPtInRecoObjEta)
        hist_.p1RecoObjPtOverGenObjPtInRecoObjEta->Fill(etaReco, ratio, weight);

    if (hist_.p1RecoObjPtOverGenObjPtInRecoObjPhi)
        hist_.p1RecoObjPtOverGenObjPtInRecoObjPhi->Fill(phiReco, ratio, weight);

    if (hist_.p1RecoObjPtOverGenObjPtInRho)
        hist_.p1RecoObjPtOverGenObjPtInRho->Fill(rho, ratio, weight);

    // -------------------------
    // 2D profiles
    // TProfile2D::Fill(x, y, z, w) with z=ratio
    // -------------------------

    if (hist_.p2RecoObjPtOverGenObjPtInRecoObjPtRecoObjEta)
        hist_.p2RecoObjPtOverGenObjPtInRecoObjPtRecoObjEta->Fill(etaReco, ptReco, ratio, weight);

    if (hist_.p2RecoObjPtOverGenObjPtInRecoObjPhiRecoObjEta)
        hist_.p2RecoObjPtOverGenObjPtInRecoObjPhiRecoObjEta->Fill(etaReco, phiReco, ratio, weight);

    if (hist_.p2RecoObjPtOverGenObjPtInRhoRecoObjEta)
        hist_.p2RecoObjPtOverGenObjPtInRhoRecoObjEta->Fill(etaReco, rho, ratio, weight);

    if (hist_.p2RecoObjPtOverGenObjPtInRecoObjEtaRecoObjPt)
        hist_.p2RecoObjPtOverGenObjPtInRecoObjEtaRecoObjPt->Fill(ptReco, etaReco, ratio, weight);

    if (hist_.p2RecoObjPtOverGenObjPtInRecoObjPhiRecoObjPt)
        hist_.p2RecoObjPtOverGenObjPtInRecoObjPhiRecoObjPt->Fill(ptReco, phiReco, ratio, weight);

    if (hist_.p2RecoObjPtOverGenObjPtInRhoRecoObjPt)
        hist_.p2RecoObjPtOverGenObjPtInRhoRecoObjPt->Fill(ptReco, rho, ratio, weight);


    //---------- GenObj
    // -------------------------
    // 1D profiles
    // -------------------------
    if (hist_.p1RecoObjPtOverGenObjPtInGenObjPt)
        hist_.p1RecoObjPtOverGenObjPtInGenObjPt->Fill(ptGen, ratio, weight);

    if (hist_.p1RecoObjPtOverGenObjPtInGenObjEta)
        hist_.p1RecoObjPtOverGenObjPtInGenObjEta->Fill(etaGen, ratio, weight);

    if (hist_.p1RecoObjPtOverGenObjPtInGenObjPhi)
        hist_.p1RecoObjPtOverGenObjPtInGenObjPhi->Fill(phiGen, ratio, weight);

    // -------------------------
    // 2D profiles
    // TProfile2D::Fill(x, y, z, w) with z=ratio
    // -------------------------

    if (hist_.p2RecoObjPtOverGenObjPtInGenObjPtGenObjEta)
        hist_.p2RecoObjPtOverGenObjPtInGenObjPtGenObjEta->Fill(etaGen, ptGen, ratio, weight);

    if (hist_.p2RecoObjPtOverGenObjPtInGenObjPhiGenObjEta)
        hist_.p2RecoObjPtOverGenObjPtInGenObjPhiGenObjEta->Fill(etaGen, phiGen, ratio, weight);

    if (hist_.p2RecoObjPtOverGenObjPtInRhoGenObjEta)
        hist_.p2RecoObjPtOverGenObjPtInRhoGenObjEta->Fill(etaGen, rho, ratio, weight);

    if (hist_.p2RecoObjPtOverGenObjPtInGenObjEtaGenObjPt)
        hist_.p2RecoObjPtOverGenObjPtInGenObjEtaGenObjPt->Fill(ptGen, etaGen, ratio, weight);

    if (hist_.p2RecoObjPtOverGenObjPtInGenObjPhiGenObjPt)
        hist_.p2RecoObjPtOverGenObjPtInGenObjPhiGenObjPt->Fill(ptGen, phiGen, ratio, weight);

    if (hist_.p2RecoObjPtOverGenObjPtInRhoGenObjPt)
        hist_.p2RecoObjPtOverGenObjPtInRhoGenObjPt->Fill(ptGen, rho, ratio, weight);


    if (hist_.p2RecoObjPtOverGenObjPtInGenObjPtRecoObjEta)
        hist_.p2RecoObjPtOverGenObjPtInGenObjPtRecoObjEta->Fill(etaReco, ptGen, ratio, weight);

}

