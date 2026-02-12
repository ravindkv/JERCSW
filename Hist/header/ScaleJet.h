#pragma once

#include <unordered_map>
#include <vector>
#include <memory>

#include "TLorentzVector.h"
#include "SkimTree.h"
#include "ScaleJetFunction.h"
#include "GlobalFlag.h"

class ScaleJet {
public:
    explicit ScaleJet(const GlobalFlag& globalFlags);

    void applyCorrection(std::shared_ptr<SkimTree>& skimT);

    const std::unordered_map<std::string, TLorentzVector>& getP4MapJet1() const { return p4MapJet1_; }
    const std::unordered_map<std::string, TLorentzVector>& getP4MapJetSum() const { return p4MapJetSum_; }
    const std::vector<double>& getJetPtRaw() const { return jet_pt_raw_; }

private:
    ScaleJetFunction functions_;
    const GlobalFlag& globalFlags_;
    const GlobalFlag::JecApplicationLevel level_;

    // cached flags (initialized once)
    const bool isDebug_;
    const bool isData_;
    const bool isBookKeep_;
    const GlobalFlag::JetAlgo jetAlgo_;
    const bool applyJer_; // MC-only (applyJer && !isData)

    std::unordered_map<std::string, TLorentzVector> p4MapJet1_;
    std::unordered_map<std::string, TLorentzVector> p4MapJetSum_;
    std::vector<double> jet_pt_raw_;
};

