#pragma once

#include <string>
#include <iostream>

#include "SkimTree.h"
#include "GlobalFlag.h"

/**
 * @brief Classify reconstructed photons into generator-level categories.
 *
 * This helper implements a CMS-style photon categorisation similar to what is
 * used in tt̄+γ analyses(TOP-18-010). For each reconstructed photon, it assigns exactly
 * one of the following mutually exclusive categories on Monte Carlo:
 *
 *   - Genuine photon:
 *       Reco photon matched to a generator-level photon (pdgId = 22) whose
 *       ancestry contains only quarks, leptons, or SM bosons (no hadrons).
 *
 *   - Misidentified electron:
 *       Reco photon matched to a generator-level electron (|pdgId| = 11).
 *
 *   - Photon with hadronic origin:
 *       Reco photon matched to a generator-level photon whose ancestry
 *       contains at least one hadron (e.g. from π0 → γγ or other hadron
 *       decays), or (in the “no direct match” case) if a π0 and at least one
 *       photon are found within a small cone around the reco photon.
 *
 *   - Hadronic fake:
 *       Reco photon matched to some other generator-level particle (neither
 *       e nor γ), or classified as hadronic in the cone-based fallback but
 *       not satisfying the π0+γ condition.
 *
 *   - PU photon:
 *       Reco photon without any suitable generator-level particles in a cone
 *       around it (typically originating from pileup, which is not stored
 *       in the generator record).
 *
 *
 * On data, the categorise() function returns a Category with all flags false.
 */
class CategorizePhoton {
public:
    /// Container for mutually exclusive photon categories.
    struct Category {
        /// True if photon is classified as a genuine prompt photon.
        bool isGenuine        = false;
        /// True if photon is a misidentified electron (e → γ fake).
        bool isMisIdEle       = false;
        /// True if photon originates from hadronic activity (π0, hadron decays).
        bool isHadronicPhoton = false;
        /// True if photon is a jet / hadron misidentified as a photon (fake).
        bool isHadronicFake   = false;
        /// True if photon is associated with pileup (no suitable gen match).
        bool isPuPhoton       = false;

        /// Convenience helper: did we assign any category at all?
        bool hasAnyCategory() const {
            return isGenuine || isMisIdEle || isHadronicPhoton ||
                   isHadronicFake || isPuPhoton;
        }
    };

    explicit CategorizePhoton(const GlobalFlag& globalFlags);
    ~CategorizePhoton() = default;

    Category categorize(const SkimTree& skimT, int phoIdx) const;

private:
    const GlobalFlag& globalFlags_;
    const GlobalFlag::Year     year_;
    const GlobalFlag::Channel  channel_;
    const bool                 isDebug_;

    // Matching / categorization parameters (configurable via JSON).
    double maxDeltaRGenMatch_;   ///< Maximum ΔR for gen–reco matching.
    double minGenPtOverRecoPt_;  ///< Minimum pT(gen)/pT(reco) to accept match.
    double minGenPartPt_;  

    /// Load matching parameters from a JSON configuration file.
    void loadConfig(const std::string& filename);

    /// Print a debug message if isDebug_ is true.
    void printDebug(const std::string& msg) const;

    /**
     * @brief Core categorisation logic using direct Photon_genPartIdx match.
     *
     * This function:
     *   - Starts from the NanoAOD Photon_genPartIdx match if available.
     *   - Re-validates the match using the pT ratio requirement.
     *   - If the direct match fails or is absent, falls back to a cone-based
     *     categorisation (categorizeFromCone).
     *   - If the direct match passes, uses the PDG ID and ancestry to classify
     *     the photon as genuine / misID electron / hadronic.
     */
    Category categorizeFromGenMatch(const SkimTree& skimT, int phoIdx) const;

    /**
     * @brief Cone-based fallback categorisation when no valid direct match exists.
     *
     * Searches all gen particles within ΔR < maxDeltaRGenMatch_ around the
     * reco photon, requiring pT(gen)/pT(reco) >= minGenPtOverRecoPt_ and
     * excluding neutrinos. If no such gen particle is found, the photon is
     * classified as a PU photon. If both a π0 (pdgId = 111) and a photon
     * (pdgId = 22) are present in the cone, it is classified as a photon with
     * hadronic origin; otherwise it is classified as a hadronic fake.
     */
    Category categorizeFromCone(const SkimTree& skimT, int phoIdx) const;
};


