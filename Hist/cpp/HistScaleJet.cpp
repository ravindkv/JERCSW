#include "HistScaleJet.h"
#include "HelperDir.hpp"
#include "HistGuard.hpp"

#include <cmath>
#include <iostream>
#include <unordered_set>

#include <TH1D.h>
#include <TDirectory.h>

#include "VarBin.h"
#include "ScaleJet.h"

HistScaleJet::~HistScaleJet() = default;

HistScaleJet::HistScaleJet(TDirectory* origDir, const std::string& directoryName, const VarBin& varBin) {
    InitializeHistograms(origDir, directoryName, varBin);
}

void HistScaleJet::InitializeHistograms(TDirectory* origDir, const std::string& directoryName, const VarBin& varBin)
{
    const std::string dirName = directoryName + "/HistScaleJet";
    TDirectory* newDir = HelperDir::createTDirectory(origDir, dirName);
    newDir->cd();

    const std::vector<double> binsPt = varBin.getBinsPt();
    const int nPt = static_cast<int>(binsPt.size()) - 1;

    // Keep same naming set as before (match ScaleJet keys!)
    jecNames_ = {"Nano","Raw","L1RcCorr","L2RelCorr","L2ResCorr","L2L3ResCorr","JerCorr","Corr"};

    for (const auto& corrName : jecNames_) {
        histJet1Pt_[corrName] = std::make_unique<TH1D>(
            ("h1EventInJet1Pt" + corrName).c_str(), "", nPt, binsPt.data());
        histJetSumPt_[corrName] = std::make_unique<TH1D>(
            ("h1EventInJetSumPt" + corrName).c_str(), "", nPt, binsPt.data());
    }

    std::cout << "Initialized HistScaleJet histograms in directory: " << dirName << std::endl;
    origDir->cd();
}

void HistScaleJet::Fill(const ScaleJet& scaleJet)
{
    static constexpr const char* kTag = "HistScaleJet";
    const auto& p4MapJet1   = scaleJet.getP4MapJet1();
    const auto& p4MapJetSum = scaleJet.getP4MapJetSum();

    for (const auto& corrName : jecNames_) {
        // Jet1
        auto it1 = p4MapJet1.find(corrName);
        if (it1 == p4MapJet1.end()) {
            HistGuard::warnOnce(kTag, "Missing Jet1 p4 for corr='" + corrName + "'");
        } else {
            HistGuard::safeFill(HistGuard::getHistOrWarn(histJet1Pt_, corrName, kTag, "Jet1Pt"),
                     it1->second.Pt(), kTag, "Jet1Pt(" + corrName + ")");
        }

        // JetSum
        auto itS = p4MapJetSum.find(corrName);
        if (itS == p4MapJetSum.end()) {
            HistGuard::warnOnce(kTag, "Missing JetSum p4 for corr='" + corrName + "'");
        } else {
            HistGuard::safeFill(HistGuard::getHistOrWarn(histJetSumPt_, corrName, kTag, "JetSumPt"),
                     itS->second.Pt(), kTag,  "JetSumPt(" + corrName + ")");
        }
    }
}

