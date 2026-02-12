#include "ScaleMuonLoader.h"

#include <iostream>

#include "ReadConfig.h"

// ROOT
#include "TFile.h"
#include "TH2.h"

ScaleMuonLoader::ScaleMuonLoader(const GlobalFlag& globalFlags)
    : globalFlags_(globalFlags),
      isDebug_(globalFlags_.isDebug()),
      isData_(globalFlags_.isData()),
      isMC_(globalFlags_.isMC()) {

    loadConfig("config/ScaleMuon.json");
    loadMuRochRef();
    loadMuSfHists();
}

void ScaleMuonLoader::loadConfig(const std::string& filename) {
    ReadConfig config(filename);
    const std::string yearStr = globalFlags_.getYearStr();

    // Rochester
    muRochJsonPath_ = config.getValue<std::string>({yearStr, "muRochJsonPath"});

    // SF files (paths)
    muIdSfPath_   = config.getValue<std::string>({yearStr, "muIdSfPath"});
    muIsoSfPath_  = config.getValue<std::string>({yearStr, "muIsoSfPath"});
    const std::string channelStr = globalFlags_.getChannelStr();
    muTrigSfPath_ = config.getValue<std::string>({yearStr, "muTrigSfPath", channelStr});

    // SF hist names
    muIdSfHist_   = config.getValue<std::string>({yearStr, "muIdSfHist"});
    muIsoSfHist_  = config.getValue<std::string>({yearStr, "muIsoSfHist"});
    muTrigSfHist_ = config.getValue<std::string>({yearStr, "muTrigSfHist", channelStr});

    if (isDebug_) printConfig();
}

void ScaleMuonLoader::printConfig() const {
    std::cout << "\n[ScaleMuonLoader] Config\n"
              << "  muRochJsonPath         = " << muRochJsonPath_ << '\n'
              << "  muIdSfPath             = " << muIdSfPath_   << '\n'
              << "  muIsoSfPath            = " << muIsoSfPath_  << '\n'
              << "  muTrigSfPath           = " << muTrigSfPath_ << '\n'
              << "  muIdSfHist             = " << muIdSfHist_   << '\n'
              << "  muIsoSfHist            = " << muIsoSfHist_  << '\n'
              << "  muTrigSfHist           = " << muTrigSfHist_ << "\n\n";
}

void ScaleMuonLoader::loadMuRochRef() {
    if (isDebug_) std::cout << "==> ScaleMuonLoader::loadMuRochRef()\n";
    try {
        loadedRochRef_.init(muRochJsonPath_);
    } catch (const std::exception& e) {
        std::cout << "\nEXCEPTION: ScaleMuonLoader::loadMuRochRef()\n"
                  << "Check " << muRochJsonPath_ << '\n'
                  << e.what() << '\n';
        throw std::runtime_error("Error loading muon Rochester corr file.");
    }
}

void ScaleMuonLoader::loadMuSfHists() {
    auto openFile = [&](const std::string& path) -> std::unique_ptr<TFile> {
        TFile* f = TFile::Open(path.c_str(), "READ");
        if (!f || f->IsZombie()) {
            throw std::runtime_error("ScaleMuonLoader: failed to open file: " + path);
        }
        return std::unique_ptr<TFile>(f);
    };

    idFile_   = openFile(muIdSfPath_);
    isoFile_  = openFile(muIsoSfPath_);
    trigFile_ = openFile(muTrigSfPath_);

    idHist_   = dynamic_cast<TH2*>(idFile_->Get(muIdSfHist_.c_str()));
    isoHist_  = dynamic_cast<TH2*>(isoFile_->Get(muIsoSfHist_.c_str()));
    trigHist_ = dynamic_cast<TH2*>(trigFile_->Get(muTrigSfHist_.c_str()));

    if (!idHist_)   throw std::runtime_error("ScaleMuonLoader: missing hist '" + muIdSfHist_   + "' in " + muIdSfPath_);
    if (!isoHist_)  throw std::runtime_error("ScaleMuonLoader: missing hist '" + muIsoSfHist_  + "' in " + muIsoSfPath_);
    if (!trigHist_) throw std::runtime_error("ScaleMuonLoader: missing hist '" + muTrigSfHist_ + "' in " + muTrigSfPath_);

    if (isDebug_) {
        std::cout << "[ScaleMuonLoader] Loaded SF hists:\n"
                  << "  ID   : "  << muIdSfPath_   << " :: " << muIdSfHist_   << '\n'
                  << "  ISO  : "  << muIsoSfPath_  << " :: " << muIsoSfHist_  << '\n'
                  << "  TRIG : "  << muTrigSfPath_ << " :: " << muTrigSfHist_ << '\n';
    }
}

