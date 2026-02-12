#include "MathHdm.h"

#include "MathGuard.hpp"
#include "TMath.h"

#include <cmath>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace {
constexpr double kEps = 1e-12;
} // namespace

// Constructor implementation
MathHdm::MathHdm(const GlobalFlag& globalFlags)
    : globalFlags_(globalFlags),
      year_(globalFlags_.getYear()),
      channel_(globalFlags_.getChannel()),
      isDebug_(globalFlags_.isDebug())
{}

//---------------------------------------------------------------------
// calcResponse:
//   Computes derived 4-vectors and scalar variables based on the inputs.
//---------------------------------------------------------------------
void MathHdm::calcResponse(const TLorentzVector& p4CorrMet,
                           const TLorentzVector& p4Ref,
                           const TLorentzVector& p4Jet1,
                           const TLorentzVector& p4Jetn)
{
    MathGuard g("MathHdm", "calcResponse", isDebug_);

    // Validate inputs before caching
    g.cacheP4("p4CorrMet(in)", p4CorrMet)
     .cacheP4("p4Ref(in)",     p4Ref)
     .cacheP4("p4Jet1(in)",    p4Jet1)
     .cacheP4("p4Jetn(in)",    p4Jetn);

    g.requireFiniteP4(p4CorrMet, "p4CorrMet");
    g.requireFiniteP4(p4Ref,     "p4Ref");
    g.requireFiniteP4(p4Jet1,    "p4Jet1");
    g.requireFiniteP4(p4Jetn,    "p4Jetn");

    // Cache inputs
    p4CorrMet_ = p4CorrMet;
    p4Ref_     = p4Ref;
    p4Jet1_    = p4Jet1;
    p4Jetn_    = p4Jetn;

    // Cache post-copied state (very useful if something mutates later)
    g.cacheP4("p4CorrMet(cached)", p4CorrMet_)
     .cacheP4("p4Ref(cached)",     p4Ref_)
     .cacheP4("p4Jet1(cached)",    p4Jet1_)
     .cacheP4("p4Jetn(cached)",    p4Jetn_);

    p4Conservation_ = p4CorrMet_ + (p4Ref_ + p4Jet1_ + p4Jetn_);
    g.cacheP4("p4Conservation", p4Conservation_);

    const double ptRef = p4Ref_.Pt();
    g.cacheScalar("ptRef", ptRef);
    g.requireFiniteScalar(ptRef, "ptRef");
    if (ptRef <= kEps) {
        g.fail("invalid p4Ref.Pt() (<=0) -> cannot compute HDM responses");
    }

    const double ptRefSqr = ptRef * ptRef;
    g.cacheScalar("ptRefSqr", ptRefSqr);
    g.requireFiniteScalar(ptRefSqr, "ptRefSqr");
    if (ptRefSqr <= kEps) {
        g.fail("invalid ptRef^2 (<=0) -> cannot compute HDM responses");
    }

    // Eq.8 of hdm_2023082.pdf // Eq.11 of JME-21-001
    p4Metu_  = -(p4CorrMet_ + p4Ref_ + p4Jet1_ + p4Jetn_);
    p4Met1_  = -(p4Ref_ + p4Jet1_);
    p4Metn_  = -p4Jetn_;
    p4Metnu_ = p4Metn_ + 1.1 * p4Metu_;

    g.cacheP4("p4Metu(preT)",  p4Metu_)
     .cacheP4("p4Met1(preT)",  p4Met1_)
     .cacheP4("p4Metn(preT)",  p4Metn_)
     .cacheP4("p4Metnu(preT)", p4Metnu_);

    // Transverse versions for dot products (your existing behavior)
    makeTransverse(p4CorrMet_);
    makeTransverse(p4Met1_);
    makeTransverse(p4Metn_);
    makeTransverse(p4Metu_);
    makeTransverse(p4Metnu_);

    g.cacheP4("p4CorrMet(T)", p4CorrMet_)
     .cacheP4("p4Met1(T)",    p4Met1_)
     .cacheP4("p4Metn(T)",    p4Metn_)
     .cacheP4("p4Metu(T)",    p4Metu_)
     .cacheP4("p4Metnu(T)",   p4Metnu_);

    // Re-check vectors after modifications
    g.requireFiniteP4(p4CorrMet_, "p4CorrMet_(transverse)");
    g.requireFiniteP4(p4Met1_,    "p4Met1_(transverse)");
    g.requireFiniteP4(p4Metn_,    "p4Metn_(transverse)");
    g.requireFiniteP4(p4Metu_,    "p4Metu_(transverse)");
    g.requireFiniteP4(p4Metnu_,   "p4Metnu_(transverse)");

    // Basic variables
    bal_   = p4Jet1_.Pt() / ptRef;

    // Guard denominators explicitly right before divisions
    g.requireDenNotSmall(ptRef,    "ptRef",    kEps);
    g.requireDenNotSmall(ptRefSqr, "ptRefSqr", kEps);

    mpf_   = 1.0 + p4CorrMet_.Vect().Dot(p4Ref_.Vect()) / ptRefSqr;
    mpf1_  = 1.0 + p4Met1_.Vect().Dot(p4Ref_.Vect())    / ptRefSqr;
    mpfn_  =        p4Metn_.Vect().Dot(p4Ref_.Vect())    / ptRefSqr;
    mpfu_  =        p4Metu_.Vect().Dot(p4Ref_.Vect())    / ptRefSqr;
    mpfnu_ =        p4Metnu_.Vect().Dot(p4Ref_.Vect())   / ptRefSqr;

    g.cacheScalar("bal",   bal_)
     .cacheScalar("mpf",   mpf_)
     .cacheScalar("mpf1",  mpf1_)
     .cacheScalar("mpfn",  mpfn_)
     .cacheScalar("mpfu",  mpfu_)
     .cacheScalar("mpfnu", mpfnu_);

    g.requireFiniteScalar(bal_,   "bal");
    g.requireFiniteScalar(mpf_,   "mpf");
    g.requireFiniteScalar(mpf1_,  "mpf1");
    g.requireFiniteScalar(mpfn_,  "mpfn");
    g.requireFiniteScalar(mpfu_,  "mpfu");
    g.requireFiniteScalar(mpfnu_, "mpfnu");

    // MPFX: rotate p4Ref by +90 degrees in phi.
    p4Refx_ = p4Ref_;
    double newPhi = p4Ref_.Phi() + 0.5 * TMath::Pi();
    newPhi = TMath::ATan2(std::sin(newPhi), std::cos(newPhi));

    g.cacheScalar("p4Ref.Phi()", p4Ref_.Phi());
    g.cacheScalar("newPhi", newPhi);
    g.requireFiniteScalar(newPhi, "newPhi");

    p4Refx_.SetPhi(newPhi);
    g.cacheP4("p4Refx", p4Refx_);
    g.requireFiniteP4(p4Refx_, "p4Refx");

    mpfx_   = 1.0 + p4CorrMet_.Vect().Dot(p4Refx_.Vect()) / ptRefSqr;
    mpf1x_  = 1.0 + p4Met1_.Vect().Dot(p4Refx_.Vect())    / ptRefSqr;
    mpfnPt_ =        p4Metn_.Vect().Dot(p4Refx_.Vect())    / ptRefSqr;
    mpfux_  =        p4Metu_.Vect().Dot(p4Refx_.Vect())    / ptRefSqr;
    mpfnux_ =        p4Metnu_.Vect().Dot(p4Refx_.Vect())   / ptRefSqr;

    g.cacheScalar("mpfx",   mpfx_)
     .cacheScalar("mpf1x",  mpf1x_)
     .cacheScalar("mpfnPt", mpfnPt_)
     .cacheScalar("mpfux",  mpfux_)
     .cacheScalar("mpfnux", mpfnux_);

    g.requireFiniteScalar(mpfx_,   "mpfx");
    g.requireFiniteScalar(mpf1x_,  "mpf1x");
    g.requireFiniteScalar(mpfnPt_, "mpfnPt");
    g.requireFiniteScalar(mpfux_,  "mpfux");
    g.requireFiniteScalar(mpfnux_, "mpfnux");

    // Sanity check for HDM identities
    const double checkSum = mpf1_ + mpfn_ - mpfu_;
    g.cacheScalar("checkSum(mpf1+mpfn+mpfu)", checkSum);
    g.requireFiniteScalar(checkSum, "checkSum");

    const double diff = checkSum - mpf_;
    g.cacheScalar("diff(checkSum-mpf)", diff);
    g.requireFiniteScalar(diff, "diff");

    if (std::fabs(diff) > 1e-4) {
        std::ostringstream oss;
        oss << "HDM identity failed: mpf != mpf1+mpfn+mpfu "
            << "(diff=" << diff
            << ", mpf=" << mpf_
            << ", mpf1=" << mpf1_
            << ", mpfn=" << mpfn_
            << ", mpfu=" << mpfu_ << ")";
        g.fail(oss.str());
    }

    if (isDebug_) {
        printInputs();
    }
}

//---------------------------------------------------------------------
// makeTransverse:
//   Sets the Pz component of a TLorentzVector to zero.
//---------------------------------------------------------------------
void MathHdm::makeTransverse(TLorentzVector& vec)
{
    vec.SetPz(0.0);
    // Intentionally do NOT touch E here; downstream uses Vect().Dot(...)
}

void MathHdm::printInputs() const
{
    std::cout << "MathHdm: Input 4-Vectors:\n";
    std::cout << "p4CorrMet: pt=" << p4CorrMet_.Pt()
              << ", eta=" << p4CorrMet_.Eta()
              << ", phi=" << p4CorrMet_.Phi() << "\n";

    std::cout << "p4Ref:     pt=" << p4Ref_.Pt()
              << ", eta=" << p4Ref_.Eta()
              << ", phi=" << p4Ref_.Phi() << "\n";

    std::cout << "p4Jet1:    pt=" << p4Jet1_.Pt()
              << ", eta=" << p4Jet1_.Eta()
              << ", phi=" << p4Jet1_.Phi() << "\n";

    std::cout << "p4Jetn:    pt=" << p4Jetn_.Pt()
              << ", eta=" << p4Jetn_.Eta()
              << ", phi=" << p4Jetn_.Phi() << "\n";

    std::cout << "p4Conservation: pt=" << p4Conservation_.Pt()
              << ", eta=" << p4Conservation_.Eta()
              << ", phi=" << p4Conservation_.Phi() << "\n\n";
}

// Builds an axis that is A + B in the transverse plane, normalized to unit length.
TLorentzVector MathHdm::buildUnitAxis(const TLorentzVector& A,
                                      const TLorentzVector& B) const
{
    MathGuard g("MathHdm", "buildUnitAxis", isDebug_);

    g.cacheP4("A(in)", A).cacheP4("B(in)", B);
    g.requireFiniteP4(A, "A");
    g.requireFiniteP4(B, "B");

    TLorentzVector axis;
    axis.SetPtEtaPhiM(0, 0, 0, 0);

    axis += A;
    axis += B;

    // Force to 2D and normalize
    axis.SetPtEtaPhiM(axis.Pt(), 0.0, axis.Phi(), 0.0);
    g.cacheP4("axis(preNorm)", axis);

    // This checks both finiteness and degeneracy (via |v|^2)
    g.requireUnitAxisOK(axis, "axis", kEps);

    const double pt = axis.Pt();
    g.cacheScalar("axis.Pt()", pt);
    g.requireDenNotSmall(pt, "axis.Pt()", kEps);

    axis *= (1.0 / pt);
    g.cacheP4("axis(unit)", axis);

    g.requireFiniteP4(axis, "axis(unit)");
    return axis;
}

TLorentzVector MathHdm::buildUnitAxisForBisector(const TLorentzVector A,
                                                 const TLorentzVector B) const
{
    MathGuard g("MathHdm", "buildUnitAxisForBisector", isDebug_);

    g.cacheP4("A(in)", A).cacheP4("B(in)", B);
    g.requireFiniteP4(A, "A");
    g.requireFiniteP4(B, "B");

    TLorentzVector p4A = A;
    TLorentzVector p4B = B;

    // Proper bisector axis (equal angles): use unit vectors in phi
    p4A.SetPtEtaPhiM(1.0, 0.0, A.Phi(), 0.0);
    p4B.SetPtEtaPhiM(1.0, 0.0, B.Phi(), 0.0);

    TLorentzVector axis;
    axis.SetPtEtaPhiM(0, 0, 0, 0);
    axis += p4A;
    axis -= p4B;

    axis.SetPtEtaPhiM(axis.Pt(), 0.0, axis.Phi(), 0.0);
    g.cacheP4("axis(preNorm)", axis);

    g.requireUnitAxisOK(axis, "axis", kEps);

    const double pt = axis.Pt();
    g.cacheScalar("axis.Pt()", pt);
    g.requireDenNotSmall(pt, "axis.Pt()", kEps);

    axis *= (1.0 / pt);
    g.cacheP4("axis(unit)", axis);

    g.requireFiniteP4(axis, "axis(unit)");
    return axis;
}

double MathHdm::mpfResponse(const TLorentzVector& obj,
                            const TLorentzVector& axis,
                            double scale,
                            double offset) const
{
    MathGuard g("MathHdm", "mpfResponse", isDebug_);

    g.cacheP4("obj(in)", obj)
     .cacheP4("axis(in)", axis)
     .cacheScalar("scale", scale)
     .cacheScalar("offset", offset);

    g.requireFiniteP4(obj,  "obj");
    g.requireFiniteP4(axis, "axis");
    g.requireFiniteScalar(scale,  "scale");
    g.requireFiniteScalar(offset, "offset");

    g.requireDenNotSmall(scale, "scale", kEps);

    const double proj = obj.Vect().Dot(axis.Vect());
    g.cacheScalar("proj(obj·axis)", proj);
    g.requireFiniteScalar(proj, "proj(obj·axis)");

    const double resp = offset + proj / scale;
    g.cacheScalar("resp", resp);
    g.requireFiniteScalar(resp, "resp");

    return resp;
}

