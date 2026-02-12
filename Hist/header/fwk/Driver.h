#pragma once

#include "fwk/Context.h"
#include "fwk/ModuleChain.h"

namespace fwk {

class Driver {
public:
    static int run(Context& ctx, ModuleChain& chain);
};

} // namespace fwk
