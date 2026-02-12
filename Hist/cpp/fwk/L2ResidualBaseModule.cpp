#include "fwk/L2ResidualBaseModule.h"

#include <cassert>
#include <iostream>

#include <TDirectory.h>

#include "HemVeto.h"
#include "VarBin.h"
#include "fwk/Context.h"
#include "fwk/OutputService.h"
#include "fwk/PickEventModule.h"
#include "fwk/ScaleJetModule.h"
#include "fwk/ScaleMetModule.h"
#include "fwk/ScaleMuonModule.h"

namespace fwk {

L2ResidualBaseModule::L2ResidualBaseModule(const GlobalFlag& gf)
    : globalFlags_(gf) {
    loadChannelConfig(configPath());
}

L2ResidualBaseModule::~L2ResidualBaseModule() = default;

void L2ResidualBaseModule::beginJob(Context& ctx) {
    assert(ctx.out && ctx.out->file() && !ctx.out->file()->IsZombie());
    ctx.out->mkdirAndCd("Base");
    origDir_ = gDirectory;

    hCutflow_ = std::make_unique<HistCutflow>(origDir_, "", cutflows_, globalFlags_);
    varBin_ = std::make_unique<VarBin>(globalFlags_);
    histL2Residual_ = std::make_unique<HistL2Residual>(origDir_, "passL2Residual", *varBin_);

    bookChannelHistograms(origDir_);

    pickEventModule_ = std::make_unique<PickEventModule>(globalFlags_);
    scaleMuonModule_ = std::make_unique<ScaleMuonModule>(globalFlags_);
    scaleJetModule_ = std::make_unique<ScaleJetModule>(globalFlags_);
    scaleMetModule_ = std::make_unique<ScaleMetModule>(globalFlags_);
    hemVeto_ = std::make_shared<HemVeto>(globalFlags_);
}

bool L2ResidualBaseModule::analyze(Context& ctx, Event&) {
    auto& skimT = ctx.skimT;

    double weight = 1.0;
    hCutflow_->fill("passSkim", weight);

    if (!pickEventModule_->passCoreEventCuts(skimT, hCutflow_.get(), weight)) {
        return true;
    }

    scaleMuonModule_->applyCorrections(skimT);
    scaleJetModule_->applyCorrections(skimT);

    L2ResidualObjects objects;
    if (!pickObjects(ctx, objects, weight)) {
        return true;
    }

    scaleMetModule_->applyCorrections(skimT, scaleJetModule_->jetPtRaw());
    TLorentzVector p4CorrMet = scaleMetModule_->correctedMet();
    p4CorrMet += objects.p4RawTag - objects.p4Tag;

    HistL2ResidualInput input = computeResponse(objects, p4CorrMet);

    const bool passResp = input.resp > minResp_ && input.resp < maxResp_;
    if (!passResp) {
        return true;
    }
    hCutflow_->fill("passL2Residual", weight);

    if (hemVeto_->isHemVeto(*skimT)) {
        if (globalFlags_.isData()) {
            return true;
        }
        weight *= hemVeto_->getMcWeight();
    }

    applyChannelWeights(ctx, objects, weight);

    input.weight = weight;
    histL2Residual_->fillHistos(input);
    fillChannelSpecificHistos(ctx, objects, input, weight);

    return true;
}

void L2ResidualBaseModule::endJob(Context& ctx) {
    hCutflow_->printCutflow();
    hCutflow_->fillFractionCutflow();
    std::cout << moduleName() << " output file: " << ctx.out->file()->GetName() << '\n';
}

} // namespace fwk
