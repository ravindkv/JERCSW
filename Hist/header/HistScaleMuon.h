#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class TDirectory;
class TH1D;
class VarBin;
class ScaleMuon;

class HistScaleMuon {
public:
    HistScaleMuon(TDirectory* origDir, const std::string& directoryName, const VarBin& varBin);
    ~HistScaleMuon();

    void Fill(const ScaleMuon& scaleMuon);

private:
    std::unordered_map<std::string, std::unique_ptr<TH1D>> histMuon1Pt_;
    std::vector<std::string> corrNames_;

    void InitializeHistograms(TDirectory* origDir, const std::string& directoryName, const VarBin& varBin);
};

