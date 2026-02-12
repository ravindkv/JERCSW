#include "fwk/ModuleChain.h"

namespace fwk {

void ModuleChain::add(std::unique_ptr<IModule> m) {
    modules_.push_back(std::move(m));
}

void ModuleChain::beginJob(Context& ctx) {
    for (auto& m : modules_) {
        m->beginJob(ctx);
    }
}

void ModuleChain::beginFile(Context& ctx) {
    for (auto& m : modules_) {
        m->beginFile(ctx);
    }
}

bool ModuleChain::analyze(Context& ctx, Event& ev) {
    for (auto& m : modules_) {
        if (!m->analyze(ctx, ev)) {
            return false;
        }
    }
    return true;
}

void ModuleChain::endJob(Context& ctx) {
    for (auto& m : modules_) {
        m->endJob(ctx);
    }
}

} // namespace fwk
