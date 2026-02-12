// cpp/SkimTree.cpp
#include "SkimTree.h"

// -------------------------------------------------------------
// Constructor / Destructor
// -------------------------------------------------------------
SkimTree::SkimTree(GlobalFlag& globalFlags)
    : SkimBranch()
    , skimReader_(globalFlags, *this)
{
}

SkimTree::~SkimTree() = default;

// -------------------------------------------------------------
// Tree operations (thin wrappers to SkimReader)
// -------------------------------------------------------------
Long64_t SkimTree::getEntries() const {
    return skimReader_.getEntries();
}

TChain* SkimTree::getChain() const {
    return skimReader_.getChain();
}

Int_t SkimTree::getEntry(Long64_t entry) {
    return skimReader_.getEntry(entry);
}

void SkimTree::loadTree(const std::vector<std::string>& skimFileList) {
    skimReader_.loadTree(skimFileList);
}

// -------------------------------------------------------------
// HLT accessors
// -------------------------------------------------------------
const std::string* SkimTree::getTrigNames() const {
    const auto& names = skimReader_.triggerNames();
    return names.empty() ? nullptr : names.data();
}

std::size_t SkimTree::getNumTrigNames() const {
    return skimReader_.triggerNames().size();
}

Bool_t SkimTree::getTrigValue(const std::string& trigName) const {
    return skimReader_.getTrigValue(trigName) ? Bool_t{1} : Bool_t{0};
}

