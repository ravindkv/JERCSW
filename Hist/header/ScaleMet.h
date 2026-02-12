#pragma once

#include <unordered_map>
#include <vector>
#include <memory>

#include "TLorentzVector.h"
#include "SkimTree.h"
#include "ScaleJetFunction.h"
#include "GlobalFlag.h"

class ScaleMet {
public:
    explicit ScaleMet(const GlobalFlag& globalFlags);

    void applyCorrection(const std::shared_ptr<SkimTree>& skimT,
                         const std::vector<double>& jetPtRaw);

    const std::unordered_map<std::string, TLorentzVector>& getP4MapMet() const { return p4MapMet_; }
    TLorentzVector getP4CorrectedMet() const { return p4CorrectedMet_; }

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

    std::unordered_map<std::string, TLorentzVector> p4MapMet_;
    TLorentzVector p4CorrectedMet_;

    void printDebug(const std::unordered_map<std::string, TLorentzVector>& p4MapJetSum) const;
};

