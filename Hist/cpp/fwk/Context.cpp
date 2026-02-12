#include "fwk/Context.h"

#include "fwk/ConfigService.h"
#include "fwk/CutflowService.h"
#include "fwk/LoggerService.h"
#include "fwk/OutputService.h"
#include "fwk/TimerService.h"

namespace fwk {
// Context is a plain data holder; implementation unit kept for framework symmetry.

Context::Context(const GlobalFlag& gfIn)
    : gf(gfIn) {}

Context::~Context() = default;
} // namespace fwk
