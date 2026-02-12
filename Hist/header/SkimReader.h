// header/SkimReader.h
#pragma once

#include "GlobalFlag.h"
#include "SkimBranch.hpp"
#include "SkimAdapter.h"
#include "SkimHlt.h"
#include "Hlt.h"

#include <TChain.h>
#include <TFile.h>

#include <memory>
#include <string>
#include <vector>

class SkimReader {
public:
    SkimReader(GlobalFlag& flags, SkimBranch& branches);
    ~SkimReader();

    void loadTree(const std::vector<std::string>& files);

    Long64_t getEntries() const;
    Int_t    getEntry(Long64_t entry);

    // HLT façade
    bool getTrigValue(const std::string& name) const;
    const std::vector<std::string>& triggerNames() const;

    TChain* getChain() const { return chain_.get(); }

private:
    GlobalFlag&   globalFlags_;
    SkimBranch& skimBranch_;

    std::unique_ptr<TChain> chain_;
    Int_t                   currentTree_{-1};

    const GlobalFlag::Year        year_;
    const GlobalFlag::Era         era_;
    const GlobalFlag::Channel     channel_;
    const GlobalFlag::JetAlgo     jetAlgo_;
    const GlobalFlag::NanoVersion nanoVer_;
    const bool                    isDebug_;
    const bool                    isData_;
    const bool                    isMC_;

    SkimAdapter  skimAdapter_;
    Hlt          hlt_;
    SkimHlt skimCache_;

    // helpers
    bool   buildChain_(const std::vector<std::string>& files);
    void   setupBranches_();

    TFile* validateAndOpenFile_(const std::string& fullPath);
    bool   addFileToChain_(const std::string& fullPath);
};

