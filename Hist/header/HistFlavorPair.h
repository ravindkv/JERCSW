#pragma once

#include <array>
#include <memory>
#include <string>
#include <vector>

class TDirectory;
class TH1D;
class VarBin;

class HistFlavorPair {
public:
    /// Pair categories (order-independent)
    enum Pair : unsigned {
        kGG = 0,     ///< both gluons (21,21)
        kGQ = 1,     ///< one gluon (21) + one quark (|pdg| in {1..5})
        kQQ = 2,     ///< both quarks (|pdg| in {1..5})
        kOther = 3,  ///< anything else (0, 6, 7, ... mixed with {Q/G} or both invalid)
        kNPairs      ///< always = 4
    };

    /**
     * @brief Constructor: books one TH1D per pair + one "total" under
     *        outDir/<directoryName>/HistFlavorPair, using histPrefix.
     *
     * @param outDir         Parent TDirectory (e.g. subdir of your TFile).
     * @param directoryName  Subdir inside outDir (e.g. "FlavorStudy").
     * @param varBin         Provides pT-bin edges (getBinsPt()).
     * @param histPrefix     Prefix for histogram names (e.g. "h_leading" or "h_recoil").
     */
    HistFlavorPair(
        TDirectory* outDir,
        const std::string& directoryName,
        const VarBin& varBin,
        const std::string& histPrefix
    );

    ~HistFlavorPair();

    /**
     * @brief Fill exactly one set of histograms (pair-binned + total) at pT = pt.
     *
     * Quark = |pdg| ∈ {1..5}, Gluon = 21, everything else → Other.
     * Order is ignored: (g,q) == (q,g).
     *
     * @param pt         
     * @param partonFlav1   PDG flavor of parton #1 (e.g. GenJet_partonFlavor of jet1).
     * @param partonFlav2   PDG flavor of parton #2 (e.g. GenJet_partonFlavor of jet2).
     * @param weight        Event weight.
     */
    void Fill(double pt, int partonFlav1, int partonFlav2, double weight);

    /// Accessors
    TH1D*        GetPairHist(Pair p)     const { return pairHist_[static_cast<size_t>(p)].get(); }
    TH1D*        GetTotalHist()          const { return totalHist_.get(); }

    /// Names for legend/title convenience
    static const char* PairName[kNPairs];

private:
    enum Kind : unsigned { kQ=0, kG=1, kX=2 }; // Quark, Gluon, Other

    static Kind  PartonToKind(int absPartonFlav) noexcept;
    static Pair  KindToPair(Kind a, Kind b) noexcept;
    static std::string MakeHistName(const std::string& pairName);

    void InitializeHistograms(TDirectory* origDir, const std::string& directoryName, const VarBin& varBin);

    std::array<std::unique_ptr<TH1D>, kNPairs>  pairHist_{};
    std::unique_ptr<TH1D>                       totalHist_;
    std::string                                 histPrefix_;
};

