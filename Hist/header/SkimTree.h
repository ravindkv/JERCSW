// header/SkimTree.h
#pragma once

#include "SkimReader.h"
#include "GlobalFlag.h"

#include "SkimBranch.hpp"
#include <TChain.h>
#include <string>
#include <vector>

class SkimTree : public SkimBranch {
public:
    // Constructor accepting a reference to GlobalFlag
    explicit SkimTree(GlobalFlag& globalFlags);
    ~SkimTree();

    // Tree operations 
    Long64_t getEntries() const;
    TChain*  getChain() const;  // Getter function to access TChain
    Int_t    getEntry(Long64_t entry);

    void loadTree(const std::vector<std::string>& skimFileList);

    // HLT 
    const std::string* getTrigNames() const;
    std::size_t        getNumTrigNames() const;
    Bool_t             getTrigValue(const std::string& trigName) const;

    // Disable copying and assignment 
    SkimTree(const SkimTree&)            = delete;
    SkimTree& operator=(const SkimTree&) = delete;

private:
    SkimReader skimReader_;
};

