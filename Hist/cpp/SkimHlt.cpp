// cpp/SkimHlt.cpp
#include "SkimHlt.h"

#include <stdexcept>
#include <iostream>

SkimHlt::SkimHlt(Hlt& hlt, bool debug)
    : hlt_(hlt)
    , debug_(debug)
{}

SkimHlt::~SkimHlt() = default;

void SkimHlt::initialize(TChain* chain) {
    if (!chain) {
        throw std::runtime_error("SkimHlt::initialize - chain is null");
    }

    names_ = hlt_.getTrigNames();
    if (names_.size() > kMaxTrig) {
        throw std::runtime_error("SkimHlt - too many triggers");
    }

    nameToIndex_.clear();
    nameToIndex_.reserve(names_.size());

    for (std::size_t i = 0; i < names_.size(); ++i) {
        const auto& nm = names_[i];
        nameToIndex_.emplace(nm, i);

        if (debug_) {
            std::cout << "[" << i << "] " << nm << "\n";
        }

        chain->SetBranchStatus(nm.c_str(), true);
        chain->SetBranchAddress(nm.c_str(), &values_[i]);
    }

    // The rest of values_ remain zero-initialized for any unused slots.
}

bool SkimHlt::getValue(const std::string& name) const {
    auto it = nameToIndex_.find(name);
    if (it == nameToIndex_.end()) {
        if (debug_) {
            std::cerr << "SkimHlt::getValue - trigger not found: "
                      << name << std::endl;
        }
        return false;
    }
    return values_[it->second] != 0;
}

