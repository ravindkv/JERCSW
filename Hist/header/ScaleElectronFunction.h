#pragma once

#include <string>
#include <algorithm> // std::clamp

#include "GlobalFlag.h"
#include "SkimTree.h"
#include "ScaleElectronLoader.h"

// ROOT forward declarations
class TH2;
class TAxis;

class ScaleElectronFunction {
public:
    // keep same convention
    enum class SystLevel : int { Down = 0, Nominal = 1, Up = 2 };

    struct EleSf {
        double total{1.0};
        double id{1.0};
        double reco{1.0};
        double trig{1.0};
    };

    explicit ScaleElectronFunction(const GlobalFlag& globalFlags);

    // --------- Electron Scale and Smearing (Ss): ONLY Syst. Nominal is already in Nano ---------
    double getElectronSsCorrection(const SkimTree& skimT, int index, const std::string& syst) const;

    // --------- Electron SFs (ID / Reco / Trig) ----------
    double getElectronIdSf(double pt, double eta, SystLevel syst = SystLevel::Nominal) const;
    double getElectronRecoSf(double pt, double eta, SystLevel syst = SystLevel::Nominal) const;
    double getElectronTrigSf(double pt, double eta, SystLevel syst = SystLevel::Nominal) const;

    EleSf getElectronSfs(const SkimTree& skimT, int index, SystLevel syst = SystLevel::Nominal) const;
    EleSf getElectronSfs(const SkimTree& skimT, int index, const std::string& systStr) const;

private:
    double getSfFromHist(const TH2* h2, double pt, double eta, SystLevel syst) const;
    static SystLevel parseSyst(const std::string& s);

private:
    ScaleElectronLoader loader_;     // composition: Loader is used ONLY here
    const GlobalFlag& globalFlags_;
    const bool isDebug_;
    const bool isData_;
    const bool isMC_;
};

