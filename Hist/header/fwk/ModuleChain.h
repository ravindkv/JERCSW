#pragma once

#include <memory>
#include <vector>

#include "fwk/IModule.h"

namespace fwk {

class ModuleChain {
public:
    void add(std::unique_ptr<IModule> m);

    void beginJob(Context& ctx);
    void beginFile(Context& ctx);
    bool analyze(Context& ctx, Event& ev);
    void endJob(Context& ctx);

private:
    std::vector<std::unique_ptr<IModule>> modules_;
};

} // namespace fwk
