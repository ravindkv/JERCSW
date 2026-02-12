#pragma once

#include <TLorentzVector.h>

#include "GlobalFlag.h"
#include "MathHdm.h"
#include "HistL3ResidualInput.hpp"

class MathL3Residual {
public:
    explicit MathL3Residual(const GlobalFlag& globalFlags);

    [[nodiscard]] HistL3ResidualInput computeResponse(
        const TLorentzVector& p4Tag,
        const TLorentzVector& p4Probe,
        const TLorentzVector& p4CorrMet,
        const TLorentzVector& p4SumOther
    );

    // Holds references -> don't allow copying/assignment by accident
    MathL3Residual(const MathL3Residual&) = delete;
    MathL3Residual& operator=(const MathL3Residual&) = delete;

    /// Print all of the currently stored inputs to the given stream (default = std::cout)
    void printInputs(const HistL3ResidualInput& inputs) const;
private:
    const GlobalFlag& globalFlags_;
    MathHdm     mathHdm_;
};

