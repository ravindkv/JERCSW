#pragma once

#include <cmath>

class HelperDelta {
public:
    // Keep same names for downstream compatibility.
    // Mark inline so it becomes header-only and inlinable.

    static inline double DELTAPHI(double phi1, double phi2) {
        double dphi = std::fabs(phi1 - phi2);

        // Avoid ROOT TMath dependency in hot code.
        // TwoPi constant is stable and fast.
        constexpr double kPi    = 3.14159265358979323846;
        constexpr double kTwoPi = 6.28318530717958647692;

        return (dphi <= kPi) ? dphi : (kTwoPi - dphi);
    }

    static inline double DELTAR(double phi1, double phi2, double eta1, double eta2) {
        const double dphi = DELTAPHI(phi1, phi2);
        const double deta = (eta1 - eta2);

        // Much faster than pow(x,2) + pow(y,2)
        return std::sqrt(dphi * dphi + deta * deta);
    }
};

