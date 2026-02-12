#pragma once

#include <string>

#include "fwk/Context.h"
#include "fwk/Event.h"

namespace fwk {

class IModule {
public:
    virtual ~IModule() = default;

    virtual std::string name() const = 0;

    virtual void beginJob(Context&) {}
    virtual void beginFile(Context&) {}
    virtual bool analyze(Context&, Event&) = 0;
    virtual void endJob(Context&) {}
};

} // namespace fwk
