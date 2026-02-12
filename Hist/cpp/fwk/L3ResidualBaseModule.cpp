#include "fwk/L3ResidualBaseModule.h"

#include <cassert>
#include <cmath>
#include <iostream>

#include <TDirectory.h>
#include <TMath.h>

#include "HemVeto.h"
#include "Helper.hpp"
#include "HelperDelta.hpp"
#include "JecUncBand.h"
#include "VarBin.h"
#include "fwk/Context.h"
#include "fwk/OutputService.h"
#include "fwk/PickEventModule.h"
#include "fwk/ScaleJetModule.h"
#include "fwk/ScaleMetModule.h"
#include "fwk/ScaleMuonModule.h"

namespace fwk {

L3ResidualBaseModule::L3ResidualBaseModule(const GlobalFlag& gf)
    : globalFlags_(gf) {}

L3ResidualBaseModule::~L3ResidualBaseModule() = default;

void L3ResidualBaseModule::beginJob(Context& ctx) {
    loadChannelConfig(configPath());

    assert(ctx.out && ctx.out->file() && !ctx.out->file()->IsZombie());
    ctx.out->mkdirAndCd("Base");
    origDir_ = gDirectory;

    hCutflow_ = std::make_unique<HistCutflow>(origDir_, "", cutflows_, globalFlags_);

    varBin_ = std::make_unique<VarBin>(globalFlags_);
    histAlpha_ = std::make_unique<HistAlpha>(origDir_, "passDeltaPhiTagProbe", *varBin_, alphaCuts_);
    histL3Residual_ = std::make_unique<HistL3Residual>(origDir_, "passL3Residual", *varBin_);

    bookChannelHistograms(origDir_);

    pickEventModule_ = std::make_unique<PickEventModule>(globalFlags_);
    scaleMuonModule_ = std::make_unique<ScaleMuonModule>(globalFlags_);
    scaleJetModule_ = std::make_unique<ScaleJetModule>(globalFlags_);
    scaleMetModule_ = std::make_unique<ScaleMetModule>(globalFlags_);

    jecUncBand_ = std::make_shared<JecUncBand>(globalFlags_);
    hemVeto_ = std::make_shared<HemVeto>(globalFlags_);

    totalTime_ = 0.0;
    startClock_ = std::chrono::high_resolution_clock::now();
    everyN_ = Helper::initProgress(ctx.skimT->getEntries());
}

bool L3ResidualBaseModule::analyze(Context& ctx, Event& ev) {
    auto& skimT = ctx.skimT;
    Helper::printProgressEveryN(ev.entry, skimT->getEntries(), everyN_, startClock_, totalTime_);

    double weight = 1.0;
    hCutflow_->fill("passSkim", weight);

    if (!pickEventModule_->passCoreEventCuts(skimT, hCutflow_.get(), weight)) {
        return true;
    }

    scaleMuonModule_->applyCorrections(skimT);
    scaleJetModule_->applyCorrections(skimT);

    L3ResidualObjects objects;
    if (!pickObjects(ctx, objects, weight)) {
        return true;
    }

    const double deltaPhi = HelperDelta::DELTAPHI(objects.p4Tag.Phi(), objects.p4Probe.Phi());
    if (std::fabs(deltaPhi - TMath::Pi()) >= maxDeltaPhiTagProbe_) {
        return true;
    }
    hCutflow_->fill("passDeltaPhiTagProbe", weight);

    if (!pickEventModule_->passProbeJetVeto(objects.p4Probe, hCutflow_.get(), weight)) {
        return true;
    }

    const double ptTag = objects.p4Tag.Pt();
    const double ptJet2 = objects.p4Jet2.Pt();
    const double alpha = ptJet2 / ptTag;
    const bool passAlpha = (alpha < maxAlpha_ || ptJet2 < minPtJet2InAlpha_);

    scaleMetModule_->applyCorrections(skimT, scaleJetModule_->jetPtRaw());
    TLorentzVector p4CorrMet = scaleMetModule_->correctedMet();
    p4CorrMet += objects.p4RawTag - objects.p4Tag;

    HistL3ResidualInput input = computeResponse(objects, p4CorrMet);

    histAlpha_->Fill(alpha, skimT->Rho, input);
    if (!passAlpha) {
        return true;
    }
    hCutflow_->fill("passAlpha", weight);

    const bool passDbResp = input.respDb > minResp_ && input.respDb < maxResp_;
    const bool passMpfResp = input.respMpf > minResp_ && input.respMpf < maxResp_;
    if (!(passDbResp && passMpfResp)) {
        return true;
    }
    hCutflow_->fill("passL3Residual", weight);

    if (hemVeto_->isHemVeto(*skimT)) {
        if (globalFlags_.isData()) {
            return true;
        }
        weight *= hemVeto_->getMcWeight();
    }

    applyChannelWeights(ctx, objects, weight);

    input.weight = weight;
    histL3Residual_->fillHistos(input);
    fillChannelSpecificHistos(ctx, objects, input, alpha, weight);

    return true;
}

void L3ResidualBaseModule::endJob(Context& ctx) {
    hCutflow_->printCutflow();
    hCutflow_->fillFractionCutflow();
    std::cout << moduleName() << " output file: " << ctx.out->file()->GetName() << '\n';
}

} // namespace fwk
