#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class TDirectory;
class TH1D;
class VarBin;
class ScalePhoton;

class HistScalePhoton {
public:
    HistScalePhoton(TDirectory* origDir, const std::string& directoryName, const VarBin& varBin);
    ~HistScalePhoton();

    void Fill(const ScalePhoton& scalePhoton);

private:
    std::unordered_map<std::string, std::unique_ptr<TH1D>> histPhoton1Pt_;
    std::vector<std::string> corrNames_;

    void InitializeHistograms(TDirectory* origDir, const std::string& directoryName, const VarBin& varBin);
};

