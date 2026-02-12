#pragma once

#include <unordered_map>
#include <memory>
#include <string>
#include <vector>

#include "SkimTree.h"
#include "TLorentzVector.h"
#include "GlobalFlag.h"
#include "ScalePhotonFunction.h"

class ScalePhoton {
public:
    using SystLevel = ScalePhotonFunction::SystLevel;
    using PhotonSf  = ScalePhotonFunction::PhotonSf;

    explicit ScalePhoton(const GlobalFlag& globalFlags);

    // -------- Scale/Smear (kinematics) via correctionlib --------
    double getPhotonScaleCorrection(const SkimTree& skimT, const std::string& nomOrSyst, int index) const;
    double getPhotonSmearCorrection(const SkimTree& skimT, const std::string& nomOrSyst, int index) const;

    // -------- SFs (weights) --------
    double getPhotonIdSf(double pt, double eta, SystLevel syst = SystLevel::Nominal) const;
    double getPhotonPsSf(double pt, double eta, SystLevel syst = SystLevel::Nominal) const;
    double getPhotonCsSf(double pt, double eta, SystLevel syst = SystLevel::Nominal) const;

    PhotonSf getPhotonSfs(const SkimTree& skimT, int index, SystLevel syst = SystLevel::Nominal) const;
    PhotonSf getPhotonSfs(const SkimTree& skimT, int index, const std::string& systStr) const;

    // Apply only kinematic corrections (not SF weights)
    void applyCorrections(std::shared_ptr<SkimTree>& skimT);

    const std::unordered_map<std::string, TLorentzVector>& getP4MapPhoton1() const {
        return p4MapPhoton1_;
    }

private:
    void initializeP4Map();
    void print() const;

private:
    ScalePhotonFunction functions_; // composition: Function used ONLY here

    const GlobalFlag& globalFlags_;
    const bool isDebug_;
    const bool isData_;
    const bool isMC_;

    std::unordered_map<std::string, TLorentzVector> p4MapPhoton1_;
};

