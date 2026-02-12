#include "ScaleElectronLoader.h"

#include <iostream>

#include "ReadConfig.h"

// ROOT
#include "TFile.h"
#include "TH2.h"

ScaleElectronLoader::ScaleElectronLoader(const GlobalFlag& globalFlags)
    : globalFlags_(globalFlags),
      isDebug_(globalFlags_.isDebug()),
      isData_(globalFlags_.isData()),
      isMC_(globalFlags_.isMC()) {

    loadConfig("config/ScaleElectronLoader.json");
    loadEleSfHists();
    ensureEleSsRefLoaded();
}

void ScaleElectronLoader::loadConfig(const std::string& filename) {
    ReadConfig config(filename);
    const std::string yearStr = globalFlags_.getYearStr();

    // Scale & Smear JSON
    eleSsJsonPath_ = config.getValue<std::string>({yearStr, "eleSsJsonPath"});
    eleSsName_     = config.getValue<std::string>({yearStr, "eleSsName"});

    // SF files & hist names
    eleIdSfPath_   = config.getValue<std::string>({yearStr, "eleIdSfPath"});
    eleRecoSfPath_ = config.getValue<std::string>({yearStr, "eleRecoSfPath"});

    const std::string channelStr = globalFlags_.getChannelStr();
    eleTrigSfPath_ = config.getValue<std::string>({yearStr, "eleTrigSfPath", channelStr});

    eleIdSfHist_   = config.getValue<std::string>({yearStr, "eleIdSfHist"});
    eleRecoSfHist_ = config.getValue<std::string>({yearStr, "eleRecoSfHist"});
    eleTrigSfHist_ = config.getValue<std::string>({yearStr, "eleTrigSfHist"});
    printConfig();
}

void ScaleElectronLoader::printConfig() const {
    std::cout << "\n[ScaleElectronLoader] Config\n"
              << "  eleSsJsonPath          = " << eleSsJsonPath_ << '\n'
              << "  eleSsName              = " << eleSsName_ << '\n'
              << "  eleIdSfPath            = " << eleIdSfPath_ << '\n'
              << "  eleRecoSfPath          = " << eleRecoSfPath_ << '\n'
              << "  eleTrigSfPath          = " << eleTrigSfPath_ << '\n'
              << "  eleIdSfHist            = " << eleIdSfHist_ << '\n'
              << "  eleRecoSfHist          = " << eleRecoSfHist_ << '\n'
              << "  eleTrigSfHist          = " << eleTrigSfHist_ << "\n\n";
}

void ScaleElectronLoader::loadEleSfHists() {
    std::cout<<"===> loadEleSfHists()"<<'\n';
    auto openFile = [&](const std::string& path) -> std::unique_ptr<TFile> {
        TFile* f = TFile::Open(path.c_str(), "READ");
        if (!f || f->IsZombie()) {
            throw std::runtime_error("ScaleElectronLoader: failed to open file: " + path);
        }
        return std::unique_ptr<TFile>(f);
    };

    idFile_   = openFile(eleIdSfPath_);
    recoFile_ = openFile(eleRecoSfPath_);
    trigFile_ = openFile(eleTrigSfPath_);

    idHist_   = dynamic_cast<TH2*>(idFile_->Get(eleIdSfHist_.c_str()));
    recoHist_ = dynamic_cast<TH2*>(recoFile_->Get(eleRecoSfHist_.c_str()));
    trigHist_ = dynamic_cast<TH2*>(trigFile_->Get(eleTrigSfHist_.c_str()));

    if (!idHist_)   throw std::runtime_error("ScaleElectronLoader: missing hist '" + eleIdSfHist_   + "' in " + eleIdSfPath_);
    if (!recoHist_) throw std::runtime_error("ScaleElectronLoader: missing hist '" + eleRecoSfHist_ + "' in " + eleRecoSfPath_);
    if (!trigHist_) throw std::runtime_error("ScaleElectronLoader: missing hist '" + eleTrigSfHist_ + "' in " + eleTrigSfPath_);

    if (isDebug_) {
        std::cout << "[ScaleElectronLoader] Loaded SF hists:\n"
                  << "  ID   : " << eleIdSfPath_   << " :: " << eleIdSfHist_   << '\n'
                  << "  Reco : " << eleRecoSfPath_ << " :: " << eleRecoSfHist_ << '\n'
                  << "  Trig : " << eleTrigSfPath_ << " :: " << eleTrigSfHist_ << '\n';
    }
}

void ScaleElectronLoader::loadEleSsRef() {
    if (hasEleSsRef_) return;
    std::cout<<"===> loadEleSsRef()"<<'\n';
    try {
        loadedEleSsRef_ = correction::CorrectionSet::from_file(eleSsJsonPath_)->at(eleSsName_);
        hasEleSsRef_ = true;
    } catch (const std::exception& e) {
        std::cout << "\nEXCEPTION: ScaleElectronLoader::loadEleSsRef()\n"
                  << "Check " << eleSsJsonPath_ << " or " << eleSsName_ << '\n'
                  << e.what() << '\n';
        throw std::runtime_error("Error loading Electron Scale and Smearing Reference.");
    }
}

void ScaleElectronLoader::ensureEleSsRefLoaded() {
    loadEleSsRef();
}

