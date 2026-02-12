#include "MathL3Residual.h"

#include "MathGuard.hpp"

#include <cmath>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>

namespace {
constexpr double kEps = 1e-12;
} // namespace

MathL3Residual::MathL3Residual(const GlobalFlag& globalFlags)
    : globalFlags_(globalFlags),
      mathHdm_(globalFlags_)
{}

HistL3ResidualInput MathL3Residual::computeResponse(const TLorentzVector& p4TagOrig,
                                                    const TLorentzVector& p4ProbeOrig,
                                                    const TLorentzVector& p4CorrMetOrig,
                                                    const TLorentzVector& p4SumOtherOrig)
{
    MathGuard g("MathL3Residual", "computeResponse", globalFlags_.isDebug());

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
    const double ptProbe = p4Probe.Pt();

    g.cacheScalar("ptTag", ptTag).cacheScalar("ptProbe", ptProbe);
    g.requireFiniteScalar(ptTag,   "ptTag");
    g.requireFiniteScalar(ptProbe, "ptProbe");

    if (ptTag <= kEps) {
        g.fail("invalid ptTag (<=0) -> cannot compute responses");
    }
    if (ptProbe < 0.0) {
        g.fail("invalid ptProbe (<0)");
    }

    TLorentzVector p4SumTnP      = p4Probe + p4Tag;
    TLorentzVector p4Unclustered = -(p4CorrMet + p4SumTnP + p4SumOther);

    g.cacheP4("sumTnP(3D)",      p4SumTnP);
    g.cacheP4("unclustered(3D)", p4Unclustered);

    // Fill basic inputs
    HistL3ResidualInput in;
    in.ptTag         = ptTag;
    in.ptProbe       = ptProbe;
    in.etaProbe      = p4Probe.Eta();
    in.ptMet         = p4CorrMet.Pt();
    in.ptOther       = p4SumOther.Pt();
    in.ptUnclustered = p4Unclustered.Pt();

    mathHdm_.calcResponse(p4CorrMet, p4Tag, p4Probe, p4SumOther);
    in.respDb    = mathHdm_.getBal();  
    in.respMpf   = mathHdm_.getMpf();  
    in.respMpf1  = mathHdm_.getMpf1(); 
    in.respMpfn  = mathHdm_.getMpfn(); 
    in.respMpfu  = mathHdm_.getMpfu(); 
    in.respMpfnu = mathHdm_.getMpfnu();

    if (globalFlags_.isDebug()) {
        printInputs(in);
    }
    return in;
}

void MathL3Residual::printInputs(const HistL3ResidualInput& inputs) const {
    const auto& v = inputs;
    std::cout
      << "=== MathL3Residual ===\n"
      << "ptTag:       " << v.ptTag    << "\n"
      << "ptProbe:         " << v.ptProbe      << "\n"
      << "ptMet:          " << v.ptMet       << "\n"
      << "ptOther:        " << v.ptOther     << "\n"
      << "ptUnclustered:  " << v.ptUnclustered << "\n"
      << "etaProbe:        " << v.etaProbe     << "\n"
      << "respDb:        " << v.respDb     << "\n"
      << "respMpf:        " << v.respMpf     << "\n"
      << "respMpfn:        " << v.respMpfn     << "\n"
      << "respMpfu:        " << v.respMpfu     << "\n"
      << "respMpfnu:        " << v.respMpfnu     << "\n"
      << "\n"
      << "weight:         " << v.weight      << "\n"
      << "==========================\n";
}
