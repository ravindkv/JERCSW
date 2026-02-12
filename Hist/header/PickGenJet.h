#pragma once

#include <string>
#include <vector>

#include <TLorentzVector.h>

#include "SkimTree.h"
#include "GlobalFlag.h"

// Standalone gen-jet matcher (call from Run* call-site).
class PickGenJet {
public:
    struct Result {
        int iGenJet1 = -1;
        int iGenJet2 = -1;
        TLorentzVector p4GenJet1{0,0,0,0};
        TLorentzVector p4GenJet2{0,0,0,0};

        std::vector<int> indicesAlwaysTwo() const { return {iGenJet1, iGenJet2}; }
        std::vector<TLorentzVector> p4sAlwaysTwo() const { return {p4GenJet1, p4GenJet2}; }

        std::vector<int> indicesValidOnly() const {
            std::vector<int> out;
            if (iGenJet1 >= 0) out.push_back(iGenJet1);
            if (iGenJet2 >= 0) out.push_back(iGenJet2);
            return out;
        }
    };

    explicit PickGenJet(const GlobalFlag& globalFlags);

    // Match based on reco-jet presence by indices (iJet != -1)
    Result matchByRecoIndices(const SkimTree& skimT,
                              int iJet1, int iJet2,
                              const TLorentzVector& p4Jet1,
                              const TLorentzVector& p4Jet2) const;

    // Match based on reco-jet presence by p4 (Pt > 0)
    Result matchByRecoP4(const SkimTree& skimT,
                         const TLorentzVector& p4Jet1,
                         const TLorentzVector& p4Jet2) const;

    [[nodiscard]] double maxDeltaR() const noexcept { return maxDeltaRgenJet_; }

    // General match for single jet 
    TLorentzVector matchedP4GenJet(const SkimTree& skimT,
                              const TLorentzVector& p4Jet) const;

private:
    const GlobalFlag& globalFlags_;
    bool isDebug_ = false;

    // config
    double maxDeltaRgenJet_ = 0.2;
    double minPtGenJet_;

    void loadConfig_(const std::string& cfgFile);
    void validateConfig_() const;

    void printDebug_(const std::string& msg) const;

    static void requireFinite_(double x, const std::string& what);
    static TLorentzVector makeP4_(double pt, double eta, double phi, double mass);

    Result matchImpl_(const SkimTree& skimT,
                      bool want1, bool want2,
                      const TLorentzVector& p4Jet1,
                      const TLorentzVector& p4Jet2) const;
};

