#include "fwk/Factory.h"

#include <memory>
#include <stdexcept>

#include "RunL2ResidualDiJet.h"
#include "RunL2ResidualGamJet.h"
#include "RunL2ResidualZeeJet.h"
#include "RunL2ResidualZmmJet.h"
#include "RunL3ResidualGamJet.h"
#include "RunL3ResidualGamJetFake.h"
#include "RunL3ResidualMultiJet.h"
#include "RunL3ResidualWqqe.h"
#include "RunL3ResidualWqqm.h"
#include "RunL3ResidualZeeJet.h"
#include "RunL3ResidualZmmJet.h"
#include "fwk/RunWrapperModule.h"

namespace fwk {

ModuleChain makeChain(const GlobalFlag& gf) {
    ModuleChain chain;

    using DerivationLevel = GlobalFlag::JecDerivationLevel;
    using Chan = GlobalFlag::Channel;

    switch (gf.getJecDerivationLevel()) {
        case DerivationLevel::L2Residual:
        case DerivationLevel::JerSF: {
            switch (gf.getChannel()) {
                case Chan::DiJet:
                    chain.add(std::make_unique<RunWrapperModule<RunL2ResidualDiJet>>(gf, "RunL2ResidualDiJet"));
                    return chain;
                case Chan::ZeeJet:
                    chain.add(std::make_unique<RunWrapperModule<RunL2ResidualZeeJet>>(gf, "RunL2ResidualZeeJet"));
                    return chain;
                case Chan::ZmmJet:
                    chain.add(std::make_unique<RunWrapperModule<RunL2ResidualZmmJet>>(gf, "RunL2ResidualZmmJet"));
                    return chain;
                case Chan::GamJet:
                    chain.add(std::make_unique<RunWrapperModule<RunL2ResidualGamJet>>(gf, "RunL2ResidualGamJet"));
                    return chain;
                default:
                    throw std::runtime_error("Unsupported channel for L2Residual/JerSF: " + gf.getChannelStr());
            }
        }

        case DerivationLevel::L3Residual: {
            switch (gf.getChannel()) {
                case Chan::ZeeJet:
                    chain.add(std::make_unique<RunWrapperModule<RunL3ResidualZeeJet>>(gf, "RunL3ResidualZeeJet"));
                    return chain;
                case Chan::ZmmJet:
                    chain.add(std::make_unique<RunWrapperModule<RunL3ResidualZmmJet>>(gf, "RunL3ResidualZmmJet"));
                    return chain;
                case Chan::GamJet:
                    chain.add(std::make_unique<RunWrapperModule<RunL3ResidualGamJet>>(gf, "RunL3ResidualGamJet"));
                    return chain;
                case Chan::GamJetFake:
                    chain.add(std::make_unique<RunWrapperModule<RunL3ResidualGamJetFake>>(gf, "RunL3ResidualGamJetFake"));
                    return chain;
                case Chan::MultiJet:
                    chain.add(std::make_unique<RunWrapperModule<RunL3ResidualMultiJet>>(gf, "RunL3ResidualMultiJet"));
                    return chain;
                case Chan::Wqqe:
                    chain.add(std::make_unique<RunWrapperModule<RunL3ResidualWqqe>>(gf, "RunL3ResidualWqqe"));
                    return chain;
                case Chan::Wqqm:
                    chain.add(std::make_unique<RunWrapperModule<RunL3ResidualWqqm>>(gf, "RunL3ResidualWqqm"));
                    return chain;
                default:
                    throw std::runtime_error("Unsupported channel for L3Residual: " + gf.getChannelStr());
            }
        }

        default:
            throw std::runtime_error("Unsupported JEC step: " + gf.getJecDerivationLevelStr());
    }
}

} // namespace fwk
