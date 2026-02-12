#include "MathL2Residual.h"

#include "MathGuard.hpp"

#include <cmath>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>

namespace {
constexpr double kEps = 1e-12;
} // namespace

MathL2Residual::MathL2Residual(const GlobalFlag& globalFlags)
    : globalFlags_(globalFlags),
      mathHdm_(globalFlags_)
{}

HistL2ResidualInput MathL2Residual::computeResponse(const TLorentzVector& p4TagOrig,
                                                    const TLorentzVector& p4ProbeOrig,
                                                    const TLorentzVector& p4CorrMetOrig,
                                                    const TLorentzVector& p4SumOtherOrig) const
{
    MathGuard g("MathL2Residual", "computeResponse", globalFlags_.isDebug());

    // Cache originals (these are what you want to see when something goes wrong)
    g.cacheP4("tag(orig)",   p4TagOrig)
     .cacheP4("probe(orig)", p4ProbeOrig)
     .cacheP4("met(orig)",   p4CorrMetOrig)
     .cacheP4("other(orig)", p4SumOtherOrig);

    // Validate inputs early (before copying/modifying)
    g.requireFiniteP4(p4TagOrig,      "p4TagOrig");
    g.requireFiniteP4(p4ProbeOrig,    "p4ProbeOrig");
    g.requireFiniteP4(p4CorrMetOrig,  "p4CorrMetOrig");
    g.requireFiniteP4(p4SumOtherOrig, "p4SumOtherOrig");

    // Make local copies so we can safely modify η etc.
    TLorentzVector p4Tag      = p4TagOrig;
    TLorentzVector p4Probe    = p4ProbeOrig;
    TLorentzVector p4CorrMet  = p4CorrMetOrig;
    TLorentzVector p4SumOther = p4SumOtherOrig;

    const double ptTag   = p4Tag.Pt();
    const double massTag   = p4Tag.M();
    const double ptProbe = p4Probe.Pt();

    g.cacheScalar("ptTag", ptTag).cacheScalar("ptProbe", ptProbe);
    g.cacheScalar("massTag", massTag).cacheScalar("ptProbe", ptProbe);
    g.requireFiniteScalar(ptTag,   "ptTag");
    g.requireFiniteScalar(massTag,   "massTag");
    g.requireFiniteScalar(ptProbe, "ptProbe");

    if (ptTag <= kEps) {
        g.fail("invalid ptTag (<=0) -> cannot compute responses");
    }
    //if (massTag <= kEps) { // this check fails for photon mass
    //    g.fail("invalid massTag (<=0) -> cannot compute responses");
    //}
    if (ptProbe < 0.0) {
        g.fail("invalid ptProbe (<0)");
    }

    const double ptAverage = 0.5 * (ptTag + ptProbe);
    g.cacheScalar("ptAverage", ptAverage);
    g.requireFiniteScalar(ptAverage, "ptAverage");
    if (ptAverage <= kEps) {
        g.fail("invalid ptAverage (<=0) -> cannot compute asymmetries");
    }

    // Bisector axis
    auto p4Bisector = mathHdm_.buildUnitAxisForBisector(p4Tag, p4Probe);
    g.cacheP4("bisector(axis)", p4Bisector);
    g.requireUnitAxisOK(p4Bisector, "bisector", kEps);

    const double ptAvgProj = 0.5 * (
        p4Tag.Vect().Dot(p4Bisector.Vect())
      - p4Probe.Vect().Dot(p4Bisector.Vect())
    );
    g.cacheScalar("ptAvgProj", ptAvgProj);
    g.requireFiniteScalar(ptAvgProj, "ptAvgProj");

    // Asymmetry A
    const double asymmA = (ptProbe - ptTag) / (2.0 * ptAverage);
    g.cacheScalar("asymmA", asymmA);
    g.requireFiniteScalar(asymmA, "asymmA");

    // MET & unclustered (3D vectors used here only for "basic" quantities; MPF will use flattened later)
    TLorentzVector p4SumTnP      = p4Probe + p4Tag;
    TLorentzVector p4Unclustered = -(p4CorrMet + p4SumTnP + p4SumOther);

    g.cacheP4("sumTnP(3D)",      p4SumTnP);
    g.cacheP4("unclustered(3D)", p4Unclustered);

    // Fill basic inputs
    HistL2ResidualInput in;
    in.ptAvgProj     = ptAvgProj;
    in.ptAverage     = ptAverage;
    in.ptMet         = p4CorrMet.Pt();
    in.ptOther       = p4SumOther.Pt();
    in.ptUnclustered = p4Unclustered.Pt();
    in.etaTag        = p4Tag.Eta();
    in.etaProbe      = p4Probe.Eta();
    in.phiProbe      = p4Probe.Phi();
    in.ptTag         = ptTag;
    in.massTag         = massTag;
    in.ptProbe       = ptProbe;
    in.asymmA        = asymmA;

    // (1+a)/(1-a) safety via guard
    in.relDbResp = g.safeOnePlusOverOneMinus(asymmA, "asymmA", kEps);

    // Asymmetry B
    const double dotMetTag = p4CorrMet.Vect().Dot(p4Tag.Vect());
    const double denomB    = (2.0 * ptTag * ptAverage);

    g.cacheScalar("dotMetTag", dotMetTag);
    g.cacheScalar("denomB",    denomB);

    g.requireFiniteScalar(dotMetTag, "dotMetTag");
    g.requireFiniteScalar(denomB,    "denomB");
    g.requireDenNotSmall(denomB, "denomB", kEps);

    const double asymmB = dotMetTag / denomB;
    g.cacheScalar("asymmB", asymmB);
    g.requireFiniteScalar(asymmB, "asymmB");

    in.asymmB     = asymmB;
    in.relMpfResp = g.safeOnePlusOverOneMinus(asymmB, "asymmB", kEps);

    // Flatten η for MPF axes (transverse plane only)
    p4CorrMet.SetPtEtaPhiM(p4CorrMet.Pt(), 0., p4CorrMet.Phi(), 0.);
    p4SumTnP.SetPtEtaPhiM(p4SumTnP.Pt(), 0., p4SumTnP.Phi(), 0.);
    p4Probe.SetPtEtaPhiM(p4Probe.Pt(), 0., p4Probe.Phi(), 0.);
    p4Tag.SetPtEtaPhiM(p4Tag.Pt(), 0., p4Tag.Phi(), 0.);
    p4SumOther.SetPtEtaPhiM(p4SumOther.Pt(), 0., p4SumOther.Phi(), 0.);

    g.cacheP4("met(flat)",   p4CorrMet);
    g.cacheP4("sumTnP(flat)",p4SumTnP);
    g.cacheP4("probe(flat)", p4Probe);
    g.cacheP4("tag(flat)",   p4Tag);
    g.cacheP4("other(flat)", p4SumOther);

    // IMPORTANT: recompute unclustered consistently in the flattened space
    p4Unclustered = -(p4CorrMet + p4SumTnP + p4SumOther);
    g.cacheP4("unclustered(flat)", p4Unclustered);

    const double one  = 1.0;
    const double zero = 0.0;

    // MPF on bisector
    in.respMetOnBisector =
        mathHdm_.mpfResponse(p4CorrMet, p4Bisector, ptAvgProj, one);
    in.respSumTnPonBisector =
        mathHdm_.mpfResponse(p4SumTnP, p4Bisector, ptAvgProj, one);
    in.respProbeOnBisector =
        mathHdm_.mpfResponse(-p4Probe, p4Bisector, ptAvgProj, zero);
    in.respTagOnBisector =
        mathHdm_.mpfResponse(p4Tag, p4Bisector, ptAvgProj, zero);
    in.respSumOtherOnBisector =
        mathHdm_.mpfResponse(p4SumOther, p4Bisector, ptAvgProj, zero);
    in.respUnclusteredOnBisector =
        mathHdm_.mpfResponse(p4Unclustered, p4Bisector, ptAvgProj, zero);

    // MPF on mean axis (M)
    auto p4M = mathHdm_.buildUnitAxis(-p4Probe, p4Tag);
    g.cacheP4("axis(mean)", p4M);
    g.requireUnitAxisOK(p4M, "mean", kEps);

    in.respMetOnMean =
        mathHdm_.mpfResponse(p4CorrMet, p4M, ptAverage, one);
    in.respSumTnPonMean =
        mathHdm_.mpfResponse(p4SumTnP, p4M, ptAverage, one);
    in.respProbeOnMean =
        mathHdm_.mpfResponse(-p4Probe, p4M, ptAverage, zero);
    in.respTagOnMean =
        mathHdm_.mpfResponse(p4Tag, p4M, ptAverage, zero);
    in.respSumOtherOnMean =
        mathHdm_.mpfResponse(p4SumOther, p4M, ptAverage, zero);
    in.respUnclusteredOnMean =
        mathHdm_.mpfResponse(p4Unclustered, p4M, ptAverage, zero);

    // MPF on probe axis (P)
    auto p4P = mathHdm_.buildUnitAxis(-p4Probe, TLorentzVector());
    g.cacheP4("axis(probe)", p4P);
    g.requireUnitAxisOK(p4P, "probe", kEps);

    in.respMetOnProbe =
        mathHdm_.mpfResponse(p4CorrMet, p4P, ptTag, one);
    in.respSumTnPonProbe =
        mathHdm_.mpfResponse(p4SumTnP, p4P, ptTag, one);
    in.respProbeOnProbe =
        mathHdm_.mpfResponse(-p4Probe, p4P, ptTag, zero);
    in.respTagOnProbe =
        mathHdm_.mpfResponse(p4Tag, p4P, ptTag, zero);
    in.respSumOtherOnProbe =
        mathHdm_.mpfResponse(p4SumOther, p4P, ptTag, zero);
    in.respUnclusteredOnProbe =
        mathHdm_.mpfResponse(p4Unclustered, p4P, ptTag, zero);

    // MPF on tag axis (T)
    TLorentzVector p4Tagx_ = p4Tag;
    double newPhi = p4Tag.Phi() + 0.5 * TMath::Pi();
    newPhi = TMath::ATan2(std::sin(newPhi), std::cos(newPhi));
    p4Tagx_.SetPhi(newPhi);
    in.respMetOnTagx   = 1.0 + p4CorrMet.Vect().Dot(p4Tagx_.Vect()) / (ptTag*ptTag);

    auto p4T = mathHdm_.buildUnitAxis(p4Tag, TLorentzVector());
    g.cacheP4("axis(tag)", p4T);
    g.requireUnitAxisOK(p4T, "tag", kEps);

    in.respMetOnTag =
        mathHdm_.mpfResponse(p4CorrMet, p4T, ptTag, one);
    in.respSumTnPonTag =
        mathHdm_.mpfResponse(p4SumTnP, p4T, ptTag, one);
    in.respProbeOnTag =
        mathHdm_.mpfResponse(-p4Probe, p4T, ptTag, zero);
    in.respTagOnTag =
        mathHdm_.mpfResponse(p4Tag, p4T, ptTag, zero);
    in.respSumOtherOnTag =
        mathHdm_.mpfResponse(p4SumOther, p4T, ptTag, zero);
    in.respUnclusteredOnTag =
        mathHdm_.mpfResponse(p4Unclustered, p4T, ptTag, zero);

    // Final safety: core scalars must be finite
    g.cacheScalar("in.ptAvgProj", in.ptAvgProj)
     .cacheScalar("in.ptAverage", in.ptAverage)
     .cacheScalar("in.relDbResp", in.relDbResp)
     .cacheScalar("in.asymmA",    in.asymmA)
     .cacheScalar("in.asymmB",    in.asymmB)
     .cacheScalar("in.relMpfResp",in.relMpfResp);

    g.requireFiniteScalar(in.ptAvgProj,  "in.ptAvgProj");
    g.requireFiniteScalar(in.ptAverage,  "in.ptAverage");
    g.requireFiniteScalar(in.relDbResp,  "in.relDbResp");
    g.requireFiniteScalar(in.asymmA,     "in.asymmA");
    g.requireFiniteScalar(in.asymmB,     "in.asymmB");
    g.requireFiniteScalar(in.relMpfResp, "in.relMpfResp");

    if (globalFlags_.isDebug()) {
        printInputs(in);
    }
    return in;
}

void MathL2Residual::printInputs(const HistL2ResidualInput& inputs) const {
    const auto& v = inputs;
    std::cout
      << "=== MathL2Residual ===\n"
      << "ptAvgProj:      " << v.ptAvgProj   << "\n"
      << "ptAverage:      " << v.ptAverage   << "\n"
      << "ptProbe:         " << v.ptProbe      << "\n"
      << "ptMet:          " << v.ptMet       << "\n"
      << "ptOther:        " << v.ptOther     << "\n"
      << "ptUnclustered:  " << v.ptUnclustered << "\n"
      << "etaProbe:        " << v.etaProbe     << "\n"
      << "phiProbe:        " << v.phiProbe     << "\n"
      << "ptTag:       " << v.ptTag    << "\n"
      << "asymmA:       " << v.asymmA    << "\n"
      << "asymmB:       " << v.asymmB    << "\n"
      << "phiTag:      " << v.phiTag   << "\n"
      << "relDbResp:        " << v.relDbResp     << "\n"
      << "relMpfResp:        " << v.relMpfResp     << "\n"
      << "\n"
      // “b” branch
      << "respMetOnBisector:            " << v.respMetOnBisector         << "\n"
      << "respSumTnPonBisector:           " << v.respSumTnPonBisector        << "\n"
      << "respProbeOnBisector:            " << v.respProbeOnBisector         << "\n"
      << "respTagOnBisector:            " << v.respTagOnBisector         << "\n"
      << "respSumOtherOnBisector:            " << v.respSumOtherOnBisector         << "\n"
      << "respUnclusteredOnBisector:            " << v.respUnclusteredOnBisector         << "\n"
      << "\n"
      // “m” branch
      << "respMetOnMean:            " << v.respMetOnMean         << "\n"
      << "respSumTnPonMean:           " << v.respSumTnPonMean        << "\n"
      << "respProbeOnMean:            " << v.respProbeOnMean         << "\n"
      << "respTagOnMean:            " << v.respTagOnMean         << "\n"
      << "respSumOtherOnMean:            " << v.respSumOtherOnMean         << "\n"
      << "respUnclusteredOnMean:            " << v.respUnclusteredOnMean         << "\n"
      << "\n"
      // “l (=probe)” branch
      << "respMetOnProbe:            " << v.respMetOnProbe         << "\n"
      << "respSumTnPonProbe:           " << v.respSumTnPonProbe        << "\n"
      << "respProbeOnProbe:            " << v.respProbeOnProbe         << "\n"
      << "respTagOnProbe:            " << v.respTagOnProbe         << "\n"
      << "respSumOtherOnProbe:            " << v.respSumOtherOnProbe         << "\n"
      << "respUnclusteredOnProbe:            " << v.respUnclusteredOnProbe         << "\n"
      << "\n"
      // “r” (= tag) branch
      << "respMetOnTag:            " << v.respMetOnTag         << "\n"
      << "respMetOnTagx:            " << v.respMetOnTagx         << "\n"
      << "respSumTnPonTag:           " << v.respSumTnPonTag        << "\n"
      << "respProbeOnTag:            " << v.respProbeOnTag         << "\n"
      << "respTagOnTag:            " << v.respTagOnTag         << "\n"
      << "respSumOtherOnTag:            " << v.respSumOtherOnTag         << "\n"
      << "respUnclusteredOnTag:            " << v.respUnclusteredOnTag         << "\n"
      << "\n"
      << "weight:         " << v.weight      << "\n"
      << "==========================\n";
}
