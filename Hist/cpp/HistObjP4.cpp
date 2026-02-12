#include "HistObjP4.h"
#include "HelperDir.hpp"
#include "VarBin.h"

#include <TH1D.h>
#include <TProfile.h>
#include <TLorentzVector.h>

#include <iostream>
#include <vector>

HistObjP4::~HistObjP4() = default;

HistObjP4::HistObjP4(TDirectory *origDir,
                     const std::string& directoryName,
                     const VarBin& varBin,
                     const std::string& objKey)
    : obj_(objKey),
      dirTag_("HistObjP4_" + objKey)
{
    InitializeHistograms(origDir, directoryName, varBin);
}

void HistObjP4::InitializeHistograms(TDirectory *origDir,
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

    // pT binning from VarBin
    std::vector<double> binsPt  = varBin.getBinsPt();
    std::vector<double> binsEta = varBin.getBinsEta();
    std::vector<double> binsPhi = varBin.getBinsPhi();
    std::vector<double> binsMass = varBin.getBinsMass();

    const int nPt  = binsPt.size()  - 1;
    const int nEta = binsEta.size() - 1;
    const int nPhi = binsPhi.size() - 1;
    const int nMass= binsMass.size() - 1;

    const std::string Obj = obj_;  // "Jet", "Pho", "Ele", ...

    // --- 1D histograms ---

    // pT with VarBin binning
    hist_.h1ObjPt = std::make_unique<TH1D>(
        ("h1EventIn" + Obj + "Pt").c_str(), "", nPt, binsPt.data());
    hist_.h1ObjPt->Sumw2();

    // eta (fixed binning, tweak if you want)
    hist_.h1ObjEta = std::make_unique<TH1D>(
        ("h1EventIn" + Obj + "Eta").c_str(), "", nEta, binsEta.data());
    hist_.h1ObjEta->Sumw2();

    // phi (fixed binning, typical -pi..pi)
    hist_.h1ObjPhi = std::make_unique<TH1D>(
        ("h1EventIn" + Obj + "Phi").c_str(), "", nPhi, binsPhi.data());
    hist_.h1ObjPhi->Sumw2();

    // mass (generic 0–200 GeV range, adjust per analysis if needed)
    hist_.h1ObjMass = std::make_unique<TH1D>(
        ("h1EventIn" + Obj + "Mass").c_str(), "", nMass, binsMass.data());
    hist_.h1ObjMass->Sumw2();

    // --- Profile: mass vs pT ---
    hist_.p1ObjMassInObjPt = std::make_unique<TProfile>(
        ("p1" + Obj + "MassIn" + Obj + "Pt").c_str(), "",
        nPt, binsPt.data());

    std::cout << "Initialized " << dirTag_
              << " histograms in directory: " << dirName << std::endl;

    origDir->cd();
}

void HistObjP4::Fill(const TLorentzVector& p4Obj, double weight)
{
    const double pt   = p4Obj.Pt();
    const double eta  = p4Obj.Eta();
    const double phi  = p4Obj.Phi();  // TLorentzVector already returns in (-pi,pi)
    const double mass = p4Obj.M();

    // Basic kinematics
    hist_.h1ObjPt  ->Fill(pt,   weight);
    hist_.h1ObjEta ->Fill(eta,  weight);
    hist_.h1ObjPhi ->Fill(phi,  weight);
    hist_.h1ObjMass->Fill(mass, weight);

    // Profile of mass vs pT
    if (pt > 0.0) {
        hist_.p1ObjMassInObjPt->Fill(pt, mass, weight);
    }
}

