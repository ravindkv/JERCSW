#include "RunL3ResidualMultiJet.h"
#include "ReadConfig.h"

#include "HistCutflow.h"
#include "HistMultiJet.h"
#include "HistTime.h"
#include "HistFlavor.h"
#include "HistPfComp.h"

#include "PickMultiJet.h"
#include "PickEvent.h"
#include "ScaleJet.h"
#include "ScaleMet.h"
#include "HemVeto.h"

#include "Helper.hpp"
#include "HelperDelta.hpp"
#include "VarBin.h"
#include "MathHdm.h"
#include "Hlt.h"

#include <cassert>
#include <cmath>
#include <TMath.h>
#include <chrono>

// Constructor: load run‐level config
RunL3ResidualMultiJet::RunL3ResidualMultiJet(const GlobalFlag& globalFlags)
  : globalFlags_(globalFlags)
{
    loadConfig("config/RunL3ResidualMultiJet.json");
}

void RunL3ResidualMultiJet::loadConfig(const std::string& filename) {
    ReadConfig config(filename);

    // Read the configuration values using getValue() for proper error handling.
    cutflows_          = config.getValue<std::vector<std::string>>({"cutflows"});
    minTagPts_         = config.getValue<std::vector<int>>({"minTagPts"});
    minRecoilJets_     = config.getValue<int>   ({"minRecoilJets"});
    dPhiWindow_        = config.getValue<double>({"dPhiWindow"});
    maxRecoilFraction_ = config.getValue<double>({"maxRecoilFraction"});
    fillPerHlt_        = config.getValue<bool>({"fillPerHlt"});
}

auto RunL3ResidualMultiJet::Run(std::shared_ptr<SkimTree>& skimT,
                      ScaleEvent*          scaleEvent,
                      TFile*               fout) -> int
{
    assert(fout && !fout->IsZombie());
    fout->mkdir("Base");
    fout->cd("Base");
    TDirectory* origDir = gDirectory;

    //------------------------------------
    // 1) Initialise histograms & dirs
    //------------------------------------
    auto h1EventInCutflow = std::make_unique<HistCutflow>(origDir, "", cutflows_, globalFlags_);

    VarBin varBin(globalFlags_);

    HistTime      histTime     (origDir, "passMultiJet", varBin, minTagPts_);
    HistMultiJet  histMultiJet (origDir, "passMultiJet", varBin);
    HistFlavor histFlavorProbe(origDir, "passMultiJet", varBin, "Probe");
    HistFlavor histFlavorRecoilJets(origDir, "passMultiJet", varBin, "RecoilJets");
    HistPfComp histPfCompProbeInProbe(origDir, "passMultiJet", varBin, "Probe", "InProbe");
    HistPfComp histPfCompRecoilJetsInTag(origDir, "passMultiJet", varBin, 
                                                "RecoilJets", "inTag");


    Hlt hlt(globalFlags_);
    const auto& hlts = hlt.getTrigMapRangePtEta();
    std::map<std::string, HistMultiJet*> mapHistMultiJet;
    if(fillPerHlt_){
        for (const auto& trigPair : hlts) {
            const auto& trigName = trigPair.first;
            const auto& r        = trigPair.second;
            auto* hMJ = new HistMultiJet(origDir, "passMultiJet/"+trigName, varBin);
            hMJ->trigPt = r.trigPt;
            mapHistMultiJet[trigName] = hMJ;
        }
    }

    auto pickMultiJet = std::make_shared<PickMultiJet>(globalFlags_);
    auto pickEvent = std::make_shared<PickEvent>(globalFlags_);
    auto scaleJet  = std::make_shared<ScaleJet>(globalFlags_);
    auto scaleMet  = std::make_shared<ScaleMet>(globalFlags_);
    auto hemVeto = std::make_shared<HemVeto>(globalFlags_);
    MathHdm mathHdm(globalFlags_);

    // timing & progress
    double totalTime = 0.0;
    auto startClock = std::chrono::high_resolution_clock::now();
    Long64_t nentries = skimT->getEntries();
    const Long64_t everyN = Helper::initProgress(nentries);

    //------------------------------------
    // 2) Event loop
    //------------------------------------
    for (Long64_t jentry = 0; jentry < nentries; ++jentry) {
        if (globalFlags_.isDebug() && jentry > globalFlags_.getNDebug()) break;
        Helper::printProgressEveryN(jentry, nentries, everyN, startClock, totalTime);
        skimT->getEntry(jentry);

        // Weight
        Double_t weight = scaleEvent->getEventWeight(*skimT);
        h1EventInCutflow->fill("passSkim", weight);

        //------------------------------------
        // Trigger and golden lumi
        //------------------------------------
        if (!pickEvent->passHlt(skimT)) continue; 
        h1EventInCutflow->fill("passHlt", weight);

        if (!pickEvent->passGoodLumi(skimT->run, skimT->luminosityBlock)) continue; 
        h1EventInCutflow->fill("passGoodLumi", weight);

        auto passedHlts = pickEvent->getPassedHlts();
        bool isHemVeto = hemVeto->isHemVeto(*skimT);
        if(isHemVeto){
            if(globalFlags_.isData()) continue;
            else weight *= hemVeto->getMcWeight();
        }

        // --- Multi‐jet selection via PickMultiJet ---
        scaleJet->applyCorrection(skimT);
        pickMultiJet->pickJets(*skimT);

        int    iProbe             = pickMultiJet->getProbeIndex();
        double ptProbe            = pickMultiJet->getProbe().Pt();
        auto   recoilIndices     = pickMultiJet->getRecoilIndices();
        double ptHardestInRecoil = pickMultiJet->getHardestRecoilPt();
        auto   p4Probe         = pickMultiJet->getProbe();
        auto   p4Tag = pickMultiJet->getSumRecoiledJets();
        auto   p4SumOther        = pickMultiJet->getSumOther();

        // Exactly one good lead
        if (iProbe < 0)                continue;
        h1EventInCutflow->fill("passExactly1Probe", weight);

        // At least N recoil jets
        if (recoilIndices.size() < static_cast<size_t>(minRecoilJets_)) continue;
        h1EventInCutflow->fill("passAtleast2Recoil", weight);

        // Jet veto map
        if (!pickEvent->passJetVetoMapOnProbe(p4Probe)) continue;
        h1EventInCutflow->fill("passJetVetoMap", weight);

        // Δφ(lead, sumRecoil) ≃ π
        double deltaPhi = HelperDelta::DELTAPHI(p4Probe.Phi(), p4Tag.Phi());
        if (!(std::fabs(deltaPhi - TMath::Pi()) < dPhiWindow_)) continue;
        h1EventInCutflow->fill("passDeltaPhiProbeAndRecoil", weight);

        // Veto events if there are near & forward jets
        if (pickMultiJet->vetoNearByJets())    continue;
        if (pickMultiJet->vetoForwardJets())   continue;
        h1EventInCutflow->fill("passVetoNearByJets", weight);

        // Multi‐jet imbalance
        if (!(ptHardestInRecoil < maxRecoilFraction_ * p4Tag.Pt()))
            continue;
        h1EventInCutflow->fill("passHardestInRecoil", weight);

        //------------------------------------
        // Compute inputs for histograms
        //------------------------------------
        // Bisector axis
        auto p4Bisector = mathHdm.buildUnitAxisForBisector(p4Tag, p4Probe);
        double ptAvgProj = 0.5 * (
            p4Tag.Vect().Dot(p4Bisector.Vect())
          - p4Probe.Vect().Dot(p4Bisector.Vect())
        );

        double ptTag  = p4Tag.Pt();
        double ptAverage = 0.5 * (ptProbe + ptTag);
        
        // Crecoil
        double logCrecoil = 0.0, sumF_i = 0.0;
        std::vector<double> recoilFs;
        TLorentzVector p4J;
        for (int idx : recoilIndices) {
            p4J.SetPtEtaPhiM(
                skimT->Jet_pt[idx],
                skimT->Jet_eta[idx],
                skimT->Jet_phi[idx],
                skimT->Jet_mass[idx]
            );
            double f_i = p4J.Pt() / ptTag;
            double F_i = f_i * std::cos(HelperDelta::DELTAPHI(p4J.Phi(), p4Tag.Phi()));
            sumF_i   += F_i;
            logCrecoil += F_i * std::log(f_i);
            recoilFs.push_back(F_i);
        }
        const double epsilon = 1e-4;
        if(std::abs(sumF_i - 1.0) > epsilon) {
            std::cout << "sumF_i not equal to 1: " << sumF_i << std::endl;
        }

        double cRecoil = std::exp(logCrecoil);
        
        // MET & unclustered
        scaleMet->applyCorrection(skimT, scaleJet->getJetPtRaw());
        TLorentzVector p4CorrMet = scaleMet->getP4CorrectedMet();
        TLorentzVector p4SumProbeAndRecoil = p4Probe + p4Tag;
        TLorentzVector p4Unclustered = -(p4CorrMet + p4SumProbeAndRecoil + p4SumOther);

        // Fill inputs struct
        HistMultiJetInputs fillInputs;
        fillInputs.ptAvgProj = ptAvgProj;
        fillInputs.ptAverage = ptAverage;
        fillInputs.ptProbe    = ptProbe;
        fillInputs.ptMet     = p4CorrMet.Pt();
        fillInputs.ptOther   = p4SumOther.Pt();
        fillInputs.ptUnclustered = p4Unclustered.Pt();
        fillInputs.etaProbe   = p4Probe.Eta();
        fillInputs.phiProbe   = p4Probe.Phi();
        fillInputs.ptRecoil  = ptTag;
        fillInputs.phiRecoil = p4Tag.Phi();
        fillInputs.cRecoil   = cRecoil;
        fillInputs.mjbResp  = ptProbe/ptTag;

        p4CorrMet.SetPtEtaPhiM(p4CorrMet.Pt(), 0., p4CorrMet.Phi(), 0.);
        p4SumProbeAndRecoil.SetPtEtaPhiM(p4SumProbeAndRecoil.Pt(), 0., p4SumProbeAndRecoil.Phi(), 0.);
        p4Probe.SetPtEtaPhiM(p4Probe.Pt(), 0., p4Probe.Phi(), 0.);
        p4Tag.SetPtEtaPhiM(p4Tag.Pt(), 0., p4Tag.Phi(), 0.);
        p4SumOther.SetPtEtaPhiM(p4SumOther.Pt(), 0., p4SumOther.Phi(), 0.);

        // MPF responses
        double one = 1.0, zero = 0.0;
        fillInputs.m0b = mathHdm.mpfResponse(p4CorrMet,       p4Bisector, ptAvgProj, one);
        fillInputs.mlrb = mathHdm.mpfResponse(p4SumProbeAndRecoil, p4Bisector, ptAvgProj, one);
        fillInputs.mlb = mathHdm.mpfResponse(-p4Probe, p4Bisector, ptAvgProj, zero);
        fillInputs.mrb = mathHdm.mpfResponse(p4Tag, p4Bisector, ptAvgProj, zero);
        fillInputs.mnb = mathHdm.mpfResponse(p4SumOther,      p4Bisector, ptAvgProj, zero);
        fillInputs.mub = mathHdm.mpfResponse(p4Unclustered,   p4Bisector, ptAvgProj, zero);

        auto p4M = mathHdm.buildUnitAxis(-p4Probe, p4Tag);
        fillInputs.m0m = mathHdm.mpfResponse(p4CorrMet,       p4M, ptAverage, one);
        fillInputs.mlrm = mathHdm.mpfResponse(p4SumProbeAndRecoil, p4M, ptAverage, one);
        fillInputs.mlm = mathHdm.mpfResponse(-p4Probe, p4M, ptAverage, zero);
        fillInputs.mrm = mathHdm.mpfResponse(p4Tag, p4M, ptAverage, zero);
        fillInputs.mnm = mathHdm.mpfResponse(p4SumOther,      p4M, ptAverage, zero);
        fillInputs.mum = mathHdm.mpfResponse(p4Unclustered,   p4M, ptAverage, zero);

        auto p4L = mathHdm.buildUnitAxis(-p4Probe, TLorentzVector());
        fillInputs.m0l = mathHdm.mpfResponse(p4CorrMet,     p4L, ptProbe, one);
        fillInputs.mlrl = mathHdm.mpfResponse(p4SumProbeAndRecoil, p4L, ptProbe, one);
        fillInputs.mll = mathHdm.mpfResponse(-p4Probe, p4L, ptProbe, zero);
        fillInputs.mrl = mathHdm.mpfResponse(p4Tag, p4L, ptProbe, zero);
        fillInputs.mnl = mathHdm.mpfResponse(p4SumOther,    p4L, ptProbe, zero);
        fillInputs.mul = mathHdm.mpfResponse(p4Unclustered, p4L, ptProbe, zero);

        auto p4R = mathHdm.buildUnitAxis(p4Tag, TLorentzVector());
        fillInputs.m0r = mathHdm.mpfResponse(p4CorrMet,     p4R, ptTag, one);
        fillInputs.mlrr = mathHdm.mpfResponse(p4SumProbeAndRecoil, p4R, ptTag, one);
        fillInputs.mlr = mathHdm.mpfResponse(-p4Probe, p4R, ptTag, zero);
        fillInputs.mrr = mathHdm.mpfResponse(p4Tag, p4R, ptTag, zero);
        fillInputs.mnr = mathHdm.mpfResponse(p4SumOther,    p4R, ptTag, zero);
        fillInputs.mur = mathHdm.mpfResponse(p4Unclustered, p4R, ptTag, zero);
        fillInputs.weight    = weight;

        //------------------------------------
        // 3) Fill HistMultiJet per HLT
        //------------------------------------
        if(fillPerHlt_){
            for (const auto& trigPair : hlts) {
                const auto& trigName = trigPair.first;
                if (std::find(passedHlts.begin(), passedHlts.end(), trigName) == passedHlts.end())
                    continue;
                double weightTemp = hlt.getHltLumiWeight(trigName);
                fillInputs.weight    = weight*weightTemp;
                auto* h = mapHistMultiJet[trigName];
                h->setInputs(fillInputs);
                for (size_t i = 0; i < recoilIndices.size(); ++i) {
                    h->fillJetLevelHistos(
                        skimT.get(), recoilIndices[i], weight * recoilFs[i]*weightTemp
                    );
                }
                h->fillEventLevelHistos(skimT.get(), iProbe, h->trigPt);
            }
        }

        //------------------------------------
        // 4) All Exclusive HLT w/ Pt/Eta
        //------------------------------------
        if (!pickEvent->passHltWithPtEta(skimT, ptProbe, p4Probe.Eta()))
            continue;
        h1EventInCutflow->fill("passMultiJet", weight);
        const std::string passedHltName = pickEvent->getPassedHlt();
        weight *= hlt.getHltLumiWeight(passedHltName);
        fillInputs.weight    = weight;
    
        histMultiJet.setInputs(fillInputs);
        if(globalFlags_.isDebug()) histMultiJet.printInputs();
        for (size_t i = 0; i < recoilIndices.size(); ++i) {
            histMultiJet.fillJetLevelHistos(
                skimT.get(), recoilIndices[i], weight * recoilFs[i]
            );
        }
        double trigPt = 1.0;
        auto it = hlts.find(passedHltName);
        if (it != hlts.end()) trigPt = it->second.trigPt;
        histMultiJet.fillEventLevelHistos(skimT.get(), iProbe, trigPt);
        //Flavor fractions
        if(globalFlags_.isMC()){
            histFlavorProbe.Fill(ptProbe, skimT->Jet_partonFlavour[iProbe], weight);
            for (int idx : recoilIndices) {
                histFlavorRecoilJets.Fill(ptProbe, skimT->Jet_partonFlavour[idx], weight);
            }
        }
        histPfCompProbeInProbe.Fill(skimT.get(), iProbe, ptProbe, p4Probe.Eta(), weight);
        for (int idx : recoilIndices) {
            histPfCompRecoilJetsInTag.Fill(skimT.get(), idx, ptTag, p4Tag.Eta(),  weight);
        }

        // Fill ref & timing
        if (globalFlags_.isData()) {
            histTime.Fill(
                skimT.get(),
                iProbe,
                fillInputs.m0l,
                fillInputs.mlrl,
                p4Probe.Pt(),
                weight
            );
        }
    } // end event loop

    h1EventInCutflow->printCutflow();
    h1EventInCutflow->fillFractionCutflow();
    std::cout << "Output file: " << fout->GetName() << "\n";
    fout->Write();
    return 0;
}

