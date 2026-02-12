#include "RunL2ResidualGamJet.h"
#include "ReadConfig.h"

#include "HistL2ResidualInput.hpp"
#include "MathL2Residual.h"
#include "HistL2Residual.h"
#include "HistCutflow.h"
#include "HistPfComp.h"
#include "HistScaleMet.h"
#include "HistScaleJet.h"
#include "HistScalePhoton.h"
#include "HistObjJER.h"
#include "HistObjP4.h"
#include "HistJecUncBand.h"

#include "PickGamJet.h"
#include "PickGenJet.h"
#include "PickEvent.h"
#include "ScaleJet.h"
#include "ScaleMet.h"
#include "JecUncBand.h"
#include "ScalePhoton.h"

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
RunL2ResidualGamJet::RunL2ResidualGamJet(const GlobalFlag& globalFlags)
  : globalFlags_(globalFlags)
{
    loadConfig("config/RunL2ResidualGamJet.json");
}

void RunL2ResidualGamJet::loadConfig(const std::string& filename) {
    ReadConfig config(filename);

    // Read the configuration values using getValue() for proper error handling.
    cutflows_          = config.getValue<std::vector<std::string>>({"cutflows"});
    dPhiWindow_        = config.getValue<double>({"dPhiWindow"});
    maxAsymmetry_      = config.getValue<double>({"maxAsymmetry"});
    nPhotonMin_        = config.getValue<int>({"nPhotonMin"});
}

auto RunL2ResidualGamJet::Run(std::shared_ptr<SkimTree>& skimT,
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

    HistScaleJet    histScaleJet (origDir, "passL2Residual", varBin);
    HistScalePhoton    histScalePhoton (origDir, "passL2Residual", varBin);
    HistScaleMet    histScaleMet (origDir, "passL2Residual", varBin);
    HistObjP4 histObjP4Tag(origDir, "passExactly1Tag", varBin, "Tag");
    HistPfComp histPfCompProbeInProbe(origDir, "passL2Residual", 
                                            varBin, "Probe", "InProbe");
    HistObjJER histTagJER(origDir,   "passL2Residual", varBin, "Tag");
    HistObjJER histProbeJER(origDir, "passL2Residual", varBin, "Probe");
    HistJecUncBand  histJecUncBand (origDir, "passL2Residual", varBin, "Probe");
    HistL2Residual  histL2Residual (origDir, "passL2Residual", varBin);
    auto hemVeto = std::make_shared<HemVeto>(globalFlags_);


    Hlt hlt(globalFlags_);
    std::map<std::string, std::unique_ptr<HistL2Residual>> mapHistL2Residual;
    auto pickGamJet = std::make_shared<PickGamJet>(globalFlags_);
    auto pickGenJet= std::make_shared<PickGenJet>(globalFlags_);
    auto pickEvent = std::make_shared<PickEvent>(globalFlags_);
    auto scalePhoton = std::make_shared<ScalePhoton>(globalFlags_);

    auto scaleJet  = std::make_shared<ScaleJet>(globalFlags_);
    auto scaleMet  = std::make_shared<ScaleMet>(globalFlags_);
    auto jecUncBand  = std::make_shared<JecUncBand>(globalFlags_);
    auto mathL2Residual  = std::make_shared<MathL2Residual>(globalFlags_);

    // timing & progress
    double totalTime = 0.0;
    auto startClock = std::chrono::high_resolution_clock::now();
    Long64_t nentries = skimT->getEntries();
    const Long64_t everyN = Helper::initProgress(nentries);

    //------------------------------------
    // 2) Event loop
    //------------------------------------
    TLorentzVector p4Tag, p4Probe, p4Jet2, p4SumOther, p4GenTag;
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

        // Apply Photon Corrections 
        scalePhoton->applyCorrections(skimT);

        pickGamJet->pickPhotons(*skimT);
        auto pickedPhotons = pickGamJet->getPickedPhotons();
        int nPickedPhoton = pickedPhotons.size();
        if (nPickedPhoton < nPhotonMin_) continue;

        pickGamJet->pickTag(*skimT);
        p4Tag = pickGamJet->getPickedTag();
        double ptTag = p4Tag.Pt();
        if (ptTag==0) continue; 
        h1EventInCutflow->fill("passExactly1Tag", weight);

        // Apply JEC
        scaleJet->applyCorrection(skimT);
        pickGamJet->pickJets(*skimT, p4Tag);
        //Pick index of jets
        std::vector<int> jetsIndex = pickGamJet->getPickedJetsIndex();
        int iProbe = jetsIndex.at(0);
        if (iProbe==-1) continue; 
        h1EventInCutflow->fill("passExactly1Probe", weight);

        //Pick p4 of jets
        std::vector<TLorentzVector> jetsP4 = pickGamJet->getPickedJetsP4();
        p4Probe  = jetsP4.at(0);
        p4Jet2  = jetsP4.at(1);
        p4SumOther  = jetsP4.at(2);
        p4SumOther+= p4Jet2; // except leading jet

        double ptProbe          = p4Probe.Pt();
        double etaProbe         = p4Probe.Eta();

        bool isHemVeto = hemVeto->isHemVeto(*skimT);
        if(isHemVeto){
            if(globalFlags_.isData()) continue;
            else weight *= hemVeto->getMcWeight();
        }
        if (globalFlags_.isMC()){
            auto phoSFs =  scalePhoton->getPhotonSfs(*skimT, pickedPhotons[0], 
                                                     ScalePhoton::SystLevel::Nominal);
            weight *= phoSFs.total;
        }
        if (iProbe < 0) continue;
        h1EventInCutflow->fill("passTagAndProbe");

        if (!pickEvent->passHltWithPt(skimT, ptTag)) continue; 
        const std::string passedHltName = pickEvent->getPassedHlt();
        weight *= hlt.getHltLumiWeight(passedHltName);
        histObjP4Tag.Fill(p4Tag, weight);

        h1EventInCutflow->fill("passHltWithPt", weight);

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

        // Jet veto map
        if (!pickEvent->passJetVetoMapOnProbe(p4Tag)) continue;
        if (!pickEvent->passJetVetoMapOnProbe(p4Probe)) continue;
        h1EventInCutflow->fill("passJetVetoMap", weight);

        h1EventInCutflow->fill("passL2Residual", weight);
        histScaleJet.Fill(*scaleJet);
        histScaleMet.Fill(*scaleMet);
        histScalePhoton.Fill(*scalePhoton);
        histL2ResidualInput.weight    = weight;
        histL2Residual.fillHistos(histL2ResidualInput);
        histPfCompProbeInProbe.Fill(skimT.get(), iProbe, ptProbe, etaProbe, weight);

        // Gen objects
        p4GenTag.SetPtEtaPhiM(0, 0, 0, 0);
        if (globalFlags_.isMC()) {
            double jesRelUnc = jecUncBand->getJesRelUncForBand(etaProbe, ptProbe);
            double jerRelUnc = jecUncBand->getJerSfRelUncForBand(etaProbe, "up");
            histJecUncBand.Fill(etaProbe, ptProbe, jesRelUnc, jerRelUnc);
            pickGamJet->pickGenPhotons(*skimT);
            pickGamJet->pickGenTags(*skimT, p4Tag);
            std::vector<TLorentzVector> p4GenTags = pickGamJet->getPickedGenTags();
            if (p4GenTags.empty()) continue;
            p4GenTag = p4GenTags.at(0);
            histTagJER.Fill(p4GenTag, p4Tag, skimT->Rho,  weight);
            auto p4GenProbe = pickGenJet->matchedP4GenJet(*skimT, p4Probe);
            histProbeJER.Fill(p4GenProbe, p4Probe, skimT->Rho,  weight);
        }
    } // end event loop
    h1EventInCutflow->printCutflow();
    h1EventInCutflow->fillFractionCutflow();
    std::cout << "Output file: " << fout->GetName() << "\n";
    fout->Write();
    return 0;
}

