#pragma once

#include <vector>
#include "TLorentzVector.h"
#include "SkimTree.h"
#include "GlobalFlag.h"

class MathHadW {
public:
    /// Constructor that takes:
    ///  - globalFlags (for debugging)
    ///  - bTagThresh (for identifying b-jets)
    ///  - wMass (nominal W mass)
    ///  - sigmaW (width used in the chi^2 term)
    explicit MathHadW(const GlobalFlag& globalFlags,
                      double bTagThresh,
                      double wMass,
                      double sigmaW);

    ~MathHadW() = default;

    /// Compute the hadronic W from the leading 8 jets (after sorting by pT),
    /// by:
    ///   1) Selecting exactly 2 b-jets above bTagThresh. If more pass, pick the two highest bTag.
    ///   2) Among the remaining jets, try all pairs. Compute chi^2 wrt. W mass:
    ///         chi^2 = ((m_jj - wMass)/sigmaW)^2
    ///      Pick the pair with the smallest chi^2. 
    ///
    /// If not enough b-jets or not enough leftover jets, returns an empty TLorentzVector.
    const TLorentzVector getP4HadW(const SkimTree& skimT,
                                   const std::vector<int>& indexJets);

    const int getIndex1ForW() const {return index1ForW_;}
    const int getIndex2ForW() const {return index2ForW_;}
    const double getChiSqr() const {return bestChi2_;}

    /// Debug print function
    void print() const;

private:
    // Internal data
    const GlobalFlag& globalFlags_;

    // B-tagging threshold
    double bTagThreshold_;

    // Nominal W mass and resolution
    double wMass_, sigmaW_;

    // Indices that form the final W
    int index1ForW_;
    int index2ForW_;

    // The final four-vector
    TLorentzVector diJet_;

    // For debug: store the best chi^2 found
    double bestChi2_;
};

