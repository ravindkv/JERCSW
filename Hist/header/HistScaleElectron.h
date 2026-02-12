#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class TDirectory;
class TH1D;
class VarBin;
class ScaleElectron;

class HistScaleElectron {
public:
    HistScaleElectron(TDirectory* origDir, const std::string& directoryName, const VarBin& varBin);
    ~HistScaleElectron();

    void Fill(const ScaleElectron& scaleElectron);

private:
    std::unordered_map<std::string, std::unique_ptr<TH1D>> histElectron1Pt_;
    std::vector<std::string> corrNames_; // e.g. Nano, Corr

    void InitializeHistograms(TDirectory* origDir, const std::string& directoryName, const VarBin& varBin);
};

