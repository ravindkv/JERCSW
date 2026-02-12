#pragma once

#include <memory>
#include <string>
#include <stdexcept>

#include "GlobalFlag.h"
#include "correction.h"

// ROOT forward declarations
class TH2;
class TFile;

class ScaleElectronLoader {
public:
    explicit ScaleElectronLoader(const GlobalFlag& globalFlags);

    // Accessors (non-owning pointers; ownership stays in this loader via TFile)
    const TH2* idHist()   const { return idHist_; }
    const TH2* recoHist() const { return recoHist_; }
    const TH2* trigHist() const { return trigHist_; }

    // Ss correctionlib ref (load on demand or in ctor—here loaded on demand)
    const correction::Correction::Ref& eleSsRef() const { return loadedEleSsRef_; }
    bool hasEleSsRef() const noexcept { return hasEleSsRef_; }
    void ensureEleSsRefLoaded(); // safe to call multiple times

    // Debug printing (optional)
    void printConfig() const;

private:
    void loadConfig(const std::string& filename);
    void loadEleSfHists();
    void loadEleSsRef();

private:
    // -------- Electron Scale & Smearing (Ss) --------
    std::string eleSsJsonPath_;
    std::string eleSsName_;
    correction::Correction::Ref loadedEleSsRef_;
    bool hasEleSsRef_{false};

    // -------- Electron SF config (ROOT files & hist names) --------
    std::string eleIdSfPath_;
    std::string eleRecoSfPath_;
    std::string eleTrigSfPath_;

    std::string eleIdSfHist_;
    std::string eleRecoSfHist_;
    std::string eleTrigSfHist_;

    // ROOT objects (owned here)
    std::unique_ptr<TFile> idFile_;
    std::unique_ptr<TFile> recoFile_;
    std::unique_ptr<TFile> trigFile_;

    TH2* idHist_{nullptr};    // non-owning; owned by files above
    TH2* recoHist_{nullptr};
    TH2* trigHist_{nullptr};

    // Global flags
    const GlobalFlag& globalFlags_;
    const bool isDebug_;
    const bool isData_;
    const bool isMC_;
};

