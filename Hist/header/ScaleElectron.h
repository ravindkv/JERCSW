#pragma once

#include <unordered_map>
#include <memory>
#include <string>
#include <vector>

#include "SkimTree.h"
#include "TLorentzVector.h"
#include "GlobalFlag.h"
#include "ScaleElectronFunction.h"

class ScaleElectron {
public:
    // Preserve old public types
    using SystLevel = ScaleElectronFunction::SystLevel;
    using EleSf     = ScaleElectronFunction::EleSf;

    explicit ScaleElectron(const GlobalFlag& globalFlags);

    double getElectronSsCorrection(const SkimTree& skimT, int index, const std::string& syst) const;

    double getElectronIdSf(double pt, double eta, SystLevel syst = SystLevel::Nominal) const;
    double getElectronRecoSf(double pt, double eta, SystLevel syst = SystLevel::Nominal) const;
    double getElectronTrigSf(double pt, double eta, SystLevel syst = SystLevel::Nominal) const;

    EleSf getElectronSfs(const SkimTree& skimT, int index, SystLevel syst = SystLevel::Nominal) const;
    EleSf getElectronSfs(const SkimTree& skimT, int index, const std::string& systStr) const;

    // Apply kinematic corrections (does NOT apply SFs—SFs are event weights)
    void applyCorrections(std::shared_ptr<SkimTree>& skimT);

    const std::unordered_map<std::string, TLorentzVector>& getP4MapElectron1() const {
        return p4MapElectron1_;
    }

private:
    void initializeP4Map();
    void print() const;

private:
    ScaleElectronFunction functions_; // composition: Function is used ONLY here

    const GlobalFlag& globalFlags_;
    const bool isDebug_;
    const bool isData_;
    const bool isMC_;

    std::unordered_map<std::string, TLorentzVector> p4MapElectron1_;
};

