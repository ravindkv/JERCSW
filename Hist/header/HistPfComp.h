#pragma once

#include <string>
#include <memory>

// forward‑declare all types we only hold pointers to
class TProfile;
class TDirectory;
class SkimTree;
class VarBin;

struct PfCompHistograms {
    std::unique_ptr<TProfile> p1ChfInPt;
    std::unique_ptr<TProfile> p1NhfInPt;
    std::unique_ptr<TProfile> p1NefInPt;
    std::unique_ptr<TProfile> p1CefInPt;
    std::unique_ptr<TProfile> p1MufInPt;

    std::unique_ptr<TProfile> p1ChfInEta;
    std::unique_ptr<TProfile> p1NhfInEta;
    std::unique_ptr<TProfile> p1NefInEta;
    std::unique_ptr<TProfile> p1CefInEta;
    std::unique_ptr<TProfile> p1MufInEta;
};

class HistPfComp {
public:
    HistPfComp(TDirectory *origDir, const std::string& directoryName, const VarBin& varBin, const std::string& ofJet, const std::string& inObj);
    
    ~HistPfComp();
    
    void Fill(SkimTree* skimT, int iOfJet, double inObjPt, double inObjEta, double weight);
    
private:
    PfCompHistograms hist_;
    std::string ofJet_;  
    std::string inObj_;  

    void InitializeHistograms(TDirectory *origDir, const std::string& directoryName, const VarBin& varBin);
};

