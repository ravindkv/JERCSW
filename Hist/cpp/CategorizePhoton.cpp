#include "CategorizePhoton.h"
#include "ReadConfig.h"
#include "HelperDelta.hpp"

#include <cmath>
#include <stdexcept>
#include <vector>
#include <algorithm>

/**
 * @file CategorizePhoton.cpp
 *
 * @brief Implementation of reconstructed photon categorisation into generator-
 * level categories (genuine, misID electron, hadronic photon, hadronic fake,
 * PU photon).
 *
 * The algorithm combines:
 *   - A direct match via Photon_genPartIdx (when available in NanoAOD),
 *   - A re-validation of the match using a pT ratio requirement
 *       pT(gen) / pT(reco) >= minGenPtOverRecoPt_,
 *   - A cone-based fallback search in ΔR < maxDeltaRGenMatch_ when no valid
 *     direct match exists,
 *   - A parentage-based prompt vs hadronic classification for matched photons
 *     based on the maximum PDG ID seen in the ancestry tree.
 *
 * Using maxPDGID < 37 is effectively a shorthand for "no hadrons in the
 * chain", because all hadrons have PDG IDs ≥ 100. This allows us to label
 * photons as "genuine" if their ancestry contains only quarks, leptons,
 * and SM bosons, and as "hadronic photon" otherwise.
 */

// ----------------------------------------------------------------------
// Constructor
// ----------------------------------------------------------------------
CategorizePhoton::CategorizePhoton(const GlobalFlag& globalFlags)
    : globalFlags_(globalFlags)
    , year_(globalFlags_.getYear())
    , channel_(globalFlags_.getChannel())
    , isDebug_(globalFlags_.isDebug())
    , maxDeltaRGenMatch_(0.3)
    , minGenPtOverRecoPt_(0.5)
{
    // Load matching parameters from JSON so that analyses can tune the
    // ΔR and pT-ratio requirements without recompiling.
    loadConfig("config/CategorizePhoton.json");
}

// ----------------------------------------------------------------------
// Load configuration (currently just a hook – easy to extend)
// ----------------------------------------------------------------------
void CategorizePhoton::loadConfig(const std::string& filename) {
    ReadConfig cfg(filename);

    maxDeltaRGenMatch_  = cfg.getValue<double>({"match", "maxDeltaRGenMatch"});
    minGenPtOverRecoPt_ = cfg.getValue<double>({"match", "minGenPtOverRecoPt"});
    minGenPartPt_ = cfg.getValue<double>({"match", "minGenPartPt"});
}

// ----------------------------------------------------------------------
void CategorizePhoton::printDebug(const std::string& msg) const {
    if (isDebug_) {
        std::cout << "[CategorizePhoton] " << msg << '\n';
    }
}

// ----------------------------------------------------------------------
// Public interface
// ----------------------------------------------------------------------
CategorizePhoton::Category
CategorizePhoton::categorize(const SkimTree& skimT, int phoIdx) const {
    Category cat;  // all flags false by default

    // On data we do not have a meaningful generator-level categorisation.
    if (globalFlags_.isData()) {
        return cat;
    }

    if (phoIdx < 0 || phoIdx >= skimT.nPhoton) {
        printDebug("WARNING: phoIdx out of range: " + std::to_string(phoIdx) +
                   " (nPhoton = " + std::to_string(skimT.nPhoton) + ")");
        return cat;
    }

    cat = categorizeFromGenMatch(skimT, phoIdx);

    if (isDebug_) {
        printDebug("phoIdx=" + std::to_string(phoIdx) +
                   " -> genuine="        + std::to_string(cat.isGenuine) +
                   ", misIDEle="         + std::to_string(cat.isMisIdEle) +
                   ", hadronicPhoton="   + std::to_string(cat.isHadronicPhoton) +
                   ", hadronicFake="     + std::to_string(cat.isHadronicFake) +
                   ", puPhoton="         + std::to_string(cat.isPuPhoton));
    }

    return cat;
}

// ----------------------------------------------------------------------
// Cone-based fallback categorisation when no valid direct gen match exists
// ----------------------------------------------------------------------
CategorizePhoton::Category
CategorizePhoton::categorizeFromCone(const SkimTree& skimT, int phoIdx) const {
    Category cat;  // all flags false by default

    std::vector<int> genParticleCone_pid;
    std::vector<int> genParticleCone_idx;

    const float recoPt  = skimT.Photon_pt[phoIdx];
    const float recoEta = skimT.Photon_eta[phoIdx];
    const float recoPhi = skimT.Photon_phi[phoIdx];

    if (isDebug_) {
        printDebug("No valid direct gen match. Searching cone around reco photon. nGenPart=" +
                   std::to_string(skimT.nGenPart) +
                   " recoPt=" + std::to_string(recoPt));
    }

    for (int genIdx = 0; genIdx < skimT.nGenPart; ++genIdx) {
        const float genPt  = skimT.GenPart_pt[genIdx];
        const int   genPID = skimT.GenPart_pdgId[genIdx];
        const float genEta = skimT.GenPart_eta[genIdx];
        const float genPhi = skimT.GenPart_phi[genIdx];

        // Skip very soft gen particles
        if (genPt < minGenPartPt_) continue;

        // Enforce pT(gen)/pT(reco) >= minGenPtOverRecoPt_ if recoPt > 0
        if (recoPt > 0.f) {
            const double ptRatio =
                static_cast<double>(genPt) / static_cast<double>(recoPt);
            if (ptRatio < minGenPtOverRecoPt_) continue;
        }

        // Exclude neutrinos
        const int absId = std::abs(genPID);
        if (absId == 12 || absId == 14 || absId == 16) continue;

        // ΔR(gen, recoPhoton)
        const double dRval = HelperDelta::DELTAR(genPhi, recoPhi, genEta, recoEta);

        if (isDebug_) {
            printDebug("  genIdx=" + std::to_string(genIdx) +
                       " pdgId=" + std::to_string(genPID) +
                       " pt=" + std::to_string(genPt) +
                       " eta=" + std::to_string(genEta) +
                       " phi=" + std::to_string(genPhi) +
                       " dR=" + std::to_string(dRval));
        }

        if (dRval < maxDeltaRGenMatch_) {
            genParticleCone_idx.push_back(genIdx);
            genParticleCone_pid.push_back(genPID);
        }
    }

    // No gen particles in cone -> PU photon
    if (genParticleCone_pid.empty()) {
        cat.isPuPhoton = true;
        return cat;
    }

    // If a photon (22) AND a pi0 (111) are in the cone -> hadronic photon
    const bool hasPi0 = (std::find(genParticleCone_pid.begin(),
                                   genParticleCone_pid.end(), 111)
                         != genParticleCone_pid.end());
    const bool hasPho = (std::find(genParticleCone_pid.begin(),
                                   genParticleCone_pid.end(), 22)
                         != genParticleCone_pid.end());

    if (hasPi0 && hasPho) {
        cat.isHadronicPhoton = true;
        return cat;
    }

    // Otherwise -> hadronic fake
    cat.isHadronicFake = true;
    return cat;
}

// ----------------------------------------------------------------------
// Core categorization logic: direct match + parentage + cone fallback
// ----------------------------------------------------------------------
CategorizePhoton::Category
CategorizePhoton::categorizeFromGenMatch(const SkimTree& skimT, int phoIdx) const {
    Category cat;  // all flags false

    // --- 1) Get direct gen match index (Photon_genPartIdx) ---
    int mcMatchInd = -1;
    if (skimT.Photon_genPartIdx) {
        mcMatchInd = skimT.Photon_genPartIdx[phoIdx];
    } else {
        // If branch does not exist / is not filled, treat as "no match"
        printDebug("Photon_genPartIdx not available; treating as no direct match.");
        mcMatchInd = -1;
    }

    // Guard against out-of-range indices
    if (mcMatchInd >= skimT.nGenPart || mcMatchInd < -1) {
        printDebug("mcMatchInd out of range -> reset to -1");
        mcMatchInd = -1;
    }

    const float recoPt  = skimT.Photon_pt[phoIdx];
    const float recoEta = skimT.Photon_eta[phoIdx];
    const float recoPhi = skimT.Photon_phi[phoIdx];

    if (isDebug_) {
        printDebug("phoIdx=" + std::to_string(phoIdx) +
                   " mcMatchInd=" + std::to_string(mcMatchInd) +
                   " pt="  + std::to_string(recoPt) +
                   " eta=" + std::to_string(recoEta) +
                   " phi=" + std::to_string(recoPhi));
    }

    // ------------------------------------------------------------------
    // 1a) Re-validate direct match using pT ratio.
    //     If the gen match is too soft compared to the reco photon,
    //     treat it as "no valid direct match" and fall back to the cone.
    // ------------------------------------------------------------------
    if (mcMatchInd >= 0) {
        const float genPtMatch = skimT.GenPart_pt[mcMatchInd];
        double ptRatio = 0.0;
        if (recoPt > 0.f) {
            ptRatio = static_cast<double>(genPtMatch) / static_cast<double>(recoPt);
        }

        if (isDebug_) {
            printDebug("Direct gen match: genPt=" + std::to_string(genPtMatch) +
                       " recoPt=" + std::to_string(recoPt) +
                       " ptRatio=" + std::to_string(ptRatio) +
                       " minGenPtOverRecoPt_=" + std::to_string(minGenPtOverRecoPt_));
        }

        if (ptRatio < minGenPtOverRecoPt_) {
            if (isDebug_) {
                printDebug("Direct gen match fails pT-ratio requirement; "
                           "falling back to cone-based categorisation.");
            }
            mcMatchInd = -1;
        }
    }

    // ------------------------------------------------------------------
    // Case A: no *valid* direct gen match -> cone-based categorisation
    // ------------------------------------------------------------------
    if (mcMatchInd == -1) {
        return categorizeFromCone(skimT, phoIdx);
    }

    // ------------------------------------------------------------------
    // Case B: direct gen match exists -> inspect PDG ID and parentage
    //
    // “Genuine photon”: reco photon matched to gen photon whose ancestors are
    // only quarks, leptons, or SM bosons (no hadrons in the chain).
    // This is implemented via maxPDGID < 37 over the ancestry.
    // ------------------------------------------------------------------
    const int mcMatchPDGID = skimT.GenPart_pdgId[mcMatchInd];

    int parentIdx   = mcMatchInd;
    int maxPDGID    = 0;
    int motherPDGID = 0;

    while (parentIdx != -1) {
        motherPDGID = std::abs(skimT.GenPart_pdgId[parentIdx]);
        maxPDGID    = std::max(maxPDGID, motherPDGID);

        const int motherIdx = skimT.GenPart_genPartIdxMother[parentIdx];
        if (motherIdx == parentIdx) {
            // Safety against pathological self-loop
            break;
        }
        parentIdx = motherIdx;
    }

    // maxPDGID < 37 effectively encodes "no hadrons in the chain"
    // (all hadrons have PDG IDs >= 100).
    const bool parentagePass = (maxPDGID < 37);

    if (isDebug_) {
        printDebug("mcMatchPDGID=" + std::to_string(mcMatchPDGID) +
                   " maxMotherPDGID=" + std::to_string(maxPDGID) +
                   " parentagePass=" + std::to_string(parentagePass));
    }

    // Photon match
    if (mcMatchPDGID == 22) {
        if (parentagePass) {
            cat.isGenuine = true;
        } else {
            cat.isHadronicPhoton = true;
        }
    }
    // Electron match (mis-ID)
    else if (std::abs(mcMatchPDGID) == 11) {
        cat.isMisIdEle = true;
    }
    // Everything else -> hadronic fake
    else {
        cat.isHadronicFake = true;
    }

    return cat;
}

