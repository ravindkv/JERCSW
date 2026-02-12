#include "HistScaleMuon.h"
#include "HelperDir.hpp"
#include "HistGuard.hpp"

#include <cmath>
#include <iostream>
#include <unordered_set>

#include <TH1D.h>
#include <TDirectory.h>

#include "VarBin.h"
#include "ScaleMuon.h"

HistScaleMuon::~HistScaleMuon() = default;

HistScaleMuon::HistScaleMuon(TDirectory* origDir, const std::string& directoryName, const VarBin& varBin) {
    InitializeHistograms(origDir, directoryName, varBin);
}

void HistScaleMuon::InitializeHistograms(TDirectory* origDir, const std::string& directoryName, const VarBin& varBin)
{
    const std::string dirName = directoryName + "/HistScaleMuon";
    TDirectory* newDir = HelperDir::createTDirectory(origDir, dirName);
    newDir->cd();

    const std::vector<double> binsPt = varBin.getBinsPt();
    const int nPt = static_cast<int>(binsPt.size()) - 1;

    corrNames_ = {"Nano", "Corr"};

    for (const auto& corrName : corrNames_) {
        const std::string histNamePt = "h1EventInMuon1Pt" + corrName;
        histMuon1Pt_[corrName] = std::make_unique<TH1D>(histNamePt.c_str(), "", nPt, binsPt.data());
    }

    std::cout << "Initialized HistScaleMuon histograms in directory: " << dirName << std::endl;
    origDir->cd();
}

void HistScaleMuon::Fill(const ScaleMuon& scaleMuon)
{
    static constexpr const char* kTag = "HistScaleMuon";
    const auto& p4Map = scaleMuon.getP4MapMuon1();

    for (const auto& corrName : corrNames_) {
        auto it = p4Map.find(corrName);
        if (it == p4Map.end()) {
            HistGuard::warnOnce(kTag, "Missing Muon1 p4 for corr='" + corrName + "'");
            continue;
        }
        TH1D* h = HistGuard::getHistOrWarn(histMuon1Pt_, corrName, kTag, "Muon1Pt");
        HistGuard::safeFill(h, it->second.Pt(), kTag, "Muon1Pt(" + corrName + ")");
    }
}

