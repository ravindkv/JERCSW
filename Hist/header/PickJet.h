// header/PickJet.h
#pragma once

#include <string>
#include "GlobalFlag.h"
#include "SkimTree.h"   

class PickJet {
public:
    // Type-safe working point labels
    enum class WorkingPoint {
        Tight,
        TightLepVeto
    };

    PickJet(const GlobalFlag& globalFlags);
    ~PickJet() = default;

    // High-level generic API
    bool passId(const SkimTree& skimT, int jetIndex, WorkingPoint wp) const;
    bool passId(const SkimTree& skimT, int jetIndex, const std::string& label) const;

    // Explicit helpers (nice for readability in callers)
    bool passTight(const SkimTree& skimT, int jetIndex) const;
    bool passTightLepVeto(const SkimTree& skimT, int jetIndex) const;

private:
    const GlobalFlag&  globalFlags_;
    GlobalFlag::NanoVersion  nanoVersion_;
    GlobalFlag::JetAlgo  jetAlgo_;

    void printDebug(const std::string& message) const;

    // Helpers
    bool useLegacyJetId_() const;
    bool passTightLegacy_(const SkimTree& skimT, int jetIndex) const;
    bool passTightLepVetoLegacy_(const SkimTree& skimT, int jetIndex) const;

    bool passTightManual_(const SkimTree& skimT, int jetIndex) const;
    bool passTightLepVetoManual_(const SkimTree& skimT, int jetIndex) const;
};

