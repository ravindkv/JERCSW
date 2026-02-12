#pragma once

#include <array>
#include <memory>
#include <string>
#include <vector>

#include "CategorizePhoton.h"  // for CategorizePhoton::Category

class TDirectory;
class TH1D;
class VarBin;

/**
 * @brief Book 1D histograms vs pT, split by photon category
 *        (genuine, misID electron, hadronic photon, hadronic fake, PU)
 *        + one "total" histogram.
 *
 * Directory structure:
 *   outDir/<directoryName>/HistPhotonCategory_<histPrefix_>/
 * with hist names:
 *   h1CountPhoton_Genuine, h1CountPhoton_MisIdEle, ...
 */
class HistPhotonCategory {
public:
    /// Photon categories
    enum PhotonCat : unsigned {
        kGenuine        = 0,
        kMisIdEle       = 1,
        kHadronicPhoton = 2,
        kHadronicFake   = 3,
        kPuPhoton       = 4,
        kNPhotonCats          ///< always = 5
    };

    /**
     * @brief Constructor: books one TH1D per photon category + one "total"
     *        under outDir/<directoryName>/HistPhotonCategory_<histPrefix_>.
     *
     * @param outDir         Pointer to a parent TDirectory (e.g. a subdir of your TFile).
     * @param directoryName  Subdirectory name inside outDir (e.g. "PhotonStudy").
     * @param varBin         VarBin object that provides pT bin edges via getBinsPt().
     * @param histPrefix     Prefix for histogram block (e.g. "Leading", "Ref").
     */
    HistPhotonCategory(
        TDirectory* outDir,
        const std::string& directoryName,
        const VarBin& varBin,
        const std::string& histPrefix
    );

    /// Default destructor (unique_ptr will clean up TH1Ds).
    ~HistPhotonCategory();

    /// Fill histograms based on CategorizePhoton::Category flags.
    void Fill(double pt, const CategorizePhoton::Category& cat, double weight);

    /// Optional: fill a single category explicitly.
    void Fill(double pt, PhotonCat cat, double weight);

    /// Accessors
    TH1D* GetCategoryHist(PhotonCat c) const {
        return catHist_[static_cast<size_t>(c)].get();
    }

    TH1D* GetTotalHist() const { return totalHist_.get(); }

    /// Category names (for histogram names / legend labels).
    static const char* CategoryName[kNPhotonCats];

private:
    // Helper: create a TH1D name = "h1Count" + catName
    static std::string MakeHistName(const std::string& catName);

    /// Book one TH1D per category + one "total" histogram, all with same binning.
    void InitializeHistograms(TDirectory* origDir,
                              const std::string& directoryName,
                              const VarBin& varBin);

    std::array<std::unique_ptr<TH1D>, kNPhotonCats>  catHist_{};
    std::unique_ptr<TH1D>                            totalHist_;

    std::string histPrefix_;  ///< e.g. "Leading", "Ref", etc.
};

