#include "HistJecUncBand.h"
#include "HelperDir.hpp"
#include "VarBin.h"

#include <TDirectory.h>
#include <TProfile.h>
#include <TProfile2D.h>

HistJecUncBand::~HistJecUncBand() = default;

HistJecUncBand::HistJecUncBand(TDirectory* origDir,
                               const std::string& directoryName,
                               const VarBin& varBin,
                               const std::string& objKey)
    : obj_(objKey)
{
    const std::string dirName = directoryName + "/HistJecUncBand_"+obj_;
    InitializeHistograms(origDir, dirName, varBin);
}

void HistJecUncBand::InitializeHistograms(TDirectory* origDir,
                                          const std::string& directoryName,
                                          const VarBin& varBin)
{
    TDirectory* newDir = HelperDir::createTDirectory(origDir, directoryName);
    newDir->cd();

    const auto& binsPt  = varBin.getBinsPt();
    const auto& binsEta = varBin.getBinsEta();

    const std::string Obj = obj_; // "Jet"
    // ---------------- JES
    p1JesUncInObjPt_ = std::make_unique<TProfile>(
        ("p1JesRelUncIn" + Obj + "Pt").c_str(), "",
        binsPt.size() - 1, binsPt.data());

    p1JesUncInObjEta_ = std::make_unique<TProfile>(
        ("p1JesRelUncIn" + Obj + "Eta").c_str(), "",
        binsEta.size() - 1, binsEta.data());

    p2JesUncInObjPtObjEta_ = std::make_unique<TProfile2D>(
        ("p2JesRelUncIn" + Obj + "Pt"+ Obj+ "Eta").c_str(), "",
        binsEta.size() - 1, binsEta.data(),
        binsPt.size()  - 1, binsPt.data());

    // ---------------- JER
    p1JerSfUncInObjPt_ = std::make_unique<TProfile>(
        ("p1JerSfRelUncIn" + Obj + "Pt").c_str(), "",
        binsPt.size() - 1, binsPt.data());

    p1JerSfUncInObjEta_ = std::make_unique<TProfile>(
        ("p1JerSfRelUncIn" + Obj + "Eta").c_str(), "",
        binsEta.size() - 1, binsEta.data());

    p2JerSfUncInObjPtObjEta_ = std::make_unique<TProfile2D>(
        ("p2JerSfRelUncIn" + Obj+ "Pt"+ Obj+ "Eta").c_str() , "",
        binsEta.size() - 1, binsEta.data(),
        binsPt.size()  - 1, binsPt.data());

    origDir->cd();
}

void HistJecUncBand::Fill(double eta, double pt,
                          double jesRelUnc,
                          double jerRelUnc)
{
    // ---------------- JES
    p1JesUncInObjPt_     ->Fill(pt,  jesRelUnc);
    p1JesUncInObjEta_    ->Fill(eta, jesRelUnc);
    p2JesUncInObjPtObjEta_  ->Fill(eta, pt, jesRelUnc);

    // ---------------- JER
    p1JerSfUncInObjPt_     ->Fill(pt,  jerRelUnc);
    p1JerSfUncInObjEta_    ->Fill(eta, jerRelUnc);
    p2JerSfUncInObjPtObjEta_  ->Fill(eta, pt, jerRelUnc);
}

