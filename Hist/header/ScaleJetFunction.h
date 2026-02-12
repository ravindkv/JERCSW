#pragma once

#include <string>
#include "SkimTree.h"
#include "GlobalFlag.h"
#include "ScaleJetLoader.h"

class ScaleJetFunction {
public:
    ScaleJetFunction(const GlobalFlag& globalFlags);

    double getL1FastJetCorrection(double jetArea, double jetEta, double jetPt, double rho) const;
    double getL2RelativeCorrection(double jetEta, double jetPt) const;
    double getL2ResidualCorrection(double jetEta, double jetPt) const;
    double getL2L3ResidualCorrection(double jetEta, double jetPt) const;

    double getJerResolution(const SkimTree& skimT, int index) const;
    double getJerScaleFactor(const SkimTree& skimT, int index, const std::string& syst) const;
    double getJerCorrection(const SkimTree& skimT, int index, const std::string& syst, double pt_corr) const;

private:
    ScaleJetLoader loader_;
    const GlobalFlag& globalFlags_;
    const GlobalFlag::JetAlgo jetAlgo_;
    const bool isDebug_;
};

