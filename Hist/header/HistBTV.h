#pragma once

#include <memory>           // std::unique_ptr
#include <string>           // std::string
#include <unordered_map>    // std::unordered_map
#include <vector>           // std::vector

// need the full GlobalFlag for its nested enum Year
#include "GlobalFlag.h"

// forward‐declare everything else
class TDirectory;
class TH1;
class VarBin;
class SkimTree;

class HistBTV {
public:
    /**
     * @brief Constructs a HistBTV object and initializes histograms.
     * 
     * @param fout Pointer to the output ROOT file.
     * @param directoryName Name of the directory within the ROOT file to store histograms.
     */
    HistBTV(TDirectory *origDir, const std::string& directoryName, const VarBin& varBin, const GlobalFlag& globalFlags);
    
    /**
     * @brief Default destructor.
     */
    ~HistBTV();

    /**
     * @brief Fills the histograms based on the event data.
     * 
     * This function computes the necessary variables, evaluates tagging conditions,
     * and fills the corresponding histograms.
     * 
     * @param skimT Pointer to the SkimTree containing event data.
     * @param iJet1 Index of the first jet in the event.
     * @param iGenJet Index of the generated jet (for MC simulations).
     */

    void FillHistograms(
        SkimTree* skimT,
        double ptRef,
        int iJet1,
        int iGenJet,
        double weight
    );
    void SetResponse(const double &bal, const double &mpf, const double &mpf1, const double &mpfn, const double &mpfu);

private:
    // Nested unordered_map to store histograms: var -> tag -> flavor -> histogram
    std::unordered_map<std::string, std::unordered_map<std::string, std::unordered_map<std::string, std::unique_ptr<TH1>>>> varTagFlvMap_;
    
    // Tag and flavor identifiers
    std::vector<std::string> atag_;
    std::vector<std::string> aflv_;

    //https://btv-wiki.docs.cern.ch/ScaleFactors/UL2016postVFP/
    double threshBtagDeepFlavB_; 
    double threshBtagDeepFlavCvL_;
    double threshBtagDeepFlavCvB_;
    double threshBtagDeepFlavG_;
    double threshBtagDeepFlavQG_;
    double threshBtagDeepFlavUDS_;

    // Map to hold variable values
    std::unordered_map<std::string, double> mvar_;
    double bal_;
    double mpf_;
    double mpf1_;
    double mpfn_;
    double mpfu_;

    /**
     * @brief Initializes all histograms and sets up the internal unordered_maps.
     * 
     * @param fout Pointer to the output ROOT file.
     * @param directoryName Name of the directory within the ROOT file to store histograms.
     */
    const GlobalFlag::Year year_;
    const bool isMC_;
    const bool isDebug_;
    void InitializeHistograms(TDirectory *origDir, const std::string& directoryName, const VarBin& varBin);
};

