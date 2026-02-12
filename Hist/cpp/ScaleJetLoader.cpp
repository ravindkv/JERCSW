#include "ScaleJetLoader.h"
#include <iostream>
#include <stdexcept>
#include "ReadConfig.h"

ScaleJetLoader::ScaleJetLoader(const GlobalFlag& globalFlags)
    : globalFlags_(globalFlags),
      year_(globalFlags_.getYear()),
      era_(globalFlags_.getEra()),
      level_(globalFlags_.getJecApplicationLevel()),
      jetAlgo_(globalFlags_.getJetAlgo()),
      channel_(globalFlags_.getChannel()),
      isDebug_(globalFlags_.isDebug()),
      isData_(globalFlags_.isData()),
      isMC_(globalFlags_.isMC()),
      isBookKeep_(globalFlags_.isBookKeep()),
      applyJer_(true)
{
    std::string fName = "config/ScaleJetLoader.json";
    switch (jetAlgo_) {
        case GlobalFlag::JetAlgo::AK4Chs:
            fName = "config/ScaleJetLoaderAK4Chs.json";
            break;
        case GlobalFlag::JetAlgo::AK4Puppi:
            fName = "config/ScaleJetLoaderAK4Puppi.json";
            break;
        case GlobalFlag::JetAlgo::AK8Puppi:
            fName = "config/ScaleJetLoaderAK8Puppi.json";
            break;
        default:
            throw std::runtime_error("ScaleJetLoader: unsupported JetAlgo");
    }
    loadConfig(fName);

    loadRefs();
}

void ScaleJetLoader::loadConfig(const std::string& filename) {
    ReadConfig config(filename);
    std::string yearStr = globalFlags_.getYearStr();

    jercJsonPath_  = config.getValue<std::string>({yearStr, "jercJsonPath"});
    JerResoName_   = config.getValue<std::string>({yearStr, "JerResoName"});
    JerSfName_     = config.getValue<std::string>({yearStr, "JerSfName"});

    if (isMC_) {
        jetL1FastJetName_  = config.getValue<std::string>({yearStr, "jetL1FastJetName"});
        jetL2RelativeName_ = config.getValue<std::string>({yearStr, "jetL2RelativeName"});
        jetL3AbsoluteName_ = config.getValue<std::string>({yearStr, "jetL3AbsoluteName"});
        jetL2ResidualName_ = config.getValue<std::string>({yearStr, "jetL2ResidualName"});
        jetL2L3ResidualName_ = config.getValue<std::string>({yearStr, "jetL2L3ResidualName"});
    } else if (isData_) {
        std::string eraStr = globalFlags_.getEraStr();
        jetL1FastJetName_  = config.getValue<std::string>({yearStr, "data", eraStr, "jetL1FastJetName"});
        jetL2RelativeName_ = config.getValue<std::string>({yearStr, "data", eraStr, "jetL2RelativeName"});
        jetL3AbsoluteName_ = config.getValue<std::string>({yearStr, "data", eraStr, "jetL3AbsoluteName"});
        jetL2ResidualName_ = config.getValue<std::string>({yearStr, "data", eraStr, "jetL2ResidualName"});
        jetL2L3ResidualName_ = config.getValue<std::string>({yearStr, "data", eraStr, "jetL2L3ResidualName"});
    }

    auto cset = correction::CorrectionSet::from_file("POG/JME/jer_smear.json.gz");
    jerSmearRef_ = cset->at("JERSmear");  

    std::cout << "\n[ScaleJetLoader] " << filename << '\n';
    std::cout << "  jercJsonPath           = " << jercJsonPath_ << '\n';
    std::cout << "  jetL1FastJetName       = " << jetL1FastJetName_ << '\n';
    std::cout << "  jetL2RelativeName      = " << jetL2RelativeName_ << '\n';
    std::cout << "  jetL2ResidualName      = " << jetL2ResidualName_ << '\n';
    std::cout << "  jetL2L3ResidualName    = " << jetL2L3ResidualName_ << '\n';
    std::cout << "  JerResoName            = " << JerResoName_ << '\n';
    std::cout << "  JerSfName              = " << JerSfName_ << '\n';
}

void ScaleJetLoader::loadRefs() {
    loadJetL1FastJetRef();
    loadJetL2RelativeRef();
    if (isData_){
        if (level_ >= GlobalFlag::JecApplicationLevel::L2L3Res) {
            loadJetL2L3ResidualRef();
        } else if (level_ >= GlobalFlag::JecApplicationLevel::L2Res) {
            loadJetL2ResidualRef();
        }
    } else { // MC
        loadJerResoRef();
        loadJerSfRef();
    }
}

void ScaleJetLoader::loadJetL1FastJetRef() {
    std::cout << "==> loadJetL1FastJetRef()\n";
    try {
        loadedJetL1FastJetRef_ = correction::CorrectionSet::from_file(jercJsonPath_)->at(jetL1FastJetName_);
    } catch (const std::exception& e) {
        std::cerr << "\nEXCEPTION: ScaleJetLoader::loadJetL1FastJetRef\n";
        std::cerr << "Check " << jercJsonPath_ << " or " << jetL1FastJetName_ << '\n';
        std::cerr << e.what() << '\n';
        throw;
    }
}

void ScaleJetLoader::loadJetL2RelativeRef() {
    std::cout << "==> loadJetL2RelativeRef()\n";
    try {
        loadedJetL2RelativeRef_ = correction::CorrectionSet::from_file(jercJsonPath_)->at(jetL2RelativeName_);
    } catch (const std::exception& e) {
        std::cerr << "\nEXCEPTION: ScaleJetLoader::loadJetL2RelativeRef\n";
        std::cerr << "Check " << jercJsonPath_ << " or " << jetL2RelativeName_ << '\n';
        std::cerr << e.what() << '\n';
        throw;
    }
}

void ScaleJetLoader::loadJetL2ResidualRef() {
    std::cout << "==> loadJetL2ResidualRef()\n";
    try {
        loadedJetL2ResidualRef_ = correction::CorrectionSet::from_file(jercJsonPath_)->at(jetL2ResidualName_);
    } catch (const std::exception& e) {
        std::cerr << "\nEXCEPTION: ScaleJetLoader::loadJetL2ResidualRef\n";
        std::cerr << "Check " << jercJsonPath_ << " or " << jetL2ResidualName_ << '\n';
        std::cerr << e.what() << '\n';
        throw;
    }
}

void ScaleJetLoader::loadJetL2L3ResidualRef() {
    std::cout << "==> loadJetL2L3ResidualRef()\n";
    try {
        loadedJetL2L3ResidualRef_ = correction::CorrectionSet::from_file(jercJsonPath_)->at(jetL2L3ResidualName_);
    } catch (const std::exception& e) {
        std::cerr << "\nEXCEPTION: ScaleJetLoader::loadJetL2L3ResidualRef\n";
        std::cerr << "Check " << jercJsonPath_ << " or " << jetL2L3ResidualName_ << '\n';
        std::cerr << e.what() << '\n';
        throw;
    }
}

void ScaleJetLoader::loadJerResoRef() {
    std::cout << "==> loadJerResoRef()\n";
    try {
        loadedJerResoRef_ = correction::CorrectionSet::from_file(jercJsonPath_)->at(JerResoName_);
    } catch (const std::exception& e) {
        std::cout << "\nEXCEPTION: ScaleJetLoader::loadJerResoRef\n";
        std::cout << "Check " << jercJsonPath_ << " or " << JerResoName_ << '\n';
        std::cout << e.what() << '\n';
        throw;
    }
}

void ScaleJetLoader::loadJerSfRef() {
    std::cout << "==> loadJerSfRef()\n";
    try {
        loadedJerSfRef_ = correction::CorrectionSet::from_file(jercJsonPath_)->at(JerSfName_);
    } catch (const std::exception& e) {
        std::cout << "\nEXCEPTION: ScaleJetLoader::loadJerSfRef\n";
        std::cout << "Check " << jercJsonPath_ << " or " << JerSfName_ << '\n';
        std::cout << e.what() << '\n';
        throw;
    }
}

