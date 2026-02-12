#include "HistFlavor.h"
#include "VarBin.h"
#include "HelperDir.hpp"

#include <cassert>
#include <cmath>
#include <string>
#include <vector>

#include <TDirectory.h>
#include <TH1D.h>

//-----------------------------------------------------------------------------
// Flavor names (for histogram titles / legend labeling if desired)
const char* HistFlavor::FlavorName[HistFlavor::kNFlavors] = {
    "Flavor_ud",
    "Flavor_s",
    "Flavor_c",
    "Flavor_b",
    "Flavor_g",
    "Flavor_o"
};

//-----------------------------------------------------------------------------
std::string HistFlavor::MakeHistName(const std::string& flavName)
{
    return "h1Count" + flavName;
}

//-----------------------------------------------------------------------------
// Helper: map |pdgFlav| → Flavor enum
HistFlavor::Flavor HistFlavor::PartonFlavToEnum(int absPartonFlav) noexcept {
    if      (absPartonFlav == 1 || absPartonFlav == 2)   return kUD;
    else if (absPartonFlav == 3)   return kS;
    else if (absPartonFlav == 4)   return kC;
    else if (absPartonFlav == 5)   return kB;
    else if (absPartonFlav == 21)  return kG;
    else                           return kOther; 
}

//-----------------------------------------------------------------------------
// Constructor: create (or cd to) origDir/<directoryName>/HistFlavor,
// then call BookAllHistograms inside that directory.
//----------------------------------------------------------------------------- 
HistFlavor::HistFlavor(TDirectory* origDir,
                         const std::string& directoryName,
                         const VarBin& varBin,
                         const std::string& histPrefix)
  : histPrefix_(histPrefix)
{
    InitializeHistograms(origDir, directoryName, varBin);
}

//-----------------------------------------------------------------------------
// Destructor: unique_ptr<TH1D> automatically deletes histograms
HistFlavor::~HistFlavor() = default;

//-----------------------------------------------------------------------------
// Book one TH1D per flavor + one "total" histogram, all with same binning.
//----------------------------------------------------------------------------- 
void HistFlavor::InitializeHistograms(TDirectory* origDir, const std::string& directoryName, const VarBin& varBin) {
    std::string dirName = directoryName + "/HistFlavor_"+histPrefix_;
    TDirectory* newDir = HelperDir::createTDirectory(origDir, dirName);
    newDir->cd();

    std::vector<double> edges = varBin.getBinsPt();
    int nBins = static_cast<int>(edges.size()) - 1;
    assert(nBins > 0);

    for (unsigned i = 0; i < kNFlavors; ++i) {
        std::string name  = MakeHistName(FlavorName[i]);
        flavorHist_[i].reset(
            new TH1D(name.c_str(), "", nBins, edges.data())
        );
        flavorHist_[i]->Sumw2();
    }

    std::string totalName  = MakeHistName("FlavorAll");
    totalHist_.reset(
        new TH1D(totalName.c_str(), "", nBins, edges.data())
    );
    totalHist_->Sumw2();
    origDir->cd();
}

//-----------------------------------------------------------------------------
// Fill: determine flavor category, fill both the flavor‐specific and total histograms.
// Everything else goes into kOther category.
//----------------------------------------------------------------------------- 
void HistFlavor::Fill(double pt, int partonFlav, double weight) {
  if (pt < 0.) return;

  Flavor f = PartonFlavToEnum(std::abs(partonFlav));

  flavorHist_[size_t(f)]->Fill(pt, weight);
  totalHist_->Fill(pt, weight);
}

