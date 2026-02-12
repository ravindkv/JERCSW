#pragma once

#include <memory>

#include "GlobalFlag.h"
#include "ScaleEvent.h"
#include "SkimTree.h"

namespace fwk {

class OutputService;
class CutflowService;
class TimerService;
class ConfigService;
class LoggerService;

struct Context {
    explicit Context(const GlobalFlag& gfIn)
        : gf(gfIn) {}

    ~Context();

    const GlobalFlag& gf;

    std::shared_ptr<SkimTree> skimT;
    ScaleEvent* scaleEvent = nullptr; // non-owning

    std::unique_ptr<OutputService> out;
    std::unique_ptr<CutflowService> cutflow;
    std::unique_ptr<TimerService> timer;
    std::unique_ptr<ConfigService> config;
    std::unique_ptr<LoggerService> log;
};

} // namespace fwk
