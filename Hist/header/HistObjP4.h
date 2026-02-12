#pragma once

#include <memory>
#include <string>

// forward declarations
class TDirectory;
class TH1D;
class TProfile;
class VarBin;
class TLorentzVector;

struct ObjP4Histograms {
    std::unique_ptr<TH1D> h1ObjPt;
    std::unique_ptr<TH1D> h1ObjEta;
    std::unique_ptr<TH1D> h1ObjPhi;
    std::unique_ptr<TH1D> h1ObjMass;

    std::unique_ptr<TProfile> p1ObjMassInObjPt;
};

class HistObjP4 {
public:
    /// objKey = "Jet", "Pho", "Ele", "Mu", ...
    HistObjP4(TDirectory *origDir,
              const std::string& directoryName,
              const VarBin& varBin,
              const std::string& objKey);

    ~HistObjP4();

    /// Fill with TLorentzVector of the chosen object and event weight
    void Fill(const TLorentzVector& p4Obj, double weight);

private:
    ObjP4Histograms hist_;
    std::string obj_;    // "Jet", "Pho", "Ele", ...
    std::string dirTag_; // "HistObjP4_"+obj_

    void InitializeHistograms(TDirectory *origDir,
                              const std::string& baseDir,
                              const VarBin& varBin);
};

