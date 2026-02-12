#pragma once

#include <string>
#include <algorithm>   // std::clamp
#include <cmath>

#include "GlobalFlag.h"
#include "SkimTree.h"
#include "ScaleMuonLoader.h"

class TH2;
class TAxis;

class ScaleMuonFunction {
public:
    enum class SystLevel : int { Down = 0, Nominal = 1, Up = 2 };

    struct MuSf {
        double total{1.0};
        double id{1.0};
        double iso{1.0};
        double trig{1.0};
    };

    explicit ScaleMuonFunction(const GlobalFlag& globalFlags);

    // -------- Rochester (kinematics) --------
    double getMuonRochCorrection(const SkimTree& skimT, int index, const std::string& syst) const;

    // -------- SFs (weights) --------
    double getMuonIdSf(double pt, double eta, SystLevel syst = SystLevel::Nominal)  const;
    double getMuonIsoSf(double pt, double eta, SystLevel syst = SystLevel::Nominal) const;
    double getMuonTrigSf(double pt, double eta, SystLevel syst = SystLevel::Nominal) const;

    MuSf getMuonSfs(const SkimTree& skimT, int index, SystLevel syst = SystLevel::Nominal) const;
    MuSf getMuonSfs(const SkimTree& skimT, int index, const std::string& systStr) const;

private:
    double getSfFromHist(const TH2* h2, double pt, double eta, SystLevel syst) const;
    static SystLevel parseSyst(const std::string& s);

private:
    ScaleMuonLoader loader_;  // composition: Loader used ONLY here

    const GlobalFlag& globalFlags_;
    const bool isDebug_;
    const bool isData_;
    const bool isMC_;
};

