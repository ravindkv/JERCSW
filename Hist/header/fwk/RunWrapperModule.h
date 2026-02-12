#pragma once

#include <memory>
#include <stdexcept>
#include <string>

#include "fwk/IModule.h"

namespace fwk {

template <typename RunT>
class RunWrapperModule final : public IModule {
public:
    RunWrapperModule(const GlobalFlag& gf, std::string label)
        : gf_(gf), label_(std::move(label)), runner_(std::make_unique<RunT>(gf_)) {}

    std::string name() const override { return label_; }

    bool analyze(Context& ctx, Event&) override {
        if (hasRun_) {
            return false;
        }

        hasRun_ = true;
        ctx.out->mkdirAndCd("Base");

        const int code = runner_->Run(ctx.skimT, ctx.scaleEvent, ctx.out->file());
        if (code != 0) {
            throw std::runtime_error("Run module failed: " + label_);
        }
        return false;
    }

private:
    const GlobalFlag& gf_;
    std::string label_;
    std::unique_ptr<RunT> runner_;
    bool hasRun_ = false;
};

} // namespace fwk
