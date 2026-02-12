#pragma once

#include <memory>
#include <string>

#include "GlobalFlag.h"
#include "correction.h"

// ROOT fwd decls
class TH2;
class TFile;

class ScaleBtagLoader {
public:
    explicit ScaleBtagLoader(const GlobalFlag& globalFlags);

    // --- accessors (read-only) ---
    const std::string& btagEffPath() const { return btagEffPath_; }
    const std::string& algo()        const { return algo_; }
    const std::string& wp()          const { return wp_; }
    const std::string& effType()     const { return effType_; }

    double sfPtMin()     const { return sfPtMin_; }
    double sfPtMax()     const { return sfPtMax_; }
    double sfAbsEtaMax() const { return sfAbsEtaMax_; }
    double wpCut()       const { return wpCut_; }

    const correction::Correction::Ref& corrMujets() const { return corr_mujets_; } // for b,c
    const correction::Correction::Ref& corrIncl()   const { return corr_incl_; }   // for light

    // Efficiency hist access (ensures lazy-load)
    void ensureEffHistsLoaded() const;
    const TH2* lEff() const { ensureEffHistsLoaded(); return lEff_; }
    const TH2* cEff() const { ensureEffHistsLoaded(); return cEff_; }
    const TH2* bEff() const { ensureEffHistsLoaded(); return bEff_; }

    // Runtime switch
    void setEffType(const std::string& effType);

    // Optional debug
    void printConfig() const;

private:
    // lifecycle
    void loadConfig(const std::string& filename);
    void loadBtvRefs();
    void loadEffHists_() const;    // internal lazy-load
    void updateEffHistNames_();

private:
    // --- config: paths & settings (per year) ---
    std::string btvJsonPath_;
    std::string btagEffPath_;
    std::string algo_;
    std::string wp_;
    std::string effType_;

    double sfPtMin_     {20.0};
    double sfPtMax_     {1000.0};
    double sfAbsEtaMax_ {2.5};
    double wpCut_       {-1.0};

    // eff hist names derived from effType_
    std::string lEffHistName_;
    std::string cEffHistName_;
    std::string bEffHistName_;

    // --- correctionlib refs ---
    mutable std::shared_ptr<const correction::CorrectionSet> btvSet_;
    mutable correction::Correction::Ref corr_mujets_;
    mutable correction::Correction::Ref corr_incl_;

    // --- ROOT ownership for eff hists ---
    mutable std::unique_ptr<TFile> effFile_;
    mutable TH2* lEff_{nullptr};
    mutable TH2* cEff_{nullptr};
    mutable TH2* bEff_{nullptr};
    mutable bool  reloadEffHists_{false};

    // --- globals ---
    const GlobalFlag& globalFlags_;
    const bool isDebug_;
    const bool isData_;
    const bool isMC_;
};

