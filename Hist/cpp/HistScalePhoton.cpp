#include "HistScalePhoton.h"
#include "HelperDir.hpp"
#include "HistGuard.hpp"

#include <cmath>
#include <iostream>
#include <unordered_set>

#include <TH1D.h>
#include <TDirectory.h>

#include "VarBin.h"
#include "ScalePhoton.h"

HistScalePhoton::~HistScalePhoton() = default;

HistScalePhoton::HistScalePhoton(TDirectory* origDir, const std::string& directoryName, const VarBin& varBin) {
    InitializeHistograms(origDir, directoryName, varBin);
}

void HistScalePhoton::InitializeHistograms(TDirectory* origDir, const std::string& directoryName, const VarBin& varBin)
{
    const std::string dirName = directoryName + "/HistScalePhoton";
    TDirectory* newDir = HelperDir::createTDirectory(origDir, dirName);
    newDir->cd();

    const std::vector<double> binsPt = varBin.getBinsPt();
    const int nPt = static_cast<int>(binsPt.size()) - 1;

    corrNames_ = {"Nano", "Corr"};

    for (const auto& corrName : corrNames_) {
        const std::string histNamePt = "h1EventInPhoton1Pt" + corrName;
        histPhoton1Pt_[corrName] = std::make_unique<TH1D>(histNamePt.c_str(), "", nPt, binsPt.data());
    }

    std::cout << "Initialized HistScalePhoton histograms in directory: " << dirName << std::endl;
    origDir->cd();
}

void HistScalePhoton::Fill(const ScalePhoton& scalePhoton)
{
    static constexpr const char* kTag = "HistScalePhoton";
    const auto& p4Map = scalePhoton.getP4MapPhoton1();

    for (const auto& corrName : corrNames_) {
        auto it = p4Map.find(corrName);
        if (it == p4Map.end()) {
            HistGuard::warnOnce(kTag, "Missing Photon1 p4 for corr='" + corrName + "'");
            continue;
        }
        TH1D* h = HistGuard::getHistOrWarn(histPhoton1Pt_, corrName, kTag, "Photon1Pt");
        HistGuard::safeFill(h, it->second.Pt(), kTag, "Photon1Pt(" + corrName + ")");
    }
}

