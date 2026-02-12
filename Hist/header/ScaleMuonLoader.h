#pragma once

#include <memory>
#include <string>
#include <stdexcept>

#include "GlobalFlag.h"
#include "RoccoR.h"
#include "TRandom3.h"

// ROOT forward decls
class TH2;
class TFile;

class ScaleMuonLoader {
public:
    explicit ScaleMuonLoader(const GlobalFlag& globalFlags);

    // SF hist access (non-owning; owned by TFiles inside this loader)
    const TH2* idHist()   const { return idHist_; }
    const TH2* isoHist()  const { return isoHist_; }
    const TH2* trigHist() const { return trigHist_; }

    // Rochester + RNG access
    const RoccoR& roch() const { return loadedRochRef_; }
    TRandom& rng() const { return *randomNumGen_; }

    // Optional debug
    void printConfig() const;

private:
    // Config
    void loadConfig(const std::string& filename);

    // Rochester
    void loadMuRochRef();

    // SFs
    void loadMuSfHists();

private:
    // ------------- Config: Rochester -------------
    std::string muRochJsonPath_;
    RoccoR loadedRochRef_;
    mutable std::unique_ptr<TRandom> randomNumGen_{std::make_unique<TRandom3>(0)};

    // ------------- Config: SF files & hists -------------
    std::string muIdSfPath_;
    std::string muIsoSfPath_;
    std::string muTrigSfPath_;
    std::string muIdSfHist_;
    std::string muIsoSfHist_;
    std::string muTrigSfHist_;

    // ROOT ownership for SFs
    std::unique_ptr<TFile> idFile_;
    std::unique_ptr<TFile> isoFile_;
    std::unique_ptr<TFile> trigFile_;
    TH2* idHist_{nullptr};   // owned by files above
    TH2* isoHist_{nullptr};
    TH2* trigHist_{nullptr};

    // Global
    const GlobalFlag& globalFlags_;
    const bool isDebug_;
    const bool isData_;
    const bool isMC_;
};

