// header/SkimAdapter.h
#pragma once

#include "GlobalFlag.h"
#include "SkimBranch.hpp"

#include <TChain.h>
#include <cstddef>
#include <type_traits> // for std::is_same_v
#include <cstring>     // for std::memcpy

class SkimAdapter {
public:
    SkimAdapter(GlobalFlag& flags, SkimBranch& branches);

    // Branch setup (called from SkimReader)
    void setupCommonBranches(TChain* chain);
    void setupJetBranches(TChain* chain);
    void setupLeptonBranches(TChain* chain);
    void setupMCBranches(TChain* chain);
    void setupMetBranches(TChain* chain);
    void printDebug() const;

    // Called after each GetEntry to convert v15 → canonical
    void afterGetEntry();

private:
    // ------------------------------------------------------------------
    // Global flags (mirroring SkimReader style)
    // ------------------------------------------------------------------
    GlobalFlag&          globalFlags_;
    SkimBranch&          br_;

    const GlobalFlag::Year        year_;
    const GlobalFlag::Era         era_;
    const GlobalFlag::Channel     channel_;
    const GlobalFlag::JetAlgo     jetAlgo_;
    const GlobalFlag::NanoVersion nanoVer_;
    const bool                    debug_;
    const bool                    isData_;
    const bool                    isMC_;

    // Convenience booleans
    bool isNanoV9_{false};
    bool isNanoV15_{false};

    // ------------------------------------------------------------------
    // Feature flags (for speed / channel–dependent enabling)
    // ------------------------------------------------------------------
    bool enablePhoton_{false};
    bool enableElectron_{false};
    bool enableMuon_{false};
    bool enableBTag_{true};
    bool enableGenJet_{false};
    bool enableGenPart_{false};
    bool enablePSWeight_{false};

    // ------------------------------------------------------------------
    // v15-only buffers (internal)
    // ------------------------------------------------------------------
    UChar_t PV_npvs_v15_{0};
    UChar_t PV_npvsGood_v15_{0};

    Int_t   nJet_v15_{};
    Short_t Jet_chMultiplicity_v15_[SkimBranch::nJetMax]{};
    Short_t Jet_neMultiplicity_v15_[SkimBranch::nJetMax]{};

    Short_t Jet_muonIdx1_v15_[SkimBranch::nJetMax]{};
    Short_t Jet_muonIdx2_v15_[SkimBranch::nJetMax]{};
    Short_t Jet_electronIdx1_v15_[SkimBranch::nJetMax]{};
    Short_t Jet_electronIdx2_v15_[SkimBranch::nJetMax]{};

    Short_t Jet_genJetIdx_v15_[SkimBranch::nJetMax]{};
    Short_t Jet_partonFlavour_v15_[SkimBranch::nJetMax]{};
    UChar_t Jet_hadronFlavour_v15_[SkimBranch::nJetMax]{};

    // Electron
    Int_t   nElectron_v15_{};
    UChar_t Electron_cutBased_v15_[SkimBranch::nEleMax]{};

    // Muon
    Int_t   nMuon_v15_{};
    UChar_t Muon_nTrackerLayers_v15_[SkimBranch::nMuonMax]{};

    // GenLepton
    Int_t nGenDressedLepton_v15_{};

    // Photon / GenPhoton
    Int_t   nPhoton_v15_{};
    UChar_t Photon_cutBased_v15_[SkimBranch::nPhotonMax]{};
    Short_t Photon_jetIdx_v15_[SkimBranch::nPhotonMax]{};
    Short_t Photon_genPartIdx_v15_[SkimBranch::nPhotonMax]{};
    Int_t   nGenIsolatedPhoton_v15_{};

    // PSWeight / GenJet / GenPart
    Int_t   nPSWeight_v15_{};
    Int_t   nGenJet_v15_{};
    Short_t GenJet_partonFlavour_v15_[SkimBranch::nGenJetMax]{};
    Short_t GenJet_hadronFlavour_v15_[SkimBranch::nGenJetMax]{};

    Int_t    nGenPart_v15_{};
    Short_t  GenPart_genPartIdxMother_v15_[SkimBranch::nGenPartMax]{};
    UShort_t GenPart_statusFlags_v15_[SkimBranch::nGenPartMax]{};

    // Helpers
    template <typename Tdst, typename Tsrc>
    inline void convertArray_(Tdst* __restrict dst,
                              const Tsrc* __restrict src,
                              std::size_t n) noexcept
    {
        if constexpr (std::is_same_v<Tdst, Tsrc>) {
            // For identical types the compiler can inline this as a memcpy
            std::memcpy(dst, src, n * sizeof(Tdst));
        } else {
            for (std::size_t i = 0; i < n; ++i) {
                dst[i] = static_cast<Tdst>(src[i]);
            }
        }
    }

    void convertV15ToV9();
};

