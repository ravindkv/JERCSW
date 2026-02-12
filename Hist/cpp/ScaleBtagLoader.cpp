#include "ScaleBtagLoader.h"

#include <iostream>
#include <stdexcept>

#include "ReadConfig.h"

// ROOT
#include "TFile.h"
#include "TH2.h"

ScaleBtagLoader::ScaleBtagLoader(const GlobalFlag& globalFlags)
    : globalFlags_(globalFlags),
      isDebug_(globalFlags_.isDebug()),
      isData_(globalFlags_.isData()),
      isMC_(globalFlags_.isMC()) {

    loadConfig("config/ScaleBtag.json");
    loadBtvRefs();
    updateEffHistNames_(); // set <effType>_*_efficiency strings
    // efficiency hists stay lazy
}

void ScaleBtagLoader::printConfig() const {
    std::cout << "\n[ScaleBtagLoader::Config]\n"
              << "  btvJsonPath   = " << btvJsonPath_   << '\n'
              << "  btagEffPath   = " << btagEffPath_   << '\n'
              << "  algo          = " << algo_          << '\n'
              << "  wp            = " << wp_            << '\n'
              << "  effType       = " << effType_       << '\n'
              << "  sfPtMin       = " << sfPtMin_       << '\n'
              << "  sfPtMax       = " << sfPtMax_       << '\n'
              << "  sfAbsEtaMax   = " << sfAbsEtaMax_   << '\n'
              << "  wpCut         = " << wpCut_         << "\n\n";
}

void ScaleBtagLoader::loadConfig(const std::string& filename) {
    ReadConfig cfg(filename);
    const std::string year = globalFlags_.getYearStr();

    btvJsonPath_  = cfg.getValue<std::string>({year, "btvJsonPath"});
    btagEffPath_  = cfg.getValue<std::string>({year, "btagEffPath"});
    algo_         = cfg.getValue<std::string>({year, "algo"});
    wp_           = cfg.getValue<std::string>({year, "wp"});
    effType_      = cfg.getValue<std::string>({year, "effType"});

    // Optional overrides (if keys exist in JSON; if not, defaults above remain)
    // If your ReadConfig throws on missing, keep these commented or guard via hasKey().
    // sfPtMin_     = cfg.getValue<double>({year, "sfPtMin"});
    // sfPtMax_     = cfg.getValue<double>({year, "sfPtMax"});
    // sfAbsEtaMax_ = cfg.getValue<double>({year, "sfAbsEtaMax"});
    // wpCut_       = cfg.getValue<double>({year, "wpCut"});

    if (isDebug_) {
        std::cout << "[ScaleBtagLoader] Loaded " << filename << '\n';
        printConfig();
    }
}

void ScaleBtagLoader::loadBtvRefs() {
    try {
        btvSet_ = correction::CorrectionSet::from_file(btvJsonPath_);
        // names match your existing code
        corr_mujets_ = btvSet_->at("deepJet_mujets");
        corr_incl_   = btvSet_->at("deepJet_incl");
    } catch (const std::exception& e) {
        std::cerr << "[ScaleBtagLoader] ERROR loading BTV correctionlib: " << e.what() << '\n';
        throw;
    }
}

void ScaleBtagLoader::updateEffHistNames_() {
    lEffHistName_ = effType_ + "_l_efficiency";
    cEffHistName_ = effType_ + "_c_efficiency";
    bEffHistName_ = effType_ + "_b_efficiency";
}

void ScaleBtagLoader::setEffType(const std::string& effType) {
    effType_ = effType;
    updateEffHistNames_();
    reloadEffHists_ = true;
}

void ScaleBtagLoader::ensureEffHistsLoaded() const {
    loadEffHists_();
}

void ScaleBtagLoader::loadEffHists_() const {
    if (effFile_ && !reloadEffHists_) return;

    effFile_.reset();
    lEff_ = cEff_ = bEff_ = nullptr;
    reloadEffHists_ = false;

    TFile* f = TFile::Open(btagEffPath_.c_str(), "READ");
    if (!f || f->IsZombie()) {
        throw std::runtime_error("ScaleBtagLoader: failed to open btagEffPath: " + btagEffPath_);
    }
    effFile_.reset(f);

    lEff_ = dynamic_cast<TH2*>(effFile_->Get(lEffHistName_.c_str()));
    cEff_ = dynamic_cast<TH2*>(effFile_->Get(cEffHistName_.c_str()));
    bEff_ = dynamic_cast<TH2*>(effFile_->Get(bEffHistName_.c_str()));

    if (!lEff_ || !cEff_ || !bEff_) {
        throw std::runtime_error("ScaleBtagLoader: missing one of efficiency hists {" +
                                 lEffHistName_ + ", " + cEffHistName_ + ", " + bEffHistName_ +
                                 "} in file " + btagEffPath_);
    }

    if (isDebug_) {
        std::cout << "[ScaleBtagLoader] Loaded efficiency hists from " << btagEffPath_ << ":\n"
                  << "  lEff: " << lEffHistName_ << '\n'
                  << "  cEff: " << cEffHistName_ << '\n'
                  << "  bEff: " << bEffHistName_ << '\n';
    }
}

