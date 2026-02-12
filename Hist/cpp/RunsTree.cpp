#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <cmath>  // For std::fabs
#include "RunsTree.h"
#include "nlohmann/json.hpp"  // Make sure the json header is available
#include <filesystem>
#include "SkimFile.h"
namespace fs = std::filesystem;

using json = nlohmann::json;

namespace {
    constexpr size_t kCacheSize = 100 * 1024 * 1024; // 100 MB cache size
}

RunsTree::RunsTree(GlobalFlag& globalFlags)
    : fCurrent_(-1),
      fChain_(std::make_unique<TChain>("Runs")),
      globalFlags_(globalFlags),
      year_(globalFlags_.getYear()),
      era_(globalFlags_.getEra()),
      channel_(globalFlags_.getChannel()),
      isDebug_(globalFlags_.isDebug()),
      isData_(globalFlags_.isData()),
      isMC_(globalFlags_.isMC()) { }

void RunsTree::loadTree(const std::vector<std::string>& skimFileList) {
    std::cout << "==> loadTree()" << '\n';

    // (Re)initialize fChain_ if necessary.
    if (!fChain_) {
        fChain_ = std::make_unique<TChain>("Runs");
    }
    fChain_->SetCacheSize(kCacheSize);

    if (skimFileList.empty()) {
        throw std::runtime_error("Error: No files provided in loadTree()");
    }

    const std::string dir = ""; // Adjust directory path as needed.
    for (const auto& fileName : skimFileList) {
        std::string fullPath = dir + fileName;
        if (fChain_->Add(fullPath.c_str()) == 0) {
            throw std::runtime_error("Error adding file to TChain: " + fullPath);
        }
        std::cout << fullPath << "  " << fChain_->GetEntries() << '\n';
    }

    // Disable all branches and enable only the ones needed.
    fChain_->SetBranchStatus("*", false);
    fChain_->SetBranchStatus("genEventCount", true);
    fChain_->SetBranchStatus("genEventSumw", true);
    fChain_->SetBranchAddress("genEventCount", &genEventCount_);
    fChain_->SetBranchAddress("genEventSumw", &genEventSumw_);
}

void RunsTree::computeSum() {
    sumGenEventCount_ = 0;
    sumGenEventSumw_  = 0.0;
    const Long64_t nentries = getEntries();

    for (Long64_t jentry = 0; jentry < nentries; ++jentry) {
        Long64_t ientry = loadEntry(jentry);
        if (ientry < 0)
            break;
        
        // Retrieve the entry from the current tree.
        TTree* tree = getChain()->GetTree();
        if (!tree) {
            throw std::runtime_error("Error: Tree pointer is null in computeSum()");
        }
        tree->GetEntry(ientry);
        sumGenEventCount_ += genEventCount_;
        sumGenEventSumw_  += genEventSumw_;
        if(isDebug_) std::cout<<"sumGenEventCount_ = "<<sumGenEventCount_<<", sumGenEventSumw_ = "<<sumGenEventSumw_<<'\n';
    }
}

Double_t RunsTree::getNormGenEventSumw() {
    computeSum();
    if (sumGenEventCount_ == 0) {
        throw std::runtime_error("Error: Total event count is zero. Cannot compute normalization.");
    }
    return sumGenEventSumw_ / sumGenEventCount_;
}

Double_t RunsTree::getCachedNormGenEventSumw(const std::string& sampleKey,
                                             const std::string& cacheFilename,
                                             const std::vector<std::string>& skimFileList) {
    json cache;

    std::ifstream inFile(cacheFilename);
    if (inFile) {
        try {
            inFile >> cache;
        } catch (const std::exception& e) {
            std::cerr << "Error reading JSON cache '" << cacheFilename
                      << "': " << e.what() << "\n";
            // Treat as empty cache; -r will recompute and overwrite.
            cache = json::object();
        }
    }

    if (cache.contains(sampleKey)) {
        if (isDebug_) {
            std::cout << "Cache hit for sample: " << sampleKey << "\n"
                      << "genEventSumw = " << cache[sampleKey] << std::endl;
        }
        return cache[sampleKey].get<double>();
    } else {
        if (isDebug_) {
            std::cout << "Cache miss for sample: " << sampleKey << "\n";
        }
        loadTree(skimFileList);
        Double_t computedSum = getNormGenEventSumw();

        // Validate the computed sum.
        if (getEntries() == 0 || std::fabs(computedSum) < 1e-10) {
            throw std::runtime_error("Error: Computed genEventSumw is invalid. Check your input ROOT files or TTree.");
        }

            cache[sampleKey] = computedSum;

    try {
        const std::string tmpName = cacheFilename + ".tmp";

        {
            // 1) Write to temporary file
            std::ofstream out(tmpName, std::ios::trunc);
            if (!out) {
                throw std::runtime_error("Error opening tmp cache file for writing: " + tmpName);
            }

            out << cache.dump(4);   // Pretty-print with 4 spaces indent
            out.flush();

            if (!out) {
                throw std::runtime_error("Error writing tmp cache file: " + tmpName);
            }
        }

        // 2) Atomically replace the original file
        fs::rename(tmpName, cacheFilename);
    }
    catch (const std::exception& e) {
        std::cerr << "Error updating cache file '" << cacheFilename << "': "
                  << e.what() << "\n";
        // Optional: you could also remove the tmp file here if it exists.
    }

        if (isDebug_) {
            std::cout << "genEventSumw = " << computedSum << std::endl;
        }
        return computedSum;
    }
}
// ----------------------------------------------------------------------
// Static helper: pre-fill RunsTree.json for ALL MC samples (-r mode)
// ----------------------------------------------------------------------
int RunsTree::prefillRunsTreeCache(const std::vector<std::string>& jsonFiles,
                                   const std::string&              jsonDir,
                                   bool                            isDebug)
{
    const std::string cacheFilePath = "config/RunsTree.json";

    std::cout << ">>> Pre-filling MC normalization cache: " << cacheFilePath << "\n";

    for (const auto& jsonFile : jsonFiles) {
        std::cout << "\n=== Processing file: " << jsonFile << " ===\n";

        std::ifstream file(jsonFile);
        if (!file.is_open()) {
            std::cerr << "  Could not open file: " << jsonFile << std::endl;
            continue;
        }

        json js;
        try {
            js = json::parse(file);
        } catch (const std::exception& e) {
            std::cerr << "  EXCEPTION: Error parsing JSON file: " << jsonFile << "\n"
                      << "  " << e.what() << std::endl;
            continue;
        }

        // Each key is something like: "AK4Chs_ZeeJet_2018_MC_DYJetsToLLM50"
        for (auto& element : js.items()) {
            const std::string sampleIoBase = element.key();

            // Skip data samples, only do MC
            if (sampleIoBase.find("_MC_") == std::string::npos) {
                continue;
            }

            // Build the same ioName as printed by -h:
            //   <sampleIoBase>_1of100.root
            std::string ioName = sampleIoBase + "_1of100.root";

            std::cout << "  -> Sample: " << sampleIoBase << std::endl;

            try {
                GlobalFlag globalFlag(ioName);
                globalFlag.setDebug(isDebug);

                if (!globalFlag.isMC()) {
                    std::cout << "     Skipping (GlobalFlag says not MC)\n";
                    continue;
                }

                // SkimFile knows how to map ioName to skim file list
                std::shared_ptr<SkimFile> skimF =
                    std::make_shared<SkimFile>(globalFlag, ioName, jsonDir);

                const std::string sampleKey = skimF->getSampleKey();
                std::cout << "     sampleKey = " << sampleKey << std::endl;

                std::shared_ptr<RunsTree> runsT =
                    std::make_shared<RunsTree>(globalFlag);

                // This will:
                //  - if sampleKey exists in cacheFilePath → read and return
                //  - else → compute from skimF->getAllFileNames() and append
                Double_t normGenEventSumw =
                    runsT->getCachedNormGenEventSumw(sampleKey,
                                                     cacheFilePath,
                                                     skimF->getAllFileNames());

                std::cout << "     normGenEventSumw = " << normGenEventSumw << "\n";

            } catch (const std::exception& e) {
                std::cerr << "     ERROR for " << sampleIoBase << ": " << e.what() << "\n";
            }
        }
    }

    std::cout << "\n>>> Done pre-filling " << cacheFilePath << "\n";
    return 0;
}

Long64_t RunsTree::getEntries() const {
    return fChain_ ? fChain_->GetEntries() : 0;
}

TChain* RunsTree::getChain() const {
    return fChain_.get();  // Return the raw pointer for use with ROOT APIs.
}

Int_t RunsTree::getEntry(Long64_t entry) {
    return fChain_ ? fChain_->GetEntry(entry) : 0;
}

Long64_t RunsTree::loadEntry(Long64_t entry) {
    if (!fChain_) {
        throw std::runtime_error("Error: fChain_ is not initialized in loadEntry()");
    }
    Long64_t centry = fChain_->LoadTree(entry);
    if (centry < 0) {
        throw std::runtime_error("Error loading entry in loadEntry()");
    }
    // Update tree number if it has changed.
    if (fChain_->GetTreeNumber() != fCurrent_) {
        fCurrent_ = fChain_->GetTreeNumber();
    }
    return centry;
}

