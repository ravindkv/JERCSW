
#pragma once

#include <string>
#include <memory>    // for std::unique_ptr

// forward‐declare all external types
class TH2D;
class TProfile;
class TLorentzVector;
class TDirectory;
class VarBin;

/**
 * @brief Struct to hold all histograms related to the final selection.
 */
struct FinalHistograms {
    // 2D Histograms
    std::unique_ptr<TH2D> h2EventInDbRespRefPt;
    std::unique_ptr<TH2D> h2EventInMpfRespRefPt;

    // Additional TProfile Histograms
    std::unique_ptr<TProfile> p1GenJet2PtOverRefPtInRefPt;
    std::unique_ptr<TProfile> p1Jet2PtOverRefPtInRefPt;
    std::unique_ptr<TProfile> p1Jet2PtOverGenJet2PtInGenJet2Pt;

    // Conditional 2D Histograms
    std::unique_ptr<TH2D> h2EventInDbRespRefPtForPassMpf;
    std::unique_ptr<TH2D> h2EventInMpfRespRefPtForPassDb;
    std::unique_ptr<TH2D> h2EventInDbRespRefPtForPassBoth;
    std::unique_ptr<TH2D> h2EventInMpfRespRefPtForPassBoth;
};

/**
 * @brief Class to handle histograms related to the final selection criteria.
 * 
 * This class encapsulates the creation and filling of histograms that monitor
 * response metrics and jet properties after the final selection. It manages
 * 2D histograms and TProfile histograms using ROOT's TH2D and TProfile classes.
 */
class HistFinal {
public:
    /**
     * @brief Constructs a HistFinal object and initializes histograms.
     * 
     * @param fout Pointer to the output ROOT file.
     * @param origDir Pointer to the original ROOT directory.
     * @param directoryName Name of the directory within the ROOT file to store histograms.
     * @param varBin Reference to the VarBin object containing binning configurations.
     */
    HistFinal(TDirectory* origDir, const std::string& directoryName, const VarBin& varBin);
    
    /**
     * @brief Default destructor.
     */
    ~HistFinal();
    
    /**
     * @brief Fills the final histograms based on event data.
     * 
     * This function fills various histograms related to response metrics and jet properties.
     * It handles 2D histograms and TProfile histograms, including conditional fills based
     * on response criteria.
     * 
     * @param ptRef Reference pT value.
     * @param bal Balance response metric.
     * @param mpf MPF response metric.
     * @param p4Jet2 TLorentzVector of the second jet.
     * @param p4GenJet2 TLorentzVector of the generated second jet.
     * @param iGenJet2 Index of the generated second jet (-1 if not matched).
     * @param isMC Flag indicating if the event is from Monte Carlo simulation.
     * @param weight Event weight.
     */
    void Fill(double ptRef, double bal, double mpf,
              const TLorentzVector& p4Jet2, const TLorentzVector& p4GenJet2,
              int iGenJet2, bool isMC, double weight);

    /**
     * @brief Fills the common histograms based on event data.
     * 
     * This function fills histograms related to response metrics that are common
     * regardless of gen-level matching.
     * 
     * @param ptRef Reference pT value.
     * @param bal Balance response metric.
     * @param mpf MPF response metric.
     * @param weight Event weight.
     */
    void FillCommon(double ptRef, double bal, double mpf, bool passDbResp, bool passMpfResp, double weight);
    
    /**
     * @brief Fills the gen-specific histograms based on event data.
     * 
     * This function fills TProfile histograms that require gen-level matching.
     * 
     * @param ptRef Reference pT value.
     * @param p4Jet2 TLorentzVector of the second jet.
     * @param p4GenJet2 TLorentzVector of the generated second jet.
     * @param iGenJet2 Index of the generated second jet (-1 if not matched).
     * @param weight Event weight.
     */
    void FillGen(double ptRef, const TLorentzVector& p4Jet2, const TLorentzVector& p4GenJet2, double weight);


private:
    // Struct holding all histograms
    FinalHistograms hist_;
    
    /**
     * @brief Initializes all histograms and sets up the internal structures.
     * 
     * @param fout Pointer to the output ROOT file.
     * @param origDir Pointer to the original ROOT directory.
     * @param directoryName Name of the directory within the ROOT file to store histograms.
     * @param varBin Reference to the VarBin object containing binning configurations.
     */
    void InitializeHistograms(TDirectory* origDir, const std::string& directoryName, const VarBin& varBin);
};
