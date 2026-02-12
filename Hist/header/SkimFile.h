#pragma once

#include <fstream>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

#include "GlobalFlag.h"

class SkimFile {
public:
    // Constructor accepting a reference to GlobalFlag
    explicit SkimFile(GlobalFlag& globalFlags, const std::string& ioName, const std::string& inJsonDir);
    ~SkimFile() noexcept;

    // Delete copy constructor and assignment operator since the class holds references.
    SkimFile(const SkimFile&) = delete;
    SkimFile& operator=(const SkimFile&) = delete;

    void loadInput();

    void setInputJsonPath(const std::string& inDir);
    void loadInputJson();

    [[nodiscard]] const std::string& getSampleKey() const { 
        return loadedSampKey_; 
    }

    [[nodiscard]] const std::vector<std::string>& getAllFileNames() const { 
        return loadedAllFileNames_; 
    }

    void loadJobFileNames();

    [[nodiscard]] const std::vector<std::string>& getJobFileNames() const { 
        return loadedJobFileNames_; 
    }

    [[nodiscard]] const double & getXsecOrLumiNano() const { 
        return nanoXssOrLumi_; 
    }

    [[nodiscard]] const long int & getEventsNano() const { 
        return nanoEvents_; 
    }

private:
    // Member variables
    std::string ioName_;
    std::string loadedSampKey_ = "JetAlgo_Channel_Year_DataOrMC_Name";
    int loadedNthJob_ = 1;
    int loadedTotJob_ = 100;
    std::string inputJsonPath_ = "./FilesSkim_2022_GamJet.json";
    std::vector<std::string> loadedAllFileNames_;
    std::vector<std::string> loadedJobFileNames_;

    // Reference to GlobalFlag instance and related constant members
    GlobalFlag& globalFlags_;

    double nanoXssOrLumi_ = 1.0;
    long int nanoEvents_ = 1;  // Changed from 1.0 to 1 for type consistency
};

