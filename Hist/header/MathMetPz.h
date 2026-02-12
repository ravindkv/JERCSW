#pragma once

#include <iostream>
#include "TLorentzVector.h"

/**
 * @brief Calculates the neutrino pz (longitudinal momentum) using
 *        the W mass constraint.
 */
class MathMetPz {
public:
    /// Options for selecting the neutrino solution.
    enum class CalculationType {
        kDefault = 0,       // If real roots, pick the one nearest to the lepton pz
                            // (unless |pz| > 300 GeV, then pick the most central)
        kClosestToMuon = 1, // If real roots, choose the one closest to the lepton pz
        kCentral = 2,       // If real roots, choose the most central solution (smallest |pz|)
        kMaxCosine = 3      // If real roots, pick the one giving the largest cosine of the decay angle
    };

    /// Constructor.
    MathMetPz();

    /// Destructor.
    virtual ~MathMetPz() = default;

    /// Set the input four-vectors and masses.
    /// @param p4Met  MET (transverse neutrino momentum) four-vector.
    /// @param p4Lep  Lepton four-vector.
    /// @param massLep Lepton mass.
    /// @param massW   W-boson mass.
    void setInput(const TLorentzVector& p4Met, const TLorentzVector& p4Lep, double massLep, double massW);

    /// Calculate the neutrino pz.
    /// The chosen solution is stored in pzBase_ (which can be accessed via getSelectedPz()).
    /// @param type Option to choose which solution to use.
    void calcMetPz(CalculationType type = CalculationType::kDefault);

    /// Check whether the solution was complex.
    bool isComplex() const;

    /// Get the selected (primary) neutrino pz solution.
    double getSelectedPz() const { return pzBase_; }

    /// Get the alternate neutrino pz solution.
    double getAlternatePz() const { return pzOther_; }

    /// Print debugging information.
    void print() const;

private:
    TLorentzVector p4Lep_;
    TLorentzVector p4Met_;
    bool isComplex_;   // True if the quadratic gave complex roots.
    double massLep_;   // Lepton mass.
    double massW_;     // W boson mass.

    double A_;
    double B_;
    double C_;
    double D_;
    double pzBase_;    // The selected neutrino pz solution.
    double pzOther_;   // The alternate solution.
};

