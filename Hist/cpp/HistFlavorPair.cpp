#include "HistFlavorPair.h"
#include "VarBin.h"
#include "HelperDir.hpp"

#include <cassert>
#include <cmath>
#include <string>
#include <vector>

#include <TDirectory.h>
#include <TH1D.h>

// -----------------------------------------------------------------------------
// Pair names (used in histogram names)
const char* HistFlavorPair::PairName[HistFlavorPair::kNPairs] = {
    "Pair_gg",
    "Pair_gq",
    "Pair_qq",
    "Pair_o"
};

// -----------------------------------------------------------------------------
std::string HistFlavorPair::MakeHistName(const std::string& pairName)
{
    // Mirrors your HistFlavor naming: h1Count<Pair>InPt<prefix>
    return "h1Count" + pairName;
}

// -----------------------------------------------------------------------------
HistFlavorPair::Kind HistFlavorPair::PartonToKind(int absPartonFlav) noexcept {
    if (absPartonFlav >= 1 && absPartonFlav <= 5) return kQ; // u,d,s,c,b
    if (absPartonFlav == 21)                      return kG; // gluon
    return kX; // anything else
}

// Order independent mapping
HistFlavorPair::Pair HistFlavorPair::KindToPair(Kind a, Kind b) noexcept {
    // If any is X → Other
    if (a == kX || b == kX) return kOther;
    // Both gluons
    if (a == kG && b == kG) return kGG;
    // Both quarks
    if (a == kQ && b == kQ) return kQQ;
    // Mixed (Q,G)
    return kGQ;
}

// -----------------------------------------------------------------------------
// ctor
HistFlavorPair::HistFlavorPair(TDirectory* origDir,
                               const std::string& directoryName,
                               const VarBin& varBin,
                               const std::string& histPrefix)
  : histPrefix_(histPrefix)
{
    InitializeHistograms(origDir, directoryName, varBin);
}

// dtor
HistFlavorPair::~HistFlavorPair() = default;

// -----------------------------------------------------------------------------
void HistFlavorPair::InitializeHistograms(TDirectory* origDir,
                                          const std::string& directoryName,
                                          const VarBin& varBin)
{
    std::string dirName = directoryName + "/HistFlavorPair_"+histPrefix_;
    TDirectory* newDir = HelperDir::createTDirectory(origDir, dirName);
    newDir->cd();

    std::vector<double> edges = varBin.getBinsPt();
    const int nBins = static_cast<int>(edges.size()) - 1;
    assert(nBins > 0);

    for (unsigned i = 0; i < kNPairs; ++i) {
        std::string name = MakeHistName(PairName[i]);
        pairHist_[i].reset(new TH1D(name.c_str(), "", nBins, edges.data()));
        pairHist_[i]->Sumw2();
    }

    std::string totalName = MakeHistName("PairAll");
    totalHist_.reset(new TH1D(totalName.c_str(), "", nBins, edges.data()));
    totalHist_->Sumw2();

    origDir->cd();
}

// -----------------------------------------------------------------------------
// Fill one entry in the pair-specific and total histograms.
// -----------------------------------------------------------------------------
void HistFlavorPair::Fill(double pt, int partonFlav1, int partonFlav2, double weight)
{
    if (pt < 0.) return;

    const Kind k1 = PartonToKind(std::abs(partonFlav1));
    const Kind k2 = PartonToKind(std::abs(partonFlav2));
    const Pair p  = KindToPair(k1, k2);

    pairHist_[static_cast<size_t>(p)]->Fill(pt, weight);
    totalHist_->Fill(pt, weight);
}

