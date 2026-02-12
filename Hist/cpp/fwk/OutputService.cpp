#include "fwk/OutputService.h"

namespace fwk {

OutputService::OutputService(TFile* fout)
    : fout_(fout) {}

TFile* OutputService::file() const {
    return fout_;
}

TDirectory* OutputService::mkdirAndCd(const std::string& dir) {
    fout_->cd();
    auto* d = fout_->GetDirectory(dir.c_str());
    if (!d) {
        d = fout_->mkdir(dir.c_str());
    }
    d->cd();
    return d;
}

} // namespace fwk
