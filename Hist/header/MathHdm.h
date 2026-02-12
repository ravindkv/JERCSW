#pragma once

#include "TLorentzVector.h"
#include "GlobalFlag.h"

class MathHdm {
public:
    explicit MathHdm(const GlobalFlag& globalFlags);

    /// Calculate the derived 4-vectors and variables.
    void calcResponse(const TLorentzVector& p4CorrMet,
                      const TLorentzVector& p4Ref,
                      const TLorentzVector& p4Jet1,
                      const TLorentzVector& p4Jetn);

    /// Print the pt, eta, phi of all input 4-vectors (only meaningful if isDebug_).
    void printInputs() const;

    // Getters for computed TLorentzVectors
    const TLorentzVector& getP4Metu()   const { return p4Metu_; }
    const TLorentzVector& getP4Met1()   const { return p4Met1_; }
    const TLorentzVector& getP4Metn()   const { return p4Metn_; }
    const TLorentzVector& getP4Metnu()  const { return p4Metnu_; }
    const TLorentzVector& getP4Refx()   const { return p4Refx_; }

    // Getters for computed scalar variables
    double getBal()    const { return bal_; }
    double getMpf()    const { return mpf_; }
    double getMpf1()   const { return mpf1_; }
    double getMpfn()   const { return mpfn_; }
    double getMpfu()   const { return mpfu_; }
    double getMpfnu()  const { return mpfnu_; }
    double getMpfx()   const { return mpfx_; }
    double getMpf1x()  const { return mpf1x_; }
    double getMpfnPt() const { return mpfnPt_; }
    double getMpfux()  const { return mpfux_; }
    double getMpfnux() const { return mpfnux_; }

    // For MultiJet / general axes
    TLorentzVector buildUnitAxis(const TLorentzVector& A,
                                 const TLorentzVector& B) const;

    // Proper bisector axis (equal angles to each jet)
    TLorentzVector buildUnitAxisForBisector(const TLorentzVector A,
                                            const TLorentzVector B) const;

    double mpfResponse(const TLorentzVector& obj,
                       const TLorentzVector& axis,
                       double scale,
                       double offset) const;

private:
    // Reference to GlobalFlag instance
    const GlobalFlag& globalFlags_;
    const GlobalFlag::Year year_;
    const GlobalFlag::Channel channel_;
    const bool isDebug_;

    // Input 4-vectors (cached)
    TLorentzVector p4CorrMet_;
    TLorentzVector p4Ref_;
    TLorentzVector p4Jet1_;
    TLorentzVector p4Jetn_;

    // For sanity check
    TLorentzVector p4Conservation_;

    // Computed 4-vectors
    TLorentzVector p4Metu_;
    TLorentzVector p4Met1_;
    TLorentzVector p4Metn_;
    TLorentzVector p4Metnu_;
    TLorentzVector p4Refx_;

    // Computed scalar variables
    double bal_    {0.0};
    double mpf_    {0.0};
    double mpf1_   {0.0};
    double mpfn_   {0.0};
    double mpfu_   {0.0};
    double mpfnu_  {0.0};
    double mpfx_   {0.0};
    double mpf1x_  {0.0};
    double mpfnPt_ {0.0};
    double mpfux_  {0.0};
    double mpfnux_ {0.0};

    // Helper method to make a 4-vector transverse (i.e. set Pz to zero)
    void makeTransverse(TLorentzVector& vec);
};

