#pragma once

#include <memory>
#include <string>

class TDirectory;
class TProfile;
class TProfile2D;
class VarBin;

class HistJecUncBand {
public:
    HistJecUncBand(TDirectory* origDir,
                   const std::string& directoryName,
                   const VarBin& varBin,
                   const std::string& objKey);
    ~HistJecUncBand();

    // Order matches correctionlib: (eta, pt)
    // Values are RELATIVE uncertainties
    void Fill(double eta, double pt,
              double jesRelUnc,
              double jerRelUnc);

private:
    std::string obj_;        // Tag or Probe "Jet" 
    // ---------------- JES
    std::unique_ptr<TProfile>   p1JesUncInObjPt_;
    std::unique_ptr<TProfile>   p1JesUncInObjEta_;
    std::unique_ptr<TProfile2D> p2JesUncInObjPtObjEta_;

    // ---------------- JER
    std::unique_ptr<TProfile>   p1JerSfUncInObjPt_;
    std::unique_ptr<TProfile>   p1JerSfUncInObjEta_;
    std::unique_ptr<TProfile2D> p2JerSfUncInObjPtObjEta_;

    void InitializeHistograms(TDirectory* origDir,
                              const std::string& directoryName,
                              const VarBin& varBin);
};

