#include "HistPhotonCategory.h"
#include "VarBin.h"
#include "HelperDir.hpp"

#include <cassert>
#include <cmath>
#include <string>
#include <vector>

#include <TDirectory.h>
#include <TH1D.h>

//-----------------------------------------------------------------------------
// Category names (for histogram names / legend labels)
const char* HistPhotonCategory::CategoryName[HistPhotonCategory::kNPhotonCats] = {
    "Photon_Genuine",
    "Photon_MisIdEle",
    "Photon_HadronicPhoton",
    "Photon_HadronicFake",
    "Photon_PuPhoton"
};

//-----------------------------------------------------------------------------
std::string HistPhotonCategory::MakeHistName(const std::string& catName)
{
    // Same style as HistFlavor: name starts with "h1Count"
    return "h1Count" + catName;
}

//-----------------------------------------------------------------------------
// Constructor: create (or cd to) origDir/<directoryName>/HistPhotonCategory_<histPrefix_>
// and book all histograms with VarBin pT binning.
//-----------------------------------------------------------------------------
HistPhotonCategory::HistPhotonCategory(
    TDirectory* origDir,
    const std::string& directoryName,
    const VarBin& varBin,
    const std::string& histPrefix
)
    : histPrefix_(histPrefix)
{
    InitializeHistograms(origDir, directoryName, varBin);
}

//-----------------------------------------------------------------------------
// Destructor: unique_ptr<TH1D> automatically deletes histograms
//-----------------------------------------------------------------------------
HistPhotonCategory::~HistPhotonCategory() = default;

//-----------------------------------------------------------------------------
// Book one TH1D per photon category + one "total" histogram, all with same binning.
//-----------------------------------------------------------------------------
void HistPhotonCategory::InitializeHistograms(
    TDirectory* origDir,
    const std::string& directoryName,
    const VarBin& varBin
) {
    std::string dirName = directoryName + "/HistPhotonCategory_" + histPrefix_;
    TDirectory* newDir = HelperDir::createTDirectory(origDir, dirName);
    newDir->cd();

    std::vector<double> edges = varBin.getBinsPt();
    int nBins = static_cast<int>(edges.size()) - 1;
    assert(nBins > 0);

    // One histogram per category
    for (unsigned i = 0; i < kNPhotonCats; ++i) {
        std::string name = MakeHistName(CategoryName[i]);
        catHist_[i].reset(
            new TH1D(name.c_str(), "", nBins, edges.data())
        );
        catHist_[i]->Sumw2();
    }

    // Total histogram
    std::string totalName = MakeHistName("Photon_All");
    totalHist_.reset(
        new TH1D(totalName.c_str(), "", nBins, edges.data())
    );
    totalHist_->Sumw2();

    origDir->cd();
}

//-----------------------------------------------------------------------------
// Fill using CategorizePhoton::Category logic.
// Assumes categories are mutually exclusive (as in your CategorizePhoton logic).
// If none of the flags is true, nothing is filled.
//-----------------------------------------------------------------------------
void HistPhotonCategory::Fill(double pt,
                              const CategorizePhoton::Category& cat,
                              double weight)
{
    if (pt < 0.) return;

    PhotonCat chosenCat;
    bool hasCategory = false;

    if (cat.isGenuine) {
        chosenCat = kGenuine;
        hasCategory = true;
    } else if (cat.isMisIdEle) {
        chosenCat = kMisIdEle;
        hasCategory = true;
    } else if (cat.isHadronicPhoton) {
        chosenCat = kHadronicPhoton;
        hasCategory = true;
    } else if (cat.isHadronicFake) {
        chosenCat = kHadronicFake;
        hasCategory = true;
    } else if (cat.isPuPhoton) {
        chosenCat = kPuPhoton;
        hasCategory = true;
    }

    if (!hasCategory) {
        // No category assigned (e.g. if all flags are false): skip fill.
        return;
    }

    catHist_[static_cast<size_t>(chosenCat)]->Fill(pt, weight);
    totalHist_->Fill(pt, weight);
}

//-----------------------------------------------------------------------------
// Optional convenience: fill a specific category explicitly
//-----------------------------------------------------------------------------
void HistPhotonCategory::Fill(double pt, PhotonCat cat, double weight) {
    if (pt < 0.) return;
    if (cat >= kNPhotonCats) return;

    catHist_[static_cast<size_t>(cat)]->Fill(pt, weight);
    totalHist_->Fill(pt, weight);
}

