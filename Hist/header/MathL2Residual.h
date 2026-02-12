#pragma once

#include <TLorentzVector.h>

#include "GlobalFlag.h"
#include "MathHdm.h"
#include "HistL2ResidualInput.hpp"

class MathL2Residual {
public:
    explicit MathL2Residual(const GlobalFlag& globalFlags);

    [[nodiscard]] HistL2ResidualInput computeResponse(
        const TLorentzVector& p4Tag,
        const TLorentzVector& p4Probe,
        const TLorentzVector& p4CorrMet,
        const TLorentzVector& p4SumOther
    ) const;

    // Holds references -> don't allow copying/assignment by accident
    MathL2Residual(const MathL2Residual&) = delete;
    MathL2Residual& operator=(const MathL2Residual&) = delete;

    /// Print all of the currently stored inputs to the given stream (default = std::cout)
    void printInputs(const HistL2ResidualInput& inputs) const;
private:
    const GlobalFlag& globalFlags_;
    MathHdm     mathHdm_;
};

