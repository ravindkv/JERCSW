// header/SkimHlt.h
#pragma once

#include "Hlt.h"

#include <TChain.h>
#include <string>
#include <vector>
#include <unordered_map>

class SkimHlt {
public:
    SkimHlt(Hlt& hlt, bool debug = false);
    ~SkimHlt();

    // Called after TChain is ready and branches are set up
    void initialize(TChain* chain);

    // Access
    bool getValue(const std::string& name) const;

    const std::vector<std::string>& names() const { return names_; }

private:
    Hlt&  hlt_;
    bool  debug_;

    static constexpr int kMaxTrig = 200;

    std::vector<std::string> names_;
    Bool_t                   values_[kMaxTrig]{};
    std::unordered_map<std::string, std::size_t> nameToIndex_;
};

