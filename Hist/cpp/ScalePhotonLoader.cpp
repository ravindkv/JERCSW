#include "ScalePhotonLoader.h"

#include <iostream>

#include "ReadConfig.h"

// ROOT
#include "TFile.h"
#include "TH2.h"
#include "TH1F.h"

ScalePhotonLoader::ScalePhotonLoader(const GlobalFlag& globalFlags)
    : globalFlags_(globalFlags),
      isDebug_(globalFlags_.isDebug()),
      isData_(globalFlags_.isData()),
      isMC_(globalFlags_.isMC()) {

    loadConfig("config/ScalePhotonLoader.json");
    loadPhotonSsRef();
    loadPhotonSfHists();
}

void ScalePhotonLoader::loadConfig(const std::string& filename) {
    ReadConfig config(filename);
    const std::string yearStr = globalFlags_.getYearStr();

    // Scale/Smear
    phoSsJsonPath_ = config.getValue<std::string>({yearStr, "phoSsJsonPath"});
    phoSsName_     = config.getValue<std::string>({yearStr, "phoSsName"});

    // SF files (paths)
    phoIdSfPath_ = config.getValue<std::string>({yearStr, "phoIdSfPath"});
    phoPsSfPath_ = config.getValue<std::string>({yearStr, "phoPsSfPath"});
    phoCsSfPath_ = config.getValue<std::string>({yearStr, "phoCsSfPath"});

    // SF hist names
    phoIdSfHist_ = config.getValue<std::string>({yearStr, "phoIdSfHist"});
    phoPsSfHist_ = config.getValue<std::string>({yearStr, "phoPsSfHist"});
    phoCsSfHist_ = config.getValue<std::string>({yearStr, "phoCsSfHist"});

    if (isDebug_) printConfig();
}

void ScalePhotonLoader::printConfig() const {
    std::cout << "\n[ScalePhotonLoader] Config\n"
              << "  phoSsJsonPath          = " << phoSsJsonPath_ << '\n'
              << "  phoSsName              = " << phoSsName_ << '\n'
              << "  phoIdSfPath            = " << phoIdSfPath_ << '\n'
              << "  phoPsSfPath            = " << phoPsSfPath_ << '\n'
              << "  phoCsSfPath            = " << phoCsSfPath_ << '\n'
              << "  phoIdSfHist            = " << phoIdSfHist_ << '\n'
              << "  phoPsSfHist            = " << phoPsSfHist_ << '\n'
              << "  phoCsSfHist            = " << phoCsSfHist_ << '\n';
}

void ScalePhotonLoader::loadPhotonSsRef() {
    if (isDebug_) std::cout << "==> ScalePhotonLoader::loadPhotonSsRef()\n";
    try {
        loadedPhotonSsRef_ =
            correction::CorrectionSet::from_file(phoSsJsonPath_)->at(phoSsName_);
    } catch (const std::exception& e) {
        std::cout << "\nEXCEPTION: ScalePhotonLoader::loadPhotonSsRef()\n"
                  << "Check " << phoSsJsonPath_ << " or " << phoSsName_ << '\n'
                  << e.what() << '\n';
        throw std::runtime_error("Error loading Photon Scale and Smearing Reference.");
    }
}

void ScalePhotonLoader::loadPhotonSfHists() {
    auto openFile = [&](const std::string& path) -> std::unique_ptr<TFile> {
        TFile* f = TFile::Open(path.c_str(), "READ");
        if (!f || f->IsZombie()) {
            throw std::runtime_error("ScalePhotonLoader: failed to open file: " + path);
        }
        return std::unique_ptr<TFile>(f);
    };

    idFile_ = openFile(phoIdSfPath_);
    psFile_ = openFile(phoPsSfPath_);
    csFile_ = openFile(phoCsSfPath_);

    idHist_ = dynamic_cast<TH2*>(idFile_->Get(phoIdSfHist_.c_str()));
    psHist_ = dynamic_cast<TH1F*>(psFile_->Get(phoPsSfHist_.c_str()));
    csHist_ = dynamic_cast<TH1F*>(csFile_->Get(phoCsSfHist_.c_str()));

    if (!idHist_) throw std::runtime_error("ScalePhotonLoader: missing hist '" + phoIdSfHist_ + "' in " + phoIdSfPath_);
    if (!psHist_) throw std::runtime_error("ScalePhotonLoader: missing hist '" + phoPsSfHist_ + "' in " + phoPsSfPath_);
    if (!csHist_) throw std::runtime_error("ScalePhotonLoader: missing hist '" + phoCsSfHist_ + "' in " + phoCsSfPath_);

    if (isDebug_) {
        std::cout << "[ScalePhotonLoader] Loaded SF hists:\n"
                  << "  ID  : " << phoIdSfPath_ << " :: " << phoIdSfHist_ << '\n'
                  << "  PS  : " << phoPsSfPath_ << " :: " << phoPsSfHist_ << '\n'
                  << "  CS  : " << phoCsSfPath_ << " :: " << phoCsSfHist_ << '\n';
    }
}

