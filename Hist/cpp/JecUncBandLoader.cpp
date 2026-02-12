#include "JecUncBandLoader.h"
#include <iostream>
#include <stdexcept>
#include "ReadConfig.h"

JecUncBandLoader::JecUncBandLoader(const GlobalFlag& globalFlags)
    : globalFlags_(globalFlags),
      year_(globalFlags_.getYear()),
      era_(globalFlags_.getEra()),
      channel_(globalFlags_.getChannel()),
      isDebug_(globalFlags_.isDebug()),
      isData_(globalFlags_.isData()),
      isMC_(globalFlags_.isMC())
{
    std::string fName = "config/JecUncBandLoader.json";
    if (isMC_){
        loadConfig(fName);
        loadJesUncBandRef();
        loadJerSfUncBandRef();
    }
}

void JecUncBandLoader::loadConfig(const std::string& filename) {
    ReadConfig config(filename);
    //std::string yearStr = globalFlags_.getYearStr();
    std::string yearStr = "2018";
    //json
    jesUncBandJsonPath_  = config.getValue<std::string>({yearStr, "jesUncBandJsonPath"});
    jerSfUncBandJsonPath_  = config.getValue<std::string>({yearStr, "jerSfUncBandJsonPath"});
    //tag name
    jesUncBandName_     = config.getValue<std::string>({yearStr, "CMS_scale_j_Total"});
    jerSfUncBandName_     = config.getValue<std::string>({yearStr, "CMS_res_j_SF"});
    
    std::cout << "\n[JecUncBandLoader] " << filename << '\n';
    std::cout << "  jesUncBandJsonPath    = " << jesUncBandJsonPath_ << '\n';
    std::cout << "  jerSfUncBandJsonPath  = " << jerSfUncBandJsonPath_ << '\n';
    std::cout << "  jesUncBandName        = " << jesUncBandName_ << '\n';
    std::cout << "  jerSfUncBandName      = " << jerSfUncBandName_ << '\n';
}


void JecUncBandLoader::loadJesUncBandRef() {
    std::cout << "==> loadJesUncBandRef()\n";
    try {
        loadedJesUncBandRef_ = correction::CorrectionSet::from_file(jesUncBandJsonPath_)->at(jesUncBandName_);
    } catch (const std::exception& e) {
        std::cout << "\nEXCEPTION: JecUncBandLoader::loadJesUncBandRef\n";
        std::cout << "Check " << jesUncBandJsonPath_ << " or " << jesUncBandName_ << '\n';
        std::cout << e.what() << '\n';
        throw;
    }
}


void JecUncBandLoader::loadJerSfUncBandRef() {
    std::cout << "==> loadJerSfUncBandRef()\n";
    try {
        loadedJerSfUncBandRef_ = correction::CorrectionSet::from_file(jerSfUncBandJsonPath_)->at(jerSfUncBandName_);
    } catch (const std::exception& e) {
        std::cout << "\nEXCEPTION: JecUncBandLoader::loadJerSfUncBandRef\n";
        std::cout << "Check " << jerSfUncBandJsonPath_ << " or " << jerSfUncBandName_ << '\n';
        std::cout << e.what() << '\n';
        throw;
    }
}

