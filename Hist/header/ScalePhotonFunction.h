#pragma once

#include <string>
#include <algorithm> // std::clamp
#include <cmath>

#include "GlobalFlag.h"
#include "SkimTree.h"
#include "ScalePhotonLoader.h"

// ROOT fwd decls
class TH2;
class TH1F;
class TAxis;

class ScalePhotonFunction {
public:
    enum class SystLevel : int { Down = 0, Nominal = 1, Up = 2 };

    struct PhotonSf {
        double total{1.0};
        double id{1.0};
        double ps{1.0};
        double cs{1.0};
    };

    explicit ScalePhotonFunction(const GlobalFlag& globalFlags);

    // -------- Scale/Smear (kinematics) via correctionlib --------
    double getPhotonScaleCorrection(const SkimTree& skimT, const std::string& nomOrSyst, int index) const;
    double getPhotonSmearCorrection(const SkimTree& skimT, const std::string& nomOrSyst, int index) const;

    // -------- SFs (weights) --------
    double getPhotonIdSf(double pt, double eta, SystLevel syst = SystLevel::Nominal) const;
    double getPhotonPsSf(double pt, double eta, SystLevel syst = SystLevel::Nominal) const;
    double getPhotonCsSf(double pt, double eta, SystLevel syst = SystLevel::Nominal) const;

    PhotonSf getPhotonSfs(const SkimTree& skimT, int index, SystLevel syst = SystLevel::Nominal) const;
    PhotonSf getPhotonSfs(const SkimTree& skimT, int index, const std::string& systStr) const;

private:
    static SystLevel parseSyst(const std::string& s);

    double getSfFromHist2D(const TH2* h2, double pt, double eta, SystLevel syst) const ;
    double getSfFromHist1D(const TH1F* h1, double pt, double eta, SystLevel syst) const;

private:
    ScalePhotonLoader loader_; // composition: Loader used ONLY here

    const GlobalFlag& globalFlags_;
    const bool isDebug_;
    const bool isData_;
    const bool isMC_;
};

