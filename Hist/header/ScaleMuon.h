#pragma once

#include <unordered_map>
#include <memory>
#include <string>
#include <vector>

#include "SkimTree.h"
#include "TLorentzVector.h"
#include "GlobalFlag.h"
#include "ScaleMuonFunction.h"

class ScaleMuon {
public:
    using SystLevel = ScaleMuonFunction::SystLevel;
    using MuSf      = ScaleMuonFunction::MuSf;

    explicit ScaleMuon(const GlobalFlag& globalFlags);

    // -------- Rochester (kinematics) --------
    double getMuonRochCorrection(const SkimTree& skimT, int index, const std::string& syst) const;
    void applyCorrections(std::shared_ptr<SkimTree>& skimT); // applies p4 corrections only

    // -------- SFs (weights) --------
    double getMuonIdSf(double pt, double eta, SystLevel syst = SystLevel::Nominal)  const;
    double getMuonIsoSf(double pt, double eta, SystLevel syst = SystLevel::Nominal) const;
    double getMuonTrigSf(double pt, double eta, SystLevel syst = SystLevel::Nominal) const;

    MuSf getMuonSfs(const SkimTree& skimT, int index, SystLevel syst = SystLevel::Nominal) const;
    MuSf getMuonSfs(const SkimTree& skimT, int index, const std::string& systStr) const;

    const std::unordered_map<std::string, TLorentzVector>& getP4MapMuon1() const {
        return p4MapMuon1_;
    }

private:
    void initializeP4Map();
    void print() const;

private:
    ScaleMuonFunction functions_; // composition: Function used ONLY here

    const GlobalFlag& globalFlags_;
    const bool isDebug_;
    const bool isData_;
    const bool isMC_;

    std::unordered_map<std::string, TLorentzVector> p4MapMuon1_;
};

