#include "fwk/RunL3ResidualZmmJetModule.h"

#include "PickGenJet.h"
#include "PickZmmJet.h"
#include "ReadConfig.h"
#include "fwk/Context.h"

namespace fwk {

RunL3ResidualZmmJetModule::RunL3ResidualZmmJetModule(const GlobalFlag& gf)
    : L3ResidualBaseModule(gf) {}

RunL3ResidualZmmJetModule::~RunL3ResidualZmmJetModule() = default;

void RunL3ResidualZmmJetModule::loadChannelConfig(const std::string& filename) {
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

void RunL3ResidualZmmJetModule::bookChannelHistograms(TDirectory* origDir) {
    histObjP4Lep1_ = std::make_unique<HistObjP4>(origDir, "passAlpha", *varBin_, "LeadLep");
    histObjP4Lep2_ = std::make_unique<HistObjP4>(origDir, "passAlpha", *varBin_, "SubLeadLep");
    histObjP4Probe_ = std::make_unique<HistObjP4>(origDir, "passAlpha", *varBin_, "Probe");
    histObjP4Tag_ = std::make_unique<HistObjP4>(origDir, "passAlpha", *varBin_, "Tag");

    histObjGenRecoTag_ = std::make_unique<HistObjGenReco>(origDir, "passL3Residual", *varBin_, "Tag");
    histObjGenRecoProbe_ = std::make_unique<HistObjGenReco>(origDir, "passL3Residual", *varBin_, "Probe");
    histPfCompProbeInProbe_ = std::make_unique<HistPfComp>(origDir, "passL3Residual", *varBin_, "Probe", "InProbe");
    histFlavorProbe_ = std::make_unique<HistFlavor>(origDir, "passL3Residual", *varBin_, "Probe");
    histJecUncBand_ = std::make_unique<HistJecUncBand>(origDir, "passL3Residual", *varBin_, "Probe");
    histTime_ = std::make_unique<HistTime>(origDir, "passL3Residual", *varBin_, minTagPts_);

    pickZmmJet_ = std::make_shared<PickZmmJet>(globalFlags_);
    pickGenJet_ = std::make_shared<PickGenJet>(globalFlags_);
    mathL3Residual_ = std::make_shared<MathL3Residual>(globalFlags_);
}

bool RunL3ResidualZmmJetModule::pickObjects(Context& ctx,
                                            L3ResidualObjects& objects,
                                            double& weight) {
    auto& skimT = ctx.skimT;

    pickZmmJet_->pickMuons(*skimT);
    pickedMuons_ = pickZmmJet_->getPickedMuons();

    pickZmmJet_->pickTags(*skimT);
    std::vector<TLorentzVector> p4Tags = pickZmmJet_->getPickedTags();
    if (p4Tags.size() != 1) {
        return false;
    }
    hCutflow_->fill("passExactly1Tag", weight);

    objects.p4Tag = p4Tags.at(0);
    objects.p4RawTag = p4Tags.at(0);

    pickZmmJet_->pickJets(*skimT, objects.p4Tag);

    std::vector<int> jetsIndex = pickZmmJet_->getPickedJetsIndex();
    objects.iProbe = jetsIndex.at(0);
    objects.iJet2 = jetsIndex.at(1);
    if (objects.iProbe == -1) {
        return false;
    }
    hCutflow_->fill("passExactly1Probe", weight);

    std::vector<TLorentzVector> jetsP4 = pickZmmJet_->getPickedJetsP4();
    objects.p4Probe = jetsP4.at(0);
    objects.p4Jet2 = jetsP4.at(1);
    objects.p4Jetn = jetsP4.at(2);
    objects.p4Jetn += objects.p4Jet2;

    return true;
}

HistL3ResidualInput RunL3ResidualZmmJetModule::computeResponse(const L3ResidualObjects& objects,
                                                               const TLorentzVector& p4CorrMet) {
    return mathL3Residual_->computeResponse(objects.p4Tag, objects.p4Probe, p4CorrMet, objects.p4Jetn);
}

void RunL3ResidualZmmJetModule::applyChannelWeights(Context& ctx,
                                                    const L3ResidualObjects&,
                                                    double& weight) {
    if (!globalFlags_.isMC()) {
        return;
    }

    auto& skimT = ctx.skimT;
    auto muSFs1 = scaleMuonModule_->getMuonSfs(*skimT, pickedMuons_[0], ScaleMuon::SystLevel::Nominal);
    auto muSFs2 = scaleMuonModule_->getMuonSfs(*skimT, pickedMuons_[1], ScaleMuon::SystLevel::Nominal);
    weight *= muSFs1.total;
    weight *= muSFs2.total;
}

void RunL3ResidualZmmJetModule::fillChannelSpecificHistos(Context& ctx,
                                                          const L3ResidualObjects& objects,
                                                          const HistL3ResidualInput& input,
                                                          double,
                                                          double weight) {
    auto& skimT = ctx.skimT;

    const double ptProbe = objects.p4Probe.Pt();
    const double etaProbe = objects.p4Probe.Eta();

    TLorentzVector p4Lep1;
    TLorentzVector p4Lep2;
    const int iLep1 = pickedMuons_.at(0);
    const int iLep2 = pickedMuons_.at(1);
    p4Lep1.SetPtEtaPhiM(skimT->Muon_pt[iLep1], skimT->Muon_eta[iLep1], skimT->Muon_phi[iLep1], skimT->Muon_mass[iLep1]);
    p4Lep2.SetPtEtaPhiM(skimT->Muon_pt[iLep2], skimT->Muon_eta[iLep2], skimT->Muon_phi[iLep2], skimT->Muon_mass[iLep2]);
    histObjP4Lep1_->Fill(p4Lep1, weight);
    histObjP4Lep2_->Fill(p4Lep2, weight);
    histObjP4Probe_->Fill(objects.p4Probe, weight);
    histObjP4Tag_->Fill(objects.p4Tag, weight);

    histPfCompProbeInProbe_->Fill(skimT.get(), objects.iProbe, ptProbe, etaProbe, weight);

    if (globalFlags_.isMC()) {
        double jesRelUnc = jecUncBand_->getJesRelUncForBand(etaProbe, ptProbe);
        double jerRelUnc = jecUncBand_->getJerSfRelUncForBand(etaProbe, "up");
        histJecUncBand_->Fill(etaProbe, ptProbe, jesRelUnc, jerRelUnc);

        pickZmmJet_->pickGenMuons(*skimT);
        pickZmmJet_->pickGenTags(*skimT, objects.p4Tag);
        std::vector<TLorentzVector> p4GenTags = pickZmmJet_->getPickedGenTags();
        if (p4GenTags.empty()) {
            return;
        }

        auto pickedGenJets = pickGenJet_->matchByRecoIndices(*skimT,
                                                              objects.iProbe,
                                                              objects.iJet2,
                                                              objects.p4Probe,
                                                              objects.p4Jet2);
        double ptGenProbe = pickedGenJets.p4GenJet1.Pt();
        histFlavorProbe_->Fill(ptProbe, skimT->Jet_partonFlavour[objects.iProbe], weight);
        histObjGenRecoTag_->Fill(p4GenTags.at(0).Pt(), objects.p4Tag.Pt(), weight);
        histObjGenRecoProbe_->Fill(ptGenProbe, ptProbe, weight);
        return;
    }

    histTime_->Fill(skimT.get(), objects.iProbe, input.respDb, input.respMpf, ptProbe, weight);
}

} // namespace fwk
