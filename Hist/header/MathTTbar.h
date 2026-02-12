#pragma once

#include "TLorentzVector.h"
#include <vector>
#include <string>

#include "SkimTree.h"
#include "GlobalFlag.h"

/**
 * @brief Class for reconstructing the ttbar decay hypothesis by minimizing a χ².
 *
 * This class loops over combinations of b–jet and light–jet candidates,
 * updates the neutrino momentum based on MathMetPz, and then calculates a χ²
 * using the invariant masses of the reconstructed top and W candidates.
 */
class MathTTbar {
public:
    /// Constructor takes a reference to GlobalFlag and a bTag threshold
    explicit MathTTbar(const GlobalFlag& globalFlags, double bTagThresh);

    // Set the event four-vectors and the list of jet indices (unseparated).
    // We'll separate them internally in minimizeChiSqr().
    void setEventObjects(const TLorentzVector& p4Lep, 
                         const TLorentzVector& p4Met, 
                         const std::vector<int>& indexJets);

    // Set resolution parameters. 
    void setResolution(double resLep, 
                       double resMet, 
                       double resHadW, 
                       double resHadT, 
                       double resLepT, 
                       const std::vector<double>& resJets, 
                       bool useRes);

    void setMass(double massLep, double massW, double massT);

    /// Main calculation function that minimizes the χ² for the ttbar hypothesis.
    /// Returns 0 on success, or -1 if there are insufficient objects.
    int minimizeChiSqr(const SkimTree& skimT);

    // Getters for the reconstructed four-vectors.
    void setFinalP4s(const SkimTree& skimT);

    TLorentzVector getP4HadB() const { return p4HadB_; } 
    TLorentzVector getP4Jet1() const { return p4Jet1_; } 
    TLorentzVector getP4Jet2() const { return p4Jet2_; } 
    TLorentzVector getP4LepB() const { return p4LepB_; } 
    TLorentzVector getP4Lep() const { return p4Lep_;   } 
    TLorentzVector getP4FullMet() const { return p4FullMet_; } 

    TLorentzVector getP4HadT() const { return p4HadT_; } 
    TLorentzVector getP4HadW() const { return p4HadW_; } 
    TLorentzVector getP4LepT() const { return p4LepT_; } 

    double getChiSqr() const { return chiSqr_; }
    bool goodCombination() const { return goodCombo_; }

    const int getIndex1ForW() const {return index1ForW_;}
    const int getIndex2ForW() const {return index2ForW_;}
    /// Debug print function.
    void print() const;

private:
    // Calculate the χ² for a given hypothesis.
    double calcChiSqr(const SkimTree& skimT, int iHadB, int iLepB,
                      int iJet1, int iJet2, double pzMetHypo);

    //--------------------------------- 
    // Input objects
    TLorentzVector p4Lep_;
    TLorentzVector p4Met_;
    std::vector<int> indexJets_; ///< All jet indices for the event

    // Separated after we read from indexJets_ in minimizeChiSqr()
    std::vector<int> indexJetsB_;    ///< Exactly 2 b-jets chosen
    std::vector<int> indexJetsNonB_; ///< All other jets

    //--------------------------------- 
    double massLep_;
    double massW_;
    double massT_;

    //--------------------------------- 
    // Resolutions
    double resLep_;
    double resMet_;
    double resHadW_;
    double resHadT_;
    double resLepT_;
    std::vector<double> resJets_;
    bool useResolutions_;

    //--------------------------------- 
    // Minimization results
    double chiSqr_;
    bool goodCombo_;

    TLorentzVector p4HadB_;  
    TLorentzVector p4Jet1_;  
    TLorentzVector p4Jet2_;  
    TLorentzVector p4LepB_;  
    TLorentzVector p4FullMet_;  

    TLorentzVector p4HadT_;  // Reconstructed hadronic top four-vector.
    TLorentzVector p4HadW_;  // Reconstructed hadronic W four-vector.
    TLorentzVector p4LepT_;  // Reconstructed leptonic top four-vector.

    int indexLepB_;
    int indexHadB_;
    int index1ForW_;
    int index2ForW_;
    double pzMet_;  // Selected neutrino pz value.

    // bTag threshold 
    double bTagThresh_;

    // Reference to the global flags.
    const GlobalFlag& globalFlags_;
};

