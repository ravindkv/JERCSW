#pragma once

#include <memory>
#include <string>
#include <stdexcept>

#include "GlobalFlag.h"
#include "correction.h"

// ROOT fwd decls
class TH2;
class TH1F;
class TFile;

class ScalePhotonLoader {
public:
    explicit ScalePhotonLoader(const GlobalFlag& globalFlags);

    // SF hist access (non-owning pointers; owned by TFiles inside loader)
    const TH2*  idHist() const { return idHist_; }
    const TH1F* psHist() const { return psHist_; }
    const TH1F* csHist() const { return csHist_; }

    // correctionlib reference
    const correction::Correction::Ref& phoSsRef() const { return loadedPhotonSsRef_; }

    // Optional debug
    void printConfig() const;

private:
    void loadConfig(const std::string& filename);

    // correctionlib
    void loadPhotonSsRef();

    // SFs
    void loadPhotonSfHists();

private:
    // ----- Config: Scale/Smear -----
    std::string phoSsJsonPath_;
    std::string phoSsName_;
    correction::Correction::Ref loadedPhotonSsRef_;

    // ----- Config: SF files & hists -----
    std::string phoIdSfPath_;
    std::string phoPsSfPath_;
    std::string phoCsSfPath_;
    std::string phoIdSfHist_;
    std::string phoPsSfHist_;
    std::string phoCsSfHist_;

    // ROOT ownership
    std::unique_ptr<TFile> idFile_;
    std::unique_ptr<TFile> psFile_;
    std::unique_ptr<TFile> csFile_;
    TH2*  idHist_{nullptr};   // borrowed from idFile_
    TH1F* psHist_{nullptr};   // borrowed from psFile_
    TH1F* csHist_{nullptr};   // borrowed from csFile_

    // Globals
    const GlobalFlag& globalFlags_;
    const bool isDebug_;
    const bool isData_;
    const bool isMC_;
};

