#include "fwk/Driver.h"

#include "fwk/Event.h"
#include "fwk/OutputService.h"

namespace fwk {

int Driver::run(Context& ctx, ModuleChain& chain) {
    chain.beginJob(ctx);
    chain.beginFile(ctx);

    const long long nentries = ctx.skimT->getEntries();
    Event ev;
    for (long long jentry = 0; jentry < nentries; ++jentry) {
        if (ctx.gf.isDebug() && jentry > ctx.gf.getNDebug()) {
            break;
        }

        ctx.skimT->getEntry(jentry);
        ev.entry = jentry;
        ev.run = ctx.skimT->run;
        ev.lumi = ctx.skimT->luminosityBlock;
        ev.event = ctx.skimT->event;

        if (!chain.analyze(ctx, ev)) {
            break;
        }
    }

    chain.endJob(ctx);

    if (ctx.out && ctx.out->file()) {
        ctx.out->file()->Write();
    }

    return 0;
}

} // namespace fwk
