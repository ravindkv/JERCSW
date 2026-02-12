#pragma once

#include <string>
#include <memory>  // for std::unique_ptr

// forward declarations
class TH2D;
class TH1D;
class TDirectory;
class VarBin;
class SkimTree;

/**
 * @brief Struct to hold all final histograms for specific directories.
 */
struct HistElectronVar {
    std::unique_ptr<TH2D> h2EventInElePtEleEta;
};

struct HistPhotonVar {
    std::unique_ptr<TH2D> h2EventInPhoEtaPhoPt;
    std::unique_ptr<TH2D> h2EventInPhoCountPhoPt;
    std::unique_ptr<TH2D> h2EventInPhoR9PhoPt;
    std::unique_ptr<TH2D> h2EventInPhoHoePhoPt;
    std::unique_ptr<TH2D> h2EventInPhoIdPhoPt;
};

struct HistMuonVar {
    std::unique_ptr<TH2D> h2EventInMuPtMuEta;
};

struct HistJetVar {
    //All jets
    std::unique_ptr<TH2D> h2EventInJetPtJetEta;
    std::unique_ptr<TH1D> h1EventInJetChHef;
    std::unique_ptr<TH1D> h1EventInJetNeHef;
    std::unique_ptr<TH1D> h1EventInJetNeEmef;
    std::unique_ptr<TH1D> h1EventInJetChEmef;
    std::unique_ptr<TH1D> h1EventInJetMuef;
    //Leading jet
    std::unique_ptr<TH2D> h2EventInJet1PtJetEta;
    std::unique_ptr<TH1D> h1EventInJet1ChHef;
    std::unique_ptr<TH1D> h1EventInJet1NeHef;
    std::unique_ptr<TH1D> h1EventInJet1NeEmef;
    std::unique_ptr<TH1D> h1EventInJet1ChEmef;
    std::unique_ptr<TH1D> h1EventInJet1Muef;
};

struct HistMetVar {
    std::unique_ptr<TH2D> h2EventInMetPtMetPhi;
};
/**
 * @brief Class to handle histograms related to jet energy corrections.
 * 
 * This class encapsulates the creation and filling of histograms that monitor
 * variables after applying jet energy corrections. It manages histograms using ROOT's TH1D.
 */
class HistObjectVar {
public:
    /**
     * @brief Constructs a HistObjectVar object and initializes histograms.
     * 
     * @param origDir Pointer to the original ROOT directory.
     * @param directoryName Name of the directory within the ROOT file to store histograms.
     * @param varBin Object defining variable binning.
     */
    HistObjectVar(TDirectory* origDir, const std::string& directoryName, const VarBin& varBin);
    
    /**
     * @brief Default destructor.
     */
    ~HistObjectVar();
    
    void FillElectron(const SkimTree& skimT, double weight);
    void FillPhoton(const SkimTree& skimT, double weight);
    void FillMuon(const SkimTree& skimT, double weight);
    void FillJet(const SkimTree& skimT, double weight, int iJet1);
    void FillMet(const SkimTree& skimT, double weight);

private:
    // Struct holding all final histograms
    HistElectronVar histEle_;
    HistPhotonVar histPho_;
    HistMuonVar histMu_;
    HistJetVar histJet_;
    HistMetVar histMet_;
    
    void InitializeHistograms(TDirectory* origDir, const std::string& directoryName, const VarBin& varBin);
};

