#include "RunL2ResidualDiJet.h"
#include "ReadConfig.h"

#include "HistL2ResidualInput.hpp"
#include "MathL2Residual.h"
#include "HistL2Residual.h"
#include "HistCutflow.h"
#include "HistPfComp.h"
#include "HistScaleMet.h"
#include "HistScaleJet.h"
#include "HistObjJER.h"
#include "HistJecUncBand.h"

#include "PickDiJet.h"
#include "PickGenJet.h"
#include "PickEvent.h"

#include "ScaleJet.h"
#include "ScaleMet.h"
#include "JecUncBand.h"

#include "Helper.hpp"
#include "HelperDelta.hpp"
#include "VarBin.h"
#include "HemVeto.h"
#include "Hlt.h"

#include <cassert>
#include <cmath>
#include <TMath.h>
#include <chrono>
#include <memory>

// Constructor: load run‐level config
RunL2ResidualDiJet::RunL2ResidualDiJet(const GlobalFlag& globalFlags)
  : globalFlags_(globalFlags)
{
    loadConfig("config/RunL2ResidualDiJet.json");
}

void RunL2ResidualDiJet::loadConfig(const std::string& filename) {
    ReadConfig config(filename);

    // Read the configuration values using getValue() for proper error handling.
    cutflows_          = config.getValue<std::vector<std::string>>({"cutflows"});
    dPhiWindow_        = config.getValue<double>({"dPhiWindow"});
    maxAsymmetry_      = config.getValue<double>({"maxAsymmetry"});
    fillPerHlt_        = config.getValue<bool>({"fillPerHlt"});
}

auto RunL2ResidualDiJet::Run(std::shared_ptr<SkimTree>& skimT,
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

    HistScaleMet    histScaleMet (origDir, "passL2Residual", varBin);
    HistScaleJet    histScaleJet (origDir, "passL2Residual", varBin);
    HistPfComp histPfCompProbeInProbe(origDir, "passL2Residual", varBin, 
                                         "Probe", "InProbe");
    HistObjJER histTagJER(origDir,   "passL2Residual", varBin, "Tag");
    HistObjJER histProbeJER(origDir, "passL2Residual", varBin, "Probe");
    HistJecUncBand  histJecUncBand (origDir, "passL2Residual", varBin, "Probe");
    HistL2Residual  histL2Residual (origDir, "passL2Residual", varBin);
    auto hemVeto = std::make_shared<HemVeto>(globalFlags_);


    Hlt hlt(globalFlags_);
    const auto& hlts = hlt.getTrigMapRangePtEta();
    std::map<std::string, std::unique_ptr<HistL2Residual>> mapHistL2Residual;
    if (fillPerHlt_) {
        for (auto const& [trigName, r] : hlts) {
            // ✔ construct the object with make_unique
            auto hMJ = std::make_unique<HistL2Residual>(
                origDir,
                "passL2Residual/" + trigName,
                varBin
            );
            // transfer ownership into the map
            mapHistL2Residual[trigName] = std::move(hMJ);
        }
    }

    auto pickDiJet = std::make_shared<PickDiJet>(globalFlags_);
    auto pickGenJet= std::make_shared<PickGenJet>(globalFlags_);
    auto pickEvent = std::make_shared<PickEvent>(globalFlags_);

    auto scaleJet  = std::make_shared<ScaleJet>(globalFlags_);
    auto scaleMet  = std::make_shared<ScaleMet>(globalFlags_);
    auto jecUncBand  = std::make_shared<JecUncBand>(globalFlags_);
    auto mathL2Residual  = std::make_shared<MathL2Residual>(globalFlags_);

    // timing & progress
    double totalTime = 0.0;
    auto startClock = std::chrono::high_resolution_clock::now();
    Long64_t nentries = skimT->getEntries();
    const Long64_t everyN = Helper::initProgress(nentries);

    TLorentzVector p4GenTag, p4GenProbe;

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

        if (!pickEvent->passMatchedGenVtx(*skimT)) continue;
        h1EventInCutflow->fill("passMatchedGenVtx", weight);

        // Apply JEC
        scaleJet->applyCorrection(skimT);

        // --- DiJet selection via PickDiJet ---
        pickDiJet->pickJets(*skimT);

        int    iTag             = pickDiJet->getIndexTag();
        int    iProbe           = pickDiJet->getIndexProbe();
        auto   p4Tag            = pickDiJet->getP4Tag();
        auto   p4Probe          = pickDiJet->getP4Probe();
        double ptTag            = p4Tag.Pt(); 
        double ptProbe          = p4Probe.Pt();
        double etaProbe         = p4Probe.Eta();
        auto   p4SumOther       = pickDiJet->getP4SumOther();

        bool isHemVeto = hemVeto->isHemVeto(*skimT);
        if(isHemVeto){
            if(globalFlags_.isData()) continue;
            else weight *= hemVeto->getMcWeight();
        }
        // At least two good lead
        if (iTag < 0 || iProbe < 0) continue;
        h1EventInCutflow->fill("passTagAndProbe", weight);

        if (!pickEvent->passPtHatFilterAuto(*skimT, ptProbe)) continue;
        h1EventInCutflow->fill("passPtHatFilter", weight);

        // Δφ(tag, probe) ≃ π
        double deltaPhi = HelperDelta::DELTAPHI(p4Tag.Phi(), p4Probe.Phi());
        if (!(std::fabs(deltaPhi - TMath::Pi()) < dPhiWindow_)) continue;
        h1EventInCutflow->fill("passDeltaPhiTnP", weight);

        // MET & unclustered
        scaleMet->applyCorrection(skimT, scaleJet->getJetPtRaw());
        TLorentzVector p4CorrMet = scaleMet->getP4CorrectedMet();
        TLorentzVector p4SumTnP = p4Probe + p4Tag;

        //------------------------------------
        // Compute inputs for histograms
        //------------------------------------
        HistL2ResidualInput histL2ResidualInput = mathL2Residual->computeResponse(
                                                    p4Tag, p4Probe, 
                                                    p4CorrMet, p4SumOther);

        if(std::abs(histL2ResidualInput.asymmA) > maxAsymmetry_) continue;
        if(std::abs(histL2ResidualInput.asymmB) > maxAsymmetry_) continue;
        h1EventInCutflow->fill("passMaxAsymmetry", weight);
        
        
        //------------------------------------
        // 3) Fill HistL2Residual per HLT
        //------------------------------------
        auto passedHlts = pickEvent->getPassedHlts();

        if(fillPerHlt_){
            for (const auto& trigPair : hlts) {
                const auto& trigName = trigPair.first;
                if (std::find(passedHlts.begin(), passedHlts.end(), trigName) == passedHlts.end())
                    continue;
                double weightTemp = hlt.getHltLumiWeight(trigName);
                histL2ResidualInput.weight    = weight*weightTemp;
                auto* h = mapHistL2Residual[trigName].get();
                h->fillHistos(histL2ResidualInput);
            }
        }

        // Jet veto map
        if (!pickEvent->passJetVetoMapOnProbe(p4Probe)) continue;
        h1EventInCutflow->fill("passJetVetoMap", weight);

        //------------------------------------
        // 4) All Exclusive HLT w/ Pt/Eta
        //------------------------------------
        double ptForHlt = (ptTag + ptProbe)/2.0;
        if (globalFlags_.getJetAlgo() == GlobalFlag::JetAlgo::AK8Puppi) {
            ptForHlt = ptProbe;
        }
        if (!pickEvent->passHltWithPtEta(skimT, ptTag, etaProbe))//FIXME
            continue;
        h1EventInCutflow->fill("passL2Residual");
        histScaleJet.Fill(*scaleJet);
        histScaleMet.Fill(*scaleMet);

        const std::string passedHltName = pickEvent->getPassedHlt();
        weight *= hlt.getHltLumiWeight(passedHltName);
        histL2ResidualInput.weight    = weight;
    
        auto it = hlts.find(passedHltName);
        histL2Residual.fillHistos(histL2ResidualInput);
        histPfCompProbeInProbe.Fill(skimT.get(), iProbe, ptProbe, etaProbe, weight);

        // Gen objects
        if (globalFlags_.isMC()) {
            double jesRelUnc = jecUncBand->getJesRelUncForBand(etaProbe, ptProbe);
            double jerRelUnc = jecUncBand->getJerSfRelUncForBand(etaProbe, "up");
            histJecUncBand.Fill(etaProbe, ptProbe, jesRelUnc, jerRelUnc);
            auto pickedGenJets = pickGenJet->matchByRecoIndices(
                                *skimT, iTag, iProbe, p4Tag, p4Probe);
            histTagJER.Fill(pickedGenJets.p4GenJet1, p4Tag, skimT->Rho,  weight);
            histProbeJER.Fill(pickedGenJets.p4GenJet2, p4Probe, skimT->Rho,  weight);
        }

    } // end event loop
    h1EventInCutflow->printCutflow();
    h1EventInCutflow->fillFractionCutflow();
    std::cout << "Output file: " << fout->GetName() << "\n";
    fout->Write();
    return 0;
}

