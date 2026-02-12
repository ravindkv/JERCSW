#pragma once

#include <string>
#include <vector>

#include <TLorentzVector.h>

#include "GlobalFlag.h"
#include "PickJet.h"
#include "SkimTree.h"

class PickDiJet {
public:
    explicit PickDiJet(const GlobalFlag& globalFlags);
    ~PickDiJet();

    /// Run the jet-selection on this event:
    void pickJets(const SkimTree& skimT);

    /// Convenience: was a tag/probe found?
    [[nodiscard]] bool hasTagProbe() const noexcept { return indexTag_ >= 0 && indexProbe_ >= 0; }

    // getters for the results:
    const int& getIndexTag()   const { return indexTag_; }
    const int& getIndexProbe() const { return indexProbe_; }

    const TLorentzVector& getP4Tag()      const { return p4Tag_; }
    const TLorentzVector& getP4Probe()    const { return p4Probe_; }
    const TLorentzVector& getP4SumOther() const { return p4SumOther_; }

private:
    // configuration & helpers
    const GlobalFlag& globalFlags_;
    PickJet     pickJet_;
    const bool  isDebug_;

    void printDebug(const std::string& msg) const;

    // selection thresholds
    double      minPtTnP_{0.0};
    std::string jetIdLabel_;
    double      minPtOther_{0.0};
    double      maxEtaTag_{999.0};

    void loadConfig(const std::string& filename);
    void validateConfig_() const;

    void resetOutputs_();

    // Deterministic swap of the leading two jets (recommended for reproducibility)
    bool useDeterministicSwap_{true};
    void maybeSwapLeadingTwo_(std::vector<int>& good, const SkimTree& skimT) const;

    // outputs
    int            indexTag_{-1};
    int            indexProbe_{-1};
    TLorentzVector p4Tag_{0,0,0,0};
    TLorentzVector p4Probe_{0,0,0,0};
    TLorentzVector p4SumOther_{0,0,0,0};
};

