#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class TDirectory;
class TH1D;
class VarBin;
class ScaleMet;

class HistScaleMet {
public:
    HistScaleMet(TDirectory* origDir, const std::string& directoryName, const VarBin& varBin);
    ~HistScaleMet();

    void Fill(const ScaleMet& scaleMet);

private:
    std::unordered_map<std::string, std::unique_ptr<TH1D>> histMetPt_;
    std::unordered_map<std::string, std::unique_ptr<TH1D>> histMetPhi_;

    std::vector<std::string> corrNamesMet_;

    void InitializeHistograms(TDirectory* origDir, const std::string& directoryName, const VarBin& varBin);
};

