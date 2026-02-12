#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include "SkimFile.h"
#include "HelperSplit.hpp"

SkimFile::SkimFile(GlobalFlag& globalFlags, const std::string& ioName, const std::string& inJsonDir)
    : ioName_(ioName),
      globalFlags_(globalFlags)
{
    std::cout << "+ SkimFile initialized with ioName = " << ioName_ << '\n';
    loadInput();
    setInputJsonPath(inJsonDir);
    loadInputJson();
    loadJobFileNames();
}

SkimFile::~SkimFile() noexcept {
    // No manual resource cleanup required.
}

void SkimFile::loadInput() {
    std::cout << "==> loadInput()\n";
    try {
        // 1) split off everything before “_Hist”
        auto tokens = HelperSplit::splitString(ioName_, "_Hist");
        if (tokens.size() < 2) {
            throw std::runtime_error(
                "Invalid ioName format: no “_Hist…” part. Expected “_Hist<SYST>_NofM.root”");
        }
        loadedSampKey_ = tokens[0];
        // tokens[1] == e.g. "Base_1of100.root" or "IsrUp_1of100.root"
        std::string histPart = tokens[1];

        // 2) separate systematic name from job tag:
        //    find first '_' in histPart
        auto pos = histPart.find('_');
        if (pos == std::string::npos) {
            throw std::runtime_error(
                "Invalid Hist part: missing '_' between systematic and job tag: " + histPart);
        }
        // systematic is the piece before the underscore
        std::string loadedSystematic = histPart.substr(0, pos);

        // 3) strip off the ".root" extension from the remainder
        std::string jobPart = histPart.substr(pos + 1);  // e.g. "1of100.root"
        const std::string ext = ".root";
        if (jobPart.size() <= ext.size() ||
            jobPart.substr(jobPart.size() - ext.size()) != ext)
        {
            throw std::runtime_error(
                "Invalid job tag: missing '.root' extension: " + jobPart);
        }
        jobPart.erase(jobPart.size() - ext.size());  // now "1of100"

        // 4) split on "of" to get N and M
        auto jobTokens = HelperSplit::splitString(jobPart, "of");
        if (jobTokens.size() != 2) {
            throw std::runtime_error(
                "Invalid job numbering: expected format 'NofM', got: " + jobPart);
        }
        loadedNthJob_ = std::stoi(jobTokens[0]);
        loadedTotJob_ = std::stoi(jobTokens[1]);

        if (globalFlags_.isDebug()) {
            std::cout << "  sample key      = " << loadedSampKey_      << "\n"
                      << "  systematic      = " << loadedSystematic  << "\n"
                      << "  job N of M      = " << loadedNthJob_
                      << " of " << loadedTotJob_ << "\n";
        }
    }
    catch (const std::exception& e) {
        std::ostringstream oss;
        oss << "Error in loadInput(): " << e.what() << "\n"
            << "Check the ioName_: " << ioName_ << "\n"
            << "Expected format: JetAlgo_Channel_Year_DataOrMC_Sample_Hist<SYST>_NofM.root\n"
               "   e.g. AK4Puppi_ZeeJet_2018A_Data_EGamma_HistBase_1of100.root\n"
               "Run ./runMain -h for more details";
        throw std::runtime_error(oss.str());
    }
}


void SkimFile::setInputJsonPath(const std::string& inDir) {
    inputJsonPath_ = inDir + "/FilesSkim_"  + globalFlags_.getJetAlgoStr() 
                                            + "_" + globalFlags_.getJecDerivationLevelStr() 
                                            + "_" + globalFlags_.getChannelStr() 
                                            + "_" + globalFlags_.getYearStr() 
                                            + ".json";
    std::cout << "+ setInputJsonPath() = " << inputJsonPath_ << '\n';
}

void SkimFile::loadInputJson() {
    std::cout << "==> loadInputJson()" << '\n';
    std::ifstream inputFile(inputJsonPath_);
    if (!inputFile.is_open()) {
        throw std::runtime_error("Unable to open input JSON file: " + inputJsonPath_);
    }

    nlohmann::json js;
    try {
        inputFile >> js;
    } catch (const std::exception& e) {
        std::ostringstream oss;
        oss << "Error parsing input JSON file: " << inputJsonPath_ << "\n"
            << e.what();
        throw std::runtime_error(oss.str());
    }

    try {
        // Parse JSON content and update member variables
        js.at(loadedSampKey_).at(1).get_to(loadedAllFileNames_);
        js.at(loadedSampKey_).at(0).at("xsecOrLumi").get_to(nanoXssOrLumi_);
        js.at(loadedSampKey_).at(0).at("nEvents").get_to(nanoEvents_);
        std::cout << "nanoXssOrLumi = " << nanoXssOrLumi_ 
                  << ", nanoEvents = " << nanoEvents_ << '\n';
    } catch (const std::exception& e) {
        std::ostringstream oss;
        oss << "Key not found in JSON: " << loadedSampKey_ << "\n"
            << e.what() << "\n"
            << "Available keys in the JSON file:";
        for (const auto& element : js.items()) {
            oss << "\n- " << element.key();
        }
        throw std::runtime_error(oss.str());
    }
}

void SkimFile::loadJobFileNames() {
    std::cout << "==> loadJobFileNames()" << '\n';
    const int nFiles = static_cast<int>(loadedAllFileNames_.size());
    std::cout << "Total files = " << nFiles << '\n';

    if (loadedTotJob_ > nFiles) {
        std::cout << "Since loadedTotJob_ > nFiles, setting loadedTotJob_ to nFiles: " << nFiles << '\n';
        loadedTotJob_ = nFiles;
    }

    if (loadedNthJob_ > loadedTotJob_) {
        throw std::runtime_error("Error: loadedNthJob_ > loadedTotJob_ in loadJobFileNames()");
    }

    if (loadedNthJob_ > 0 && loadedTotJob_ > 0) {
        std::cout << "Jobs: " << loadedNthJob_ << " of " << loadedTotJob_ << '\n';
        std::cout << static_cast<double>(nFiles) / loadedTotJob_ 
                  << " files per job on average" << '\n';
    } else {
        throw std::runtime_error("Error: Ensure loadedNthJob_ and loadedTotJob_ are greater than 0 in loadJobFileNames()");
    }

    const std::vector<std::vector<std::string>> smallVectors = HelperSplit::splitVector(loadedAllFileNames_, loadedTotJob_);
    if (loadedNthJob_ - 1 >= static_cast<int>(smallVectors.size())) {
        throw std::runtime_error("Error: loadedNthJob_ is out of range after splitting file names in loadJobFileNames()");
    }
    loadedJobFileNames_ = smallVectors[loadedNthJob_ - 1];
}

