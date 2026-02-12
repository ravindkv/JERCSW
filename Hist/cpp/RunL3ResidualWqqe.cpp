#include "RunL3ResidualWqqe.h"

#include "HistCutflow.h"
#include "HistHadW.h"
#include "HistTTbar.h"

#include "ScaleElectron.h"
#include "ScaleJet.h"
#include "ScaleMet.h"
#include "ScaleBtag.h"
#include "HemVeto.h"
#include "PickWqqe.h"
#include "PickEvent.h"

#include "Helper.hpp"
#include "VarBin.h"
#include "MathHadW.h"
#include "MathTTbar.h"
#include "ReadConfig.h"

#include <chrono>
#include <iostream>

RunL3ResidualWqqe::RunL3ResidualWqqe(const GlobalFlag& globalFlags)
  : globalFlags_(globalFlags)
{
    loadConfig("config/RunL3ResidualWqqe.json");
}

void RunL3ResidualWqqe::loadConfig(const std::string& filename) {
    ReadConfig cfg(filename);

    // cutflow steps
    cutflows_ = cfg.getValue<std::vector<std::string>>({"cutflows"});

    massW_  = cfg.getValue<double>({"massW"});
    massT_  = cfg.getValue<double>({"massT"});
    massEle_  = cfg.getValue<double>({"massEle"});
    maxChi2_  = cfg.getValue<int>({"maxChi2"});
    minMet_  = cfg.getValue<int>({"minMet"});

    nJetMin_  = cfg.getValue<int>({"nJetMin"});
    maxEtaW_  = cfg.getValue<double>({"maxEtaW"});
    minPtW_  = cfg.getValue<double>({"minPtW"});

    // b‑Jet cuts
    std::string yearStr = globalFlags_.getYearStr();
    minBJetDisc_      = cfg.getValue<double>({yearStr, "minBJetDisc"});
    nBJetMin_  = cfg.getValue<int>({"nBJetMin"});


    // resolution & smearing parameters
    resLep_  = cfg.getValue<double>({"resLep"});
    resMet_  = cfg.getValue<double>({"resMet"});
    resHadW_ = cfg.getValue<double>({"resHadW"});
    resHadT_ = cfg.getValue<double>({"resHadT"});
    resLepT_ = cfg.getValue<double>({"resLepT"});
    useReso_ = cfg.getValue<bool>   ({"useReso"});
}

auto RunL3ResidualWqqe::Run(std::shared_ptr<SkimTree>& skimT,
                  ScaleEvent*             scaleEvent,
                  TFile*                  fout) -> int
{
    assert(fout && !fout->IsZombie());
    fout->mkdir("Base");
    fout->cd("Base");
    TDirectory* origDir = gDirectory;

    //------------------------------------
    // 1) Initialise histograms & dirs
    //------------------------------------
    auto h1EventInCutflow = std::make_unique<HistCutflow>(
        origDir, "", cutflows_, globalFlags_);

    VarBin varBin(globalFlags_);

    HistHadW   histHadW  (origDir, "passMaxChiSqr", varBin);
    HistTTbar  histTTbar (origDir, "passMaxChiSqr", varBin);

    MathHadW  mathHadW  (globalFlags_, minBJetDisc_, massW_, resHadW_);
    MathTTbar mathTTbar (globalFlags_, minBJetDisc_);
    mathTTbar.setMass(massEle_, massW_, massT_);

    auto scaleElectron   = std::make_shared<ScaleElectron>(globalFlags_);
    auto scaleJet  = std::make_shared<ScaleJet>(globalFlags_);
    auto scaleMet  = std::make_shared<ScaleMet>(globalFlags_);
    auto scaleBtag = std::make_shared<ScaleBtag>(globalFlags_);
    auto hemVeto = std::make_shared<HemVeto>(globalFlags_);

    auto pickWqqe = std::make_shared<PickWqqe>(globalFlags_);
    auto pickEvent = std::make_shared<PickEvent>(globalFlags_);

    //------------------------------------
    // 2) Event loop
    //------------------------------------
    TLorentzVector p4CorrMet;
    double totalTime = 0.0;
    auto startClock  = std::chrono::high_resolution_clock::now();
    Long64_t nentries = skimT->getEntries();
    const Long64_t everyN = Helper::initProgress(nentries);

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

        // Apply Electron Corrections 
        scaleElectron->applyCorrections(skimT);

        // --- Electron selection ---
        pickWqqe->pickElectrons(*skimT);
        auto pickedElectrons = pickWqqe->getPickedElectrons();
        if (pickedElectrons.size() != 1) continue;
        int eleInd = pickedElectrons[0];
        TLorentzVector p4Lep;
        p4Lep.SetPtEtaPhiM(
            skimT->Electron_pt[eleInd],
            skimT->Electron_eta[eleInd],
            skimT->Electron_phi[eleInd],
            skimT->Electron_mass[eleInd]
        );
        h1EventInCutflow->fill("passExactly1Lep", weight);

        // --- JES/JER ---
        scaleJet->applyCorrection(skimT);

        // --- Jet / b‑jet selection ---
        pickWqqe->pickJets(*skimT, minBJetDisc_);
        auto indexJets   = pickWqqe->getPickedJets();
        auto indexBjets  = pickWqqe->getPickedBJets();

        if (indexJets.size() < static_cast<size_t>(nJetMin_)) continue;
        h1EventInCutflow->fill("passAtleast4Jet", weight);

        if (indexBjets.size() < static_cast<size_t>(nBJetMin_))
            continue;
        h1EventInCutflow->fill("passAtleast2bJet", weight);
        h1EventInCutflow->fill("passJetVetoMap", weight);

        bool isHemVeto = hemVeto->isHemVeto(*skimT);
        if(isHemVeto){
            if(globalFlags_.isData()) continue;
            else weight *= hemVeto->getMcWeight();
        }
        if (globalFlags_.isMC()){
            auto eleSFs =  scaleElectron->getElectronSfs(*skimT, pickedElectrons[0], 
                                                     ScaleElectron::SystLevel::Nominal);
            weight *= eleSFs.total;
        }   

        scaleMet->applyCorrection(skimT, scaleJet->getJetPtRaw());
        p4CorrMet = scaleMet->getP4CorrectedMet();
        if(p4CorrMet.Pt() < minMet_) continue;
        h1EventInCutflow->fill("passMinMet30", weight);

        /*
        // --- W→qq analysis ---
        auto p4HadW = mathHadW.getP4HadW(*skimT, indexJets);
        int i1W     = mathHadW.getIndex1ForW();
        int i2W     = mathHadW.getIndex2ForW();
        double chiW = mathHadW.getChiSqr();
        if (i1W >= 0 && i2W >= 0 &&
            std::abs(p4HadW.Eta()) < maxEtaW_ &&
            p4HadW.Pt() > minPtW_ &&
            chiW < maxChi2_ )
        {
            double avgPt = 0.5f * (skimT->Jet_pt[i1W] + skimT->Jet_pt[i2W]);
            histHadW.Fill(p4HadW, avgPt, chiW, skimT->run, weight);
        }
        */

        // --- t𝑡̄ reconstruction ---
        mathTTbar.setEventObjects(p4Lep, p4CorrMet, indexJets);
        mathTTbar.setResolution(
            resLep_, resMet_, resHadW_, resHadT_, resLepT_, /*extra=*/{}, useReso_);
        mathTTbar.minimizeChiSqr(*skimT);

        int i1T = mathTTbar.getIndex1ForW();
        int i2T = mathTTbar.getIndex2ForW();
        double chiT = mathTTbar.getChiSqr();

        const auto& p4HadWfromTT =  mathTTbar.getP4HadW();
        if(std::abs(p4HadWfromTT.Eta()) > maxEtaW_) continue;
        h1EventInCutflow->fill("passWmaxEta", weight);
        if(p4HadWfromTT.Pt()  < minPtW_) continue;
        h1EventInCutflow->fill("passWminPt", weight);
        if(chiT > maxChi2_) continue;
        h1EventInCutflow->fill("passMaxChiSqr", weight);
        
        if (i1T >= 0 && i2T >= 0)
        {
            double avgPtT = 0.5f * (skimT->Jet_pt[i1T] + skimT->Jet_pt[i2T]);
            histTTbar.FillAll(
                mathTTbar.getP4HadW(),
                mathTTbar.getP4HadT(),
                mathTTbar.getP4LepT(),
                mathTTbar.getP4HadB(),
                mathTTbar.getP4Jet1(),
                mathTTbar.getP4Jet2(),
                mathTTbar.getP4LepB(),
                mathTTbar.getP4Lep(),
                mathTTbar.getP4FullMet(),
                avgPtT,
                chiT,
                skimT->run,
                weight
            );
        }
    } // end event loop

    //------------------------------------
    // 3) Finalize
    //------------------------------------
    h1EventInCutflow->printCutflow();
    h1EventInCutflow->fillFractionCutflow();
    std::cout << "Output file: " << fout->GetName() << "\n";
    fout->Write();
    return 0;
}

