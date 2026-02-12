
#pragma once

#include <array>
#include <memory>
#include <string>
#include <vector>

class TDirectory;
class TH1D;
class VarBin;

class HistFlavor {
public:
    /// Flavor categories
    enum Flavor : unsigned {
        kUD = 0,   ///< u/d‐jets (|pdgFlav| ≤ 2)
        kS  = 1,   ///< s‐jets (|pdgFlav| = 3)
        kC  = 2,   ///< c‐jets (|pdgFlav| = 4)
        kB  = 3,   ///< b‐jets (|pdgFlav| = 5)
        kG  = 4,   ///< g‐jets (|pdgFlav| = 21)
        kOther = 5,
        kNFlavors   ///< always = 6
    };

    /**
     * @brief Constructor: books one TH1D per flavor + one "total" under 
     *        `outDir/<directoryName>/HistFlavor`, using `histPrefix` .
     *
     * @param outDir         Pointer to a parent TDirectory (e.g. a subdirectory of your TFile).
     * @param directoryName  Name of the subdirectory inside `outDir` (e.g. "FlavorStudy").
     * @param varBin         A VarBin object that provides pT‐bin edges (getBinsPt()).
     * @param histPrefix     Prefix for histogram names (e.g. "h_leading" or "h_recoil").
     */
    HistFlavor(
        TDirectory* outDir,
        const std::string& directoryName,
        const VarBin& varBin,
        const std::string& histPrefix
    );

    /// Default destructor (unique_ptr will clean up TH1Ds).
    ~HistFlavor();

    void Fill(double pt, int partonFlav, double weight);

    /// Return a pointer to the TH1D of a given flavor (0..kNFlavors-1).
    TH1D*        GetFlavorHist(Flavor f)       const { return flavorHist_[static_cast<size_t>(f)].get(); }

    /// Return a pointer to the "total" histogram (sum over all flavors).
    TH1D*        GetTotalHist()                const { return totalHist_.get(); }

    /// Return the internal list of flavor‐names (e.g. "ud-jets", "s-jets", …).
    static const char* FlavorName[kNFlavors];

private:
    // Helper: create a TH1D name = "<histPrefix>_<flavName>"
    static std::string MakeHistName(const std::string& flavName);

    // Map |partonFlav| → our Flavor enum. If not recognized, returns kUD but caller should skip if out of range.
    static Flavor PartonFlavToEnum(int absPartonFlav) noexcept;

    /**
     * @brief Book one TH1D per flavor + one "total" histogram, all with identical binning.
     * @param dir         The TDirectory in which to create TH1Ds.
     * @param varBin      Supplies the bin edges via varBin.getBinsPt().
     */
    void InitializeHistograms(TDirectory* origDir, const std::string& directoryName, const VarBin& varBin);

    std::array<std::unique_ptr<TH1D>, kNFlavors>  flavorHist_{};
    std::unique_ptr<TH1D>                         totalHist_;

    std::string histPrefix_;  ///< e.g. "h_leading" or "h_recoil"
};

