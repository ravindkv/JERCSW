#pragma once

#include <vector>
#include <algorithm>
#include <cmath>
#include <limits>
#include <iostream>

#include "GlobalFlag.h"
#include "SkimTree.h"

/// \brief Computes a single selected systematic weight for MC events
class Systematics {
public:
    /// \param globalFlags Debug/logging settings, also holds the selected systematic
    Systematics(const GlobalFlag& globalFlags);

    /// \brief Compute only the selected systematic for this event
    /// \param skimT The event skim tree with weight arrays
    void compute(const SkimTree& skimT);

    // Getters for individual systematics
    double getQsqrUp()    const { return qSqrUp_; }
    double getQsqrDown()  const { return qSqrDown_; }
    double getPdfUp()   const { return pdfUp_; }
    double getPdfDown() const { return pdfDown_; }
    double getIsrUp()   const { return isrUp_; }
    double getIsrDown() const { return isrDown_; }
    double getFsrUp()   const { return fsrUp_; }
    double getFsrDown() const { return fsrDown_; }

    /// \brief Access the computed systematic value corresponding to the selected flag
    double getSystValue() const;

    /// \brief Print all computed values (for debugging)
    void print() const;

private:
    const GlobalFlag& globalFlags_;
    const GlobalFlag::Systematic systFlag_;

    // Scale variation
    double qSqrUp_      = 1.0;
    double qSqrDown_    = 1.0;

    // Pdf variation
    double pdfUp_     = 1.0;
    double pdfDown_   = 1.0;

    // Parton shower variations
    double isrUp_     = 1.0;
    double isrDown_   = 1.0;
    double fsrUp_     = 1.0;
    double fsrDown_   = 1.0;

    // Temporary storage
    std::vector<double> genScaleWeights_;
    std::vector<double> pdfWeights_;

    // Category-specific computations
    void computeQsqr(const SkimTree& skimT);
    void computePdf(const SkimTree& skimT);
    void computeIsrFsr(const SkimTree& skimT);
};

