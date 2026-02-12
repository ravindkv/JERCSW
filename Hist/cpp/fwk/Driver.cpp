#include "fwk/Driver.h"

#include "fwk/Event.h"

namespace fwk {

int Driver::run(Context& ctx, ModuleChain& chain) {
    chain.beginJob(ctx);
    chain.beginFile(ctx);

    Event ev;
    ev.entry = 0;
    chain.analyze(ctx, ev);

    chain.endJob(ctx);

    if (ctx.out && ctx.out->file()) {
        ctx.out->file()->Write();
    }

    return 0;
}

} // namespace fwk
