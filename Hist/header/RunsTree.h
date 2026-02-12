#pragma once

#include <TFile.h>
#include <TTree.h>
#include <TChain.h>
#include <fstream>
#include <string>
#include <vector>
#include <memory>

#include "GlobalFlag.h"

class RunsTree {
public:
    explicit RunsTree(GlobalFlag& globalFlags);
    ~RunsTree() = default;

    // Delete copy operations to prevent accidental copying.
    RunsTree(const RunsTree&) = delete;
    RunsTree& operator=(const RunsTree&) = delete;
    // Allow moves if necessary.
    RunsTree(RunsTree&&) = default;
    RunsTree& operator=(RunsTree&&) = default;

    [[nodiscard]] Long64_t getEntries() const;
    [[nodiscard]] TChain*  getChain() const;
    [[nodiscard]] Int_t    getEntry(Long64_t entry);
    [[nodiscard]] Long64_t loadEntry(Long64_t entry);

    void loadTree(const std::vector<std::string>& skimFileList);

    // Returns the computed normalized sum (genEventSumw/genEventCount) from tree entries.
    Double_t getNormGenEventSumw();

    // Caches the computed normalized sum in a JSON file under a given sample key.
    Double_t getCachedNormGenEventSumw(const std::string& sampleKey,
                                       const std::string& cacheFilename,
                                       const std::vector<std::string>& skimFileList);

    // ------------------------------------------------------------------
    // Static helper: pre-fill RunsTree.json for ALL MC samples (-r mode)
    // ------------------------------------------------------------------
    static int prefillRunsTreeCache(const std::vector<std::string>& jsonFiles,
                                    const std::string&              jsonDir,
                                    bool                            isDebug);

private:
    // Computes the sum of genEventCount and genEventSumw over all entries.
    void computeSum();

    Int_t fCurrent_{-1};  // Current tree number in the TChain.
    std::unique_ptr<TChain> fChain_;

    GlobalFlag&          globalFlags_;
    const GlobalFlag::Year    year_;
    const GlobalFlag::Era     era_;
    const GlobalFlag::Channel channel_;
    const bool isDebug_;
    const bool isData_;
    const bool isMC_;

    Long64_t genEventCount_{0};
    Double_t genEventSumw_{0.0};

    Long64_t sumGenEventCount_{0};
    Double_t sumGenEventSumw_{0.0};
};

