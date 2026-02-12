#include "HistScaleElectron.h"
#include "HelperDir.hpp"
#include "HistGuard.hpp"

#include <cmath>
#include <iostream>
#include <unordered_set>

#include <TH1D.h>
#include <TDirectory.h>

#include "VarBin.h"
#include "ScaleElectron.h"

HistScaleElectron::~HistScaleElectron() = default;

HistScaleElectron::HistScaleElectron(TDirectory* origDir, const std::string& directoryName, const VarBin& varBin) {
    InitializeHistograms(origDir, directoryName, varBin);
}

void HistScaleElectron::InitializeHistograms(TDirectory* origDir, const std::string& directoryName, const VarBin& varBin)
{
    const std::string dirName = directoryName + "/HistScaleElectron";
    TDirectory* newDir = HelperDir::createTDirectory(origDir, dirName);
    newDir->cd();

    const std::vector<double> binsPt = varBin.getBinsPt();
    const int nPt = static_cast<int>(binsPt.size()) - 1;

    corrNames_ = {"Nano", "Corr"};

    for (const auto& corrName : corrNames_) {
        const std::string histNamePt = "h1EventInElectron1Pt" + corrName;
        histElectron1Pt_[corrName] = std::make_unique<TH1D>(histNamePt.c_str(), "", nPt, binsPt.data());
    }

    std::cout << "Initialized HistScaleElectron histograms in directory: " << dirName << std::endl;
    origDir->cd();
}

void HistScaleElectron::Fill(const ScaleElectron& scaleElectron)
{
    static constexpr const char* kTag = "HistScaleElectron";
    const auto& p4Map = scaleElectron.getP4MapElectron1();

    for (const auto& corrName : corrNames_) {
        auto it = p4Map.find(corrName);
        if (it == p4Map.end()) {
            HistGuard::warnOnce(kTag, "Missing Electron1 p4 for corr='" + corrName + "'");
            continue;
        }
        TH1D* h = HistGuard::getHistOrWarn(histElectron1Pt_, corrName, kTag, "Electron1Pt");
        HistGuard::safeFill(h, it->second.Pt(), kTag, "Electron1Pt(" + corrName + ")");
    }
}

