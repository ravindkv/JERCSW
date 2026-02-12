#include "HistScaleMet.h"
#include "HelperDir.hpp"
#include "HistGuard.hpp"

#include <cmath>
#include <iostream>
#include <unordered_set>

#include <TH1D.h>
#include <TDirectory.h>

#include "VarBin.h"
#include "ScaleMet.h"

HistScaleMet::~HistScaleMet() = default;

HistScaleMet::HistScaleMet(TDirectory* origDir, const std::string& directoryName, const VarBin& varBin) {
    InitializeHistograms(origDir, directoryName, varBin);
}

void HistScaleMet::InitializeHistograms(TDirectory* origDir, const std::string& directoryName, const VarBin& varBin)
{
    const std::string dirName = directoryName + "/HistScaleMet";
    TDirectory* newDir = HelperDir::createTDirectory(origDir, dirName);
    newDir->cd();

    const std::vector<double> binsPt  = varBin.getBinsPt();
    const std::vector<double> binsPhi = varBin.getBinsPhi();
    const int nPt  = static_cast<int>(binsPt.size())  - 1;
    const int nPhi = static_cast<int>(binsPhi.size()) - 1;

    corrNamesMet_ = {"Raw", "Nano", "Corr"};

    for (const auto& corrName : corrNamesMet_) {
        histMetPt_[corrName] = std::make_unique<TH1D>(
            ("h1EventInMetPt" + corrName).c_str(), "", nPt, binsPt.data());

        histMetPhi_[corrName] = std::make_unique<TH1D>(
            ("h1EventInMetPhi" + corrName).c_str(), "", nPhi, binsPhi.data());
    }

    std::cout << "Initialized HistScaleMet histograms in directory: " << dirName << std::endl;
    origDir->cd();
}

void HistScaleMet::Fill(const ScaleMet& scaleMet)
{
    static constexpr const char* kTag = "HistScaleMet";
    const auto& p4MapMet = scaleMet.getP4MapMet();

    for (const auto& corrName : corrNamesMet_) {
        auto it = p4MapMet.find(corrName);
        if (it == p4MapMet.end()) {
            HistGuard::warnOnce(kTag, "Missing MET p4 for corr='" + corrName + "'");
            continue;
        }
        const auto& p4Met = it->second;

        HistGuard::safeFill(HistGuard::getHistOrWarn(histMetPt_,  corrName, kTag, "MetPt"),  p4Met.Pt(),  kTag, "MetPt("  + corrName + ")");
        HistGuard::safeFill(HistGuard::getHistOrWarn(histMetPhi_, corrName, kTag, "MetPhi"), p4Met.Phi(), kTag, "MetPhi(" + corrName + ")");
    }
}

