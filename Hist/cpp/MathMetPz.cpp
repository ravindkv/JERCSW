#include "MathMetPz.h"
#include <cmath>        // for std::fabs, std::sqrt
#include <algorithm>    // for std::max
#include "TVector3.h"   // for TVector3

// ---------------------------------------------------------------------------
MathMetPz::MathMetPz()
    : isComplex_{false},
      pzBase_{0.0},
      pzOther_{0.0},
      massLep_{0.105658367}, // default: muon mass (GeV)
      massW_{80.4} { }       // default: W-boson mass (GeV)

// ---------------------------------------------------------------------------
void MathMetPz::setInput(const TLorentzVector& p4Met, const TLorentzVector& p4Lep, double massLep, double massW) {
    p4Met_  = p4Met;
    p4Lep_  = p4Lep;
    massLep_ = massLep;
    massW_   = massW;
}

// ---------------------------------------------------------------------------
void MathMetPz::calcMetPz(CalculationType type) {
    const double eLep  = p4Lep_.E();
    const double pxLep = p4Lep_.Px();
    const double pyLep = p4Lep_.Py();
    const double pzLep = p4Lep_.Pz();
    const double pxMet = p4Met_.Px();
    const double pyMet = p4Met_.Py();
    pzBase_  = 0.0;
    pzOther_ = 0.0;

    // Define the helper quantity a.
    const double a = massW_ * massW_ - massLep_ * massLep_ + 2.0 * (pxLep * pxMet + pyLep * pyMet);
    // A equals 4*(Eₗ² - pzₗ²) = 4*(pTₗ² + mₗ²) so it should be positive unless pTₗ is (nearly) 0.
    const double A = 4.0 * (eLep * eLep - pzLep * pzLep);

    // Protect against a (near) division by zero.
    const double epsilon = 1e-12;
    if (std::fabs(A) < epsilon) {
        // Fallback: use a crude approximation if A is nearly zero.
        isComplex_ = false;
        pzBase_ = -(pxMet * pxLep + pyMet * pyLep) / pzLep; 
        pzOther_ = pzBase_;
        return;
    }

    const double B = -4.0 * a * pzLep;
    const double C = 4.0 * eLep * eLep * (pxMet * pxMet + pyMet * pyMet) - a * a;
    const double discriminant = B * B - 4.0 * A * C;
    //For printing only
    A_ = A;
    B_ = B;
    C_ = C;
    D_ = discriminant;

    if (discriminant < 0) {
        isComplex_ = true;
        // Use the real part of the complex roots.
        pzBase_  = -B / (2.0 * A);
        pzOther_ = pzBase_;
    } else {
        isComplex_ = false;
        const double sqrtDisc = std::sqrt(discriminant);
        const double sol1 = (-B + sqrtDisc) / (2.0 * A);
        const double sol2 = (-B - sqrtDisc) / (2.0 * A);

        switch (type) {
            case CalculationType::kDefault:
            {
                // Choose the solution closest to the lepton pz.
                if (std::fabs(sol2 - pzLep) < std::fabs(sol1 - pzLep)) {
                    pzBase_  = sol2;
                    pzOther_ = sol1;
                } else {
                    pzBase_  = sol1;
                    pzOther_ = sol2;
                }
                // If the chosen solution has |pz| > 300 GeV, choose the most central solution.
                if (std::fabs(pzBase_) > 300.0) {
                    if (std::fabs(sol1) < std::fabs(sol2)) {
                        pzBase_  = sol1;
                        pzOther_ = sol2;
                    } else {
                        pzBase_  = sol2;
                        pzOther_ = sol1;
                    }
                }
                break;
            }
            case CalculationType::kClosestToMuon:
            {
                // Choose the solution closest to the lepton pz.
                if (std::fabs(sol2 - pzLep) < std::fabs(sol1 - pzLep)) {
                    pzBase_  = sol2;
                    pzOther_ = sol1;
                } else {
                    pzBase_  = sol1;
                    pzOther_ = sol2;
                }
                break;
            }
            case CalculationType::kCentral:
            {
                // Choose the solution with the smallest |pz|.
                if (std::fabs(sol1) < std::fabs(sol2)) {
                    pzBase_  = sol1;
                    pzOther_ = sol2;
                } else {
                    pzBase_  = sol2;
                    pzOther_ = sol1;
                }
                break;
            }
            case CalculationType::kMaxCosine:
            {
                // Choose the solution that gives the largest cosine of the decay angle.
                // Build the W boson momentum vector for each solution.
                TVector3 lep3(pxLep, pyLep, pzLep);
                TVector3 wVec1(pxLep + pxMet, pyLep + pyMet, pzLep + sol1);
                TVector3 wVec2(pxLep + pxMet, pyLep + pyMet, pzLep + sol2);

                // Compute sinθ in the W rest frame using a simplified measure.
                double sinThetaCm1 = 2.0 * lep3.Perp(wVec1) / massW_;
                double sinThetaCm2 = 2.0 * lep3.Perp(wVec2) / massW_;
                // Protect against round-off errors.
                double cosThetaCm1 = std::sqrt(std::max(0.0, 1.0 - sinThetaCm1 * sinThetaCm1));
                double cosThetaCm2 = std::sqrt(std::max(0.0, 1.0 - sinThetaCm2 * sinThetaCm2));

                if (cosThetaCm1 > cosThetaCm2) {
                    pzBase_  = sol1;
                    pzOther_ = sol2;
                } else {
                    pzBase_  = sol2;
                    pzOther_ = sol1;
                }
                break;
            }
            default:
                // Fallback: choose the first solution.
                pzBase_  = sol1;
                pzOther_ = sol2;
                break;
        }
    }
}

// ---------------------------------------------------------------------------
bool MathMetPz::isComplex() const {
    return isComplex_;
}

// ---------------------------------------------------------------------------
void MathMetPz::print() const {
    std::cout << "=== MathMetPz Debug Info ===" << std::endl;
    std::cout << "Lepton four-momentum:" << std::endl;
    std::cout << "  E   : " << p4Lep_.E() << std::endl;
    std::cout << "  Px  : " << p4Lep_.Px() << std::endl;
    std::cout << "  Py  : " << p4Lep_.Py() << std::endl;
    std::cout << "  Pz  : " << p4Lep_.Pz() << std::endl;
    std::cout << "  Eta  : " << p4Lep_.Eta() << std::endl;
    std::cout << "  Mass (used): " << massLep_ << std::endl;
    std::cout << std::endl;

    std::cout << "MET (transverse neutrino momentum):" << std::endl;
    std::cout << "  Px  : " << p4Met_.Px() << std::endl;
    std::cout << "  Py  : " << p4Met_.Py() << std::endl;
    std::cout << std::endl;

    std::cout << "  A  : " << A_ << std::endl;
    std::cout << "  B  : " << B_ << std::endl;
    std::cout << "  C  : " << C_ << std::endl;
    std::cout << "  D  : " << D_ << std::endl;
    std::cout << "Calculation details:" << std::endl;
    std::cout << "  Complex solution encountered: " << std::boolalpha << isComplex_ << std::noboolalpha << std::endl;
    std::cout << "  Selected neutrino pz (getSelectedPz()): " << pzBase_ << std::endl;
    std::cout << "  Alternate neutrino pz (getAlternatePz()): " << pzOther_ << std::endl;
    std::cout << "=============================" << std::endl;
}

