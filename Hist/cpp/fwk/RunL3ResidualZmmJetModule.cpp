#include "fwk/RunL3ResidualZmmJetModule.h"

#include <cassert>
#include <cmath>
#include <iostream>

#include <TDirectory.h>
#include <TMath.h>

#include "HemVeto.h"
#include "Helper.hpp"
#include "HelperDelta.hpp"
#include "JecUncBand.h"
#include "PickGenJet.h"
#include "PickZmmJet.h"
#include "ReadConfig.h"
#include "VarBin.h"
#include "fwk/Context.h"
#include "fwk/Event.h"
#include "fwk/OutputService.h"
#include "fwk/PickEventModule.h"
#include "fwk/ScaleJetModule.h"
#include "fwk/ScaleMetModule.h"
#include "fwk/ScaleMuonModule.h"

namespace fwk {

RunL3ResidualZmmJetModule::RunL3ResidualZmmJetModule(const GlobalFlag& gf)
    : globalFlags_(gf) {
    loadConfig("config/RunL3ResidualZmmJet.json");
}

RunL3ResidualZmmJetModule::~RunL3ResidualZmmJetModule() = default;

void RunL3ResidualZmmJetModule::loadConfig(const std::string& filename) {
    ReadConfig config(filename);
    cutflows_ = config.getValue<std::vector<std::string>>({"cutflows"});
    minTagPts_ = config.getValue<std::vector<int>>({"minTagPts"});
    maxDeltaPhiTagProbe_ = config.getValue<double>({"maxDeltaPhiTagProbe"});
    minPtJet2InAlpha_ = config.getValue<double>({"minPtJet2InAlpha"});
    maxAlpha_ = config.getValue<double>({"maxAlpha"});
    alphaCuts_ = config.getValue<std::vector<double>>({"alphaCuts"});
    minResp_ = config.getValue<double>({"minResp"});
    maxResp_ = config.getValue<double>({"maxResp"});
}

void RunL3ResidualZmmJetModule::beginJob(Context& ctx) {
    assert(ctx.out && ctx.out->file() && !ctx.out->file()->IsZombie());
    ctx.out->mkdirAndCd("Base");
    origDir_ = gDirectory;

    h1EventInCutflow_ = std::make_unique<HistCutflow>(origDir_, "", cutflows_, globalFlags_);

    varBin_ = std::make_unique<VarBin>(globalFlags_);
    histAlpha_ = std::make_unique<HistAlpha>(origDir_, "passDeltaPhiTagProbe", *varBin_, alphaCuts_);
    histObjP4Lep1_ = std::make_unique<HistObjP4>(origDir_, "passAlpha", *varBin_, "LeadLep");
    histObjP4Lep2_ = std::make_unique<HistObjP4>(origDir_, "passAlpha", *varBin_, "SubLeadLep");
    histObjP4Probe_ = std::make_unique<HistObjP4>(origDir_, "passAlpha", *varBin_, "Probe");
    histObjP4Tag_ = std::make_unique<HistObjP4>(origDir_, "passAlpha", *varBin_, "Tag");

    histObjGenRecoTag_ = std::make_unique<HistObjGenReco>(origDir_, "passL3Residual", *varBin_, "Tag");
    histObjGenRecoProbe_ = std::make_unique<HistObjGenReco>(origDir_, "passL3Residual", *varBin_, "Probe");
    histPfCompProbeInProbe_ = std::make_unique<HistPfComp>(origDir_, "passL3Residual", *varBin_, "Probe", "InProbe");
    histFlavorProbe_ = std::make_unique<HistFlavor>(origDir_, "passL3Residual", *varBin_, "Probe");
    histJecUncBand_ = std::make_unique<HistJecUncBand>(origDir_, "passL3Residual", *varBin_, "Probe");
    histL3Residual_ = std::make_unique<HistL3Residual>(origDir_, "passL3Residual", *varBin_);
    histTime_ = std::make_unique<HistTime>(origDir_, "passL3Residual", *varBin_, minTagPts_);

    pickZmmJet_ = std::make_shared<PickZmmJet>(globalFlags_);
    pickEventModule_ = std::make_unique<PickEventModule>(globalFlags_);

    scaleMuonModule_ = std::make_unique<ScaleMuonModule>(globalFlags_);
    scaleJetModule_ = std::make_unique<ScaleJetModule>(globalFlags_);
    pickGenJet_ = std::make_shared<PickGenJet>(globalFlags_);
    scaleMetModule_ = std::make_unique<ScaleMetModule>(globalFlags_);
    jecUncBand_ = std::make_shared<JecUncBand>(globalFlags_);
    mathL3Residual_ = std::make_shared<MathL3Residual>(globalFlags_);
    hemVeto_ = std::make_shared<HemVeto>(globalFlags_);

    totalTime_ = 0.0;
    startClock_ = std::chrono::high_resolution_clock::now();
    everyN_ = Helper::initProgress(ctx.skimT->getEntries());
}

bool RunL3ResidualZmmJetModule::analyze(Context& ctx, Event& ev) {
    auto& skimT = ctx.skimT;
    Helper::printProgressEveryN(ev.entry, skimT->getEntries(), everyN_, startClock_, totalTime_);

    TLorentzVector p4RawTag, p4Tag, p4GenTag, p4CorrMet;
    TLorentzVector p4Probe, p4Jet2, p4Jetn;

    double weight = 1.0;
    h1EventInCutflow_->fill("passSkim", weight);

    if (!pickEventModule_->passCoreEventCuts(skimT, h1EventInCutflow_.get(), weight)) {
        return true;
    }

    scaleMuonModule_->applyCorrections(skimT);

    pickZmmJet_->pickMuons(*skimT);
    auto pickedMuons = pickZmmJet_->getPickedMuons();
    pickZmmJet_->pickTags(*skimT);
    std::vector<TLorentzVector> p4Tags = pickZmmJet_->getPickedTags();
    if (p4Tags.size() != 1) return true;
    h1EventInCutflow_->fill("passExactly1Tag", weight);

    p4Tag = p4Tags.at(0);
    p4RawTag = p4Tags.at(0);
    const double ptTag = p4Tag.Pt();

    scaleJetModule_->applyCorrections(skimT);
    pickZmmJet_->pickJets(*skimT, p4Tag);

    std::vector<int> jetsIndex = pickZmmJet_->getPickedJetsIndex();
    int iProbe = jetsIndex.at(0);
    int iJet2 = jetsIndex.at(1);
    if (iProbe == -1) return true;
    h1EventInCutflow_->fill("passExactly1Probe", weight);

    std::vector<TLorentzVector> jetsP4 = pickZmmJet_->getPickedJetsP4();
    p4Probe = jetsP4.at(0);
    p4Jet2 = jetsP4.at(1);
    p4Jetn = jetsP4.at(2);
    p4Jetn += p4Jet2;

    const double ptProbe = p4Probe.Pt();
    const double etaProbe = p4Probe.Eta();

    const double deltaPhi = HelperDelta::DELTAPHI(p4Tag.Phi(), p4Probe.Phi());
    if (std::fabs(deltaPhi - TMath::Pi()) >= maxDeltaPhiTagProbe_) return true;
    h1EventInCutflow_->fill("passDeltaPhiTagProbe", weight);

    if (!pickEventModule_->passProbeJetVeto(p4Probe, h1EventInCutflow_.get(), weight)) {
        return true;
    }

    const double ptJet2 = p4Jet2.Pt();
    const double alpha = ptJet2 / ptTag;
    const bool passAlpha = (alpha < maxAlpha_ || ptJet2 < minPtJet2InAlpha_);

    scaleMetModule_->applyCorrections(skimT, scaleJetModule_->jetPtRaw());
    p4CorrMet = scaleMetModule_->correctedMet();
    p4CorrMet += p4RawTag - p4Tag;

    HistL3ResidualInput histL3ResidualInput = mathL3Residual_->computeResponse(p4Tag, p4Probe, p4CorrMet, p4Jetn);

    histAlpha_->Fill(alpha, skimT->Rho, histL3ResidualInput);
    if (!passAlpha) return true;
    h1EventInCutflow_->fill("passAlpha", weight);

    const double bal = histL3ResidualInput.respDb;
    const double mpf = histL3ResidualInput.respMpf;
    const bool passDbResp = bal > minResp_ && bal < maxResp_;
    const bool passMpfResp = mpf > minResp_ && mpf < maxResp_;
    if (!(passDbResp && passMpfResp)) return true;
    h1EventInCutflow_->fill("passL3Residual", weight);

    if (hemVeto_->isHemVeto(*skimT)) {
        if (globalFlags_.isData()) return true;
        weight *= hemVeto_->getMcWeight();
    }
    if (globalFlags_.isMC()) {
        auto muSFs1 = scaleMuonModule_->getMuonSfs(*skimT, pickedMuons[0], ScaleMuon::SystLevel::Nominal);
        auto muSFs2 = scaleMuonModule_->getMuonSfs(*skimT, pickedMuons[1], ScaleMuon::SystLevel::Nominal);
        weight *= muSFs1.total;
        weight *= muSFs2.total;
    }
    histL3ResidualInput.weight = weight;
    histL3Residual_->fillHistos(histL3ResidualInput);

    TLorentzVector p4Lep1, p4Lep2;
    const int iLep1 = pickedMuons.at(0);
    const int iLep2 = pickedMuons.at(1);
    p4Lep1.SetPtEtaPhiM(skimT->Muon_pt[iLep1], skimT->Muon_eta[iLep1], skimT->Muon_phi[iLep1], skimT->Muon_mass[iLep1]);
    p4Lep2.SetPtEtaPhiM(skimT->Muon_pt[iLep2], skimT->Muon_eta[iLep2], skimT->Muon_phi[iLep2], skimT->Muon_mass[iLep2]);
    histObjP4Lep1_->Fill(p4Lep1, weight);
    histObjP4Lep2_->Fill(p4Lep2, weight);
    histObjP4Probe_->Fill(p4Probe, weight);
    histObjP4Tag_->Fill(p4Tag, weight);

    histPfCompProbeInProbe_->Fill(skimT.get(), iProbe, ptProbe, etaProbe, weight);
    p4GenTag.SetPtEtaPhiM(0, 0, 0, 0);
    if (globalFlags_.isMC()) {
        double jesRelUnc = jecUncBand_->getJesRelUncForBand(etaProbe, ptProbe);
        double jerRelUnc = jecUncBand_->getJerSfRelUncForBand(etaProbe, "up");
        histJecUncBand_->Fill(etaProbe, ptProbe, jesRelUnc, jerRelUnc);
        pickZmmJet_->pickGenMuons(*skimT);
        pickZmmJet_->pickGenTags(*skimT, p4Tag);
        std::vector<TLorentzVector> p4GenTags = pickZmmJet_->getPickedGenTags();
        if (p4GenTags.empty()) return true;
        p4GenTag = p4GenTags.at(0);
        auto pickedGenJets = pickGenJet_->matchByRecoIndices(*skimT, iProbe, iJet2, p4Probe, p4Jet2);
        double ptGenProbe = pickedGenJets.p4GenJet1.Pt();
        histFlavorProbe_->Fill(ptProbe, skimT->Jet_partonFlavour[iProbe], weight);
        histObjGenRecoTag_->Fill(p4GenTag.Pt(), ptTag, weight);
        histObjGenRecoProbe_->Fill(ptGenProbe, ptProbe, weight);
    } else {
        histTime_->Fill(skimT.get(), iProbe, bal, mpf, ptProbe, weight);
    }

    return true;
}

void RunL3ResidualZmmJetModule::endJob(Context& ctx) {
    h1EventInCutflow_->printCutflow();
    h1EventInCutflow_->fillFractionCutflow();
    std::cout << "Output file: " << ctx.out->file()->GetName() << '\n';
}

} // namespace fwk
