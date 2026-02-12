#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class TDirectory;
class TH1D;
class VarBin;
class ScaleJet;

class HistScaleJet {
public:
    HistScaleJet(TDirectory* origDir, const std::string& directoryName, const VarBin& varBin);
    ~HistScaleJet();

    void Fill(const ScaleJet& scaleJet);

private:
    std::unordered_map<std::string, std::unique_ptr<TH1D>> histJet1Pt_;
    std::unordered_map<std::string, std::unique_ptr<TH1D>> histJetSumPt_;

    std::vector<std::string> jecNames_;

    void InitializeHistograms(TDirectory* origDir, const std::string& directoryName, const VarBin& varBin);
};

