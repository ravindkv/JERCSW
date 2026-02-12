#pragma once

#include <string>

#include "TDirectory.h"
#include "TFile.h"

namespace fwk {

class OutputService {
public:
    explicit OutputService(TFile* fout);

    TFile* file() const;
    TDirectory* mkdirAndCd(const std::string& dir);

private:
    TFile* fout_; // non-owning
};

} // namespace fwk
