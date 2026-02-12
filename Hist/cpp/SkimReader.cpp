// cpp/SkimReader.cpp
#include "SkimReader.h"

#include <iostream>
#include <stdexcept>

// -------------------------------------------------------------
// Constructor / Destructor
// -------------------------------------------------------------
SkimReader::SkimReader(GlobalFlag& flags, SkimBranch& branches)
    : globalFlags_(flags)
    , skimBranch_(branches)
    , chain_(std::make_unique<TChain>("Events"))
    , year_(flags.getYear())
    , era_(flags.getEra())
    , channel_(flags.getChannel())
    , jetAlgo_(flags.getJetAlgo())
    , nanoVer_(flags.getNanoVersion())
    , isDebug_(flags.isDebug())
    , isData_(flags.isData())
    , isMC_(flags.isMC())
    , currentTree_(-1)
    , skimAdapter_(globalFlags_, skimBranch_)
    , hlt_(flags)
    , skimCache_(hlt_, isDebug_)
{
}

SkimReader::~SkimReader() = default;

// -------------------------------------------------------------
// File helpers
// -------------------------------------------------------------
TFile* SkimReader::validateAndOpenFile_(const std::string& fullPath) {
    TFile* file = TFile::Open(fullPath.c_str(), "READ");
    if (!file || file->IsZombie()) {
        std::cerr << "Error: Failed to open or corrupted file " << fullPath << '\n';
        if (file) file->Close();
        return nullptr;
    }

    // Check file size (using 3000 bytes as a threshold)
    if (file->GetSize() < 3000) {
        std::cerr << "Warning: file " << fullPath
                  << " has less than 3000 bytes, skipping.\n";
        file->Close();
        return nullptr;
    }

    TTree* tree = dynamic_cast<TTree*>(file->Get("Events"));
    if (!tree) {
        std::cerr << "Error: 'Events' not found in " << fullPath << '\n';
        file->Close();
        return nullptr;
    }

    if (tree->GetEntries() == 0) {
        std::cerr << "Warning: 'Events' TTree in file " << fullPath
                  << " has 0 entries. Skipping file.\n";
        file->Close();
        return nullptr;
    }

    return file;
}

bool SkimReader::addFileToChain_(const std::string& fullPath) {
    int added = chain_->Add(fullPath.c_str());
    if (added == 0) {
        std::cerr << "Warning: TChain::Add failed for " << fullPath << '\n';
        return false;
    }
    return true;
}

// Build TChain from file list; returns false if no valid trees
bool SkimReader::buildChain_(const std::vector<std::string>& files) {
    std::cout << "==> loadTree()" << '\n';

    if (!chain_) {
        chain_ = std::make_unique<TChain>("Events");
    }
    chain_->SetCacheSize(100 * 1024 * 1024);

    if (files.empty()) {
        throw std::runtime_error("SkimReader::buildChain_ - no files to load");
    }

    int totalFiles  = 0;
    int addedFiles  = 0;
    int failedFiles = 0;

    for (const auto& fName : files) {
        totalFiles++;
        const std::string& fullPath = fName;
        std::cout << fullPath << '\n';

        TFile* file = validateAndOpenFile_(fullPath);
        if (!file) {
            failedFiles++;
            continue;
        }
        if (!addFileToChain_(fullPath)) {
            failedFiles++;
            file->Close();
            continue;
        }

        std::cout << "Total Entries: " << chain_->GetEntries() << '\n';
        addedFiles++;
        file->Close();
    }

    std::cout << "==> Finished loading files.\n";
    std::cout << "Total files processed: "   << totalFiles  << '\n';
    std::cout << "Successfully added files: " << addedFiles << '\n';
    std::cout << "Failed to add files: "      << failedFiles << '\n';

    if (chain_->GetNtrees() == 0) {
        std::cerr << "Error: No valid ROOT files were added to the TChain. Exiting.\n";
        return false;
    }
    return true;
}

// -------------------------------------------------------------
// Public loadTree
// -------------------------------------------------------------
void SkimReader::loadTree(const std::vector<std::string>& files) {
    if (!buildChain_(files)) {
        throw std::runtime_error("SkimReader::loadTree - failed to build chain");
    }
    setupBranches_();
}

// -------------------------------------------------------------
// Branch setup
// -------------------------------------------------------------
void SkimReader::setupBranches_() {

    skimAdapter_.setupCommonBranches(chain_.get());
    skimAdapter_.setupJetBranches(chain_.get());
    skimAdapter_.setupLeptonBranches(chain_.get());
    skimAdapter_.setupMCBranches(chain_.get());
    skimAdapter_.setupMetBranches(chain_.get());
    if (isDebug_) skimAdapter_.printDebug();

    // HLT
    skimCache_.initialize(chain_.get());

    // -----------------------------
    // TTree cache tuning
    // -----------------------------
    if (chain_) {
        // let ROOT learn which branches are used over first 1000 entries
        chain_->SetCacheLearnEntries(1000);

        // cache all active branches ("*" respects SetBranchStatus)
        chain_->AddBranchToCache("*", /*subbranches*/ true);
    }
}


// -------------------------------------------------------------
// Basic tree operations
// -------------------------------------------------------------
Long64_t SkimReader::getEntries() const {
    return chain_ ? chain_->GetEntries() : 0;
}

Int_t SkimReader::getEntry(Long64_t entry) {
    Int_t ret = chain_ ? chain_->GetEntry(entry) : 0;

    // Convert v15 types to canonical representation
    skimAdapter_.afterGetEntry();

    if (chain_->GetTreeNumber() != currentTree_) {
        currentTree_ = chain_->GetTreeNumber();
        const TFile* cur = chain_->GetCurrentFile();
        const char* name = cur ? cur->GetName() : "<unknown>";
        std::cout << "[Events] Switched to file #" << currentTree_
                  << ": " << name << '\n';
    }


    return ret;
}

// -------------------------------------------------------------
// HLT façade
// -------------------------------------------------------------
bool SkimReader::getTrigValue(const std::string& name) const {
    return skimCache_.getValue(name);
}

const std::vector<std::string>& SkimReader::triggerNames() const {
    return skimCache_.names();
}

