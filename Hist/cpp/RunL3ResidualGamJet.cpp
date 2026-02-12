#include "RunL3ResidualGamJet.h"

#include "HistCutflow.h"
#include "HistL3ResidualInput.hpp"
#include "MathL3Residual.h"
#include "HistL3Residual.h"
#include "HistObjectVar.h"
#include "HistTime.h"
#include "HistAlpha.h"
#include "HistFlavor.h"
#include "HistObjP4.h"
#include "HistObjGenReco.h"
#include "HistPhotonCategory.h"
#include "HistPfComp.h"
#include "HistJecUncBand.h"
#include "HistScaleJet.h"
#include "HistScaleMet.h"
#include "HistScalePhoton.h"

#include "PickGamJet.h"
#include "PickGenJet.h"
#include "PickEvent.h"

#include "CategorizePhoton.h"
#include "ScalePhoton.h"
#include "ScaleJet.h"
#include "ScaleMet.h"
#include "JecUncBand.h"
#include "HemVeto.h"

#include "Helper.hpp"
#include "HelperDelta.hpp"
#include "VarBin.h"
#include "Hlt.h"
   
#include "ReadConfig.h"

// Constructor implementation
RunL3ResidualGamJet::RunL3ResidualGamJet(const GlobalFlag& globalFlags)
    :globalFlags_(globalFlags) {
    loadConfig("config/RunL3ResidualGamJet.json");
}

void RunL3ResidualGamJet::loadConfig(const std::string& filename) {
    ReadConfig config(filename);
    
    // Read the configuration values using getValue() for proper error handling.
    cutflows_          = config.getValue<std::vector<std::string>>({"cutflows"});
    minTagPts_         = config.getValue<std::vector<int>>({"minTagPts"});
    nPhotonMin_        = config.getValue<int>({"nPhotonMin"});
    maxDeltaPhiTagProbe_= config.getValue<double>({"maxDeltaPhiTagProbe"});
    maxAlpha_          = config.getValue<double>({"maxAlpha"});
    minPtJet2InAlpha_  = config.getValue<double>({"minPtJet2InAlpha"});
    alphaCuts_         = config.getValue<std::vector<double>>({"alphaCuts"});
    minResp_           = config.getValue<double>({"minResp"});
    maxResp_           = config.getValue<double>({"maxResp"});
}

auto RunL3ResidualGamJet::Run(std::shared_ptr<SkimTree>& skimT, ScaleEvent* scaleEvent,  TFile *fout) -> int{
   
    assert(fout && !fout->IsZombie());
    fout->mkdir("Base");
    fout->cd("Base");
 
    TDirectory *origDir = gDirectory;
    //------------------------------------
    // Initialise hists and directories 
    //------------------------------------
    
    auto h1EventInCutflow = std::make_unique<HistCutflow>(origDir, "", cutflows_, globalFlags_);
      
    // Variable binning
    VarBin varBin(globalFlags_);
    HistObjectVar histObjectVar(origDir, "passSkim", varBin);
    HistPhotonCategory histPhotonCat1(origDir, "passExactly1Tag", varBin, "Tag");
    HistAlpha histAlpha(origDir, "passDeltaPhiTagProbe", varBin, alphaCuts_);
    HistObjP4 histObjP4Probe(origDir, "passExactly1Probe", varBin, "Probe");
    HistObjP4 histObjP4Tag(origDir, "passExactly1Probe", varBin, "Tag");

    HistPhotonCategory histPhotonCat2(origDir, "passL3Residual", varBin, "Tag");
    HistObjGenReco histObjGenReco_Tag(origDir,   "passL3Residual", varBin, "Tag");
    HistObjGenReco histObjGenReco_Probe(origDir, "passL3Residual", varBin, "Probe");

    HistScaleJet    histScaleJet (origDir, "passL3Residual", varBin);
    HistScalePhoton    histScalePhoton (origDir, "passL3Residual", varBin);
    HistScaleMet    histScaleMet (origDir, "passL3Residual", varBin);
    HistPfComp histPfCompProbeInProbe(origDir, "passL3Residual", 
                                            varBin, "Probe", "InProbe");
    HistFlavor histFlavorProbe(origDir, "passL3Residual", varBin, "Probe");
    HistJecUncBand  histJecUncBand (origDir, "passL3Residual", varBin, "Probe");
    HistL3Residual  histL3Residual (origDir, "passL3Residual", varBin);
    HistTime histTime(origDir, "passL3Residual", varBin, minTagPts_);

    Hlt hlt(globalFlags_);

    auto pickGamJet = std::make_shared<PickGamJet>(globalFlags_);
    auto pickEvent = std::make_shared<PickEvent>(globalFlags_);

    // Scale constructor
    auto categorizePhoton = std::make_shared<CategorizePhoton>(globalFlags_);
    auto scalePhoton = std::make_shared<ScalePhoton>(globalFlags_);
    auto scaleJet  = std::make_shared<ScaleJet>(globalFlags_);
    auto pickGenJet= std::make_shared<PickGenJet>(globalFlags_);
    auto scaleMet  = std::make_shared<ScaleMet>(globalFlags_);
    auto jecUncBand  = std::make_shared<JecUncBand>(globalFlags_);
    auto mathL3Residual  = std::make_shared<MathL3Residual>(globalFlags_);
    auto hemVeto = std::make_shared<HemVeto>(globalFlags_);

    TLorentzVector p4RawTag, p4Tag, p4GenTag, p4CorrMet;
    TLorentzVector p4Probe, p4Jet2, p4Jetn;

    //------------------------------------
    // Event loop
    //------------------------------------

    double totalTime = 0.0;
    auto startClock = std::chrono::high_resolution_clock::now();
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

        if (!pickEvent->passMatchedGenVtx(*skimT)) continue;
        h1EventInCutflow->fill("passMatchedGenVtx", weight);

        histObjectVar.FillPhoton(*skimT, weight);
        histObjectVar.FillJet(*skimT, weight, 0);

        //------------------------------------------
        // Correct and select photon 
        //------------------------------------------
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
        p4RawTag = p4Tag;

        if (!pickEvent->passHltWithPt(skimT, ptTag)) continue; 
        const std::string passedHltName = pickEvent->getPassedHlt();
        weight *= hlt.getHltLumiWeight(passedHltName);
        h1EventInCutflow->fill("passHltWithPt", weight);

        auto catPhoton = categorizePhoton->categorize(*skimT, pickedPhotons.at(0));
        histPhotonCat1.Fill(ptTag, catPhoton, weight);

        //------------------------------------------------
        // Correct and select jets 
        //------------------------------------------------
        scaleJet->applyCorrection(skimT);
        pickGamJet->pickJets(*skimT, p4Tag);

        //Pick index of jets
        std::vector<int> jetsIndex = pickGamJet->getPickedJetsIndex();
        int iProbe = jetsIndex.at(0);
        int iJet2 = jetsIndex.at(1);
        if (iProbe==-1) continue; 
        h1EventInCutflow->fill("passExactly1Probe", weight);

        //Pick p4 of jets
        std::vector<TLorentzVector> jetsP4 = pickGamJet->getPickedJetsP4();
        p4Probe  = jetsP4.at(0);
        p4Jet2  = jetsP4.at(1);
        p4Jetn  = jetsP4.at(2);
        p4Jetn+= p4Jet2; // except leading jet

        double ptProbe = p4Probe.Pt();
        double etaProbe = p4Probe.Eta();
        histObjP4Tag.Fill(p4Tag, weight);
        histObjP4Probe.Fill(p4Probe, weight);

        //------------------------------------------------
        // Phi and Alpha cut
        //------------------------------------------------
        // Require Tag(=Z) and Probe (jet1) back-to-back
        double deltaPhi = HelperDelta::DELTAPHI(p4Tag.Phi(), p4Probe.Phi());
        if (fabs(deltaPhi - TMath::Pi()) >= maxDeltaPhiTagProbe_) continue; 
        h1EventInCutflow->fill("passDeltaPhiTagProbe", weight);

        // JVM cut
        if (!pickEvent->passJetVetoMapOnProbe(p4Tag)) continue; 
        if (!pickEvent->passJetVetoMapOnProbe(p4Probe)) continue; 
        h1EventInCutflow->fill("passJetVetoMap", weight);
        
        // Alpha cut
        double ptJet2 = p4Jet2.Pt();
        double alpha = ptJet2/ptTag;
        bool passAlpha = ( alpha  < maxAlpha_ || ptJet2 < minPtJet2InAlpha_); 

        //------------------------------------------------
        // Correct and select MET 
        //------------------------------------------------
        scaleMet->applyCorrection(skimT, scaleJet->getJetPtRaw());
        p4CorrMet = scaleMet->getP4CorrectedMet();
        // Replace PF Tag with Reco Tag
        p4CorrMet += p4RawTag - p4Tag; 

        // compute L3 inputs
        HistL3ResidualInput histL3ResidualInput = mathL3Residual->computeResponse(
                                                    p4Tag, p4Probe, 
                                                    p4CorrMet, p4Jetn);
        histAlpha.Fill(alpha, skimT->Rho, histL3ResidualInput);
        if(!passAlpha) continue;
        h1EventInCutflow->fill("passAlpha", weight);

        //apply response cut
        double bal = histL3ResidualInput.respDb; 
        double mpf = histL3ResidualInput.respMpf; 
        bool passDbResp  = bal > minResp_ && bal < maxResp_;
        bool passMpfResp = mpf > minResp_ && mpf < maxResp_;

        if (!(passDbResp && passMpfResp)) continue;
        h1EventInCutflow->fill("passL3Residual", weight);
        histScaleJet.Fill(*scaleJet);
        histScaleMet.Fill(*scaleMet);
        histScalePhoton.Fill(*scalePhoton);

        //------------------------------------------------
        // Event weights and histogram filling 
        //------------------------------------------------
        // HEM veto and compute weights
        if(hemVeto->isHemVeto(*skimT)){
            if(globalFlags_.isData()) continue;
            else weight *= hemVeto->getMcWeight();
        }
        if (globalFlags_.isMC()){
            auto phoSFs =  scalePhoton->getPhotonSfs(*skimT, pickedPhotons[0], 
                                                     ScalePhoton::SystLevel::Nominal);
            weight *= phoSFs.total;
        }
        histPhotonCat2.Fill(ptTag, catPhoton, weight);

        histPfCompProbeInProbe.Fill(skimT.get(), iProbe, ptProbe, p4Probe.Eta(), weight);
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
            auto pickedGenJets = pickGenJet->matchByRecoIndices(
                                *skimT, iProbe, iJet2, p4Probe, p4Jet2);
            double ptGenProbe =  pickedGenJets.p4GenJet1.Pt();
            double etaGenProbe =  pickedGenJets.p4GenJet1.Eta();
            histFlavorProbe.Fill(ptProbe, skimT->Jet_partonFlavour[iProbe], weight);
            histObjGenReco_Tag.Fill(p4GenTag.Pt(), ptTag, weight);
            histObjGenReco_Probe.Fill(ptGenProbe, ptProbe,weight);
        }
        else{
            histTime.Fill(skimT.get(), iProbe, bal, mpf, ptProbe, weight);
        }
        histL3ResidualInput.weight = weight;
        histL3Residual.fillHistos(histL3ResidualInput);

    }  // end of event loop

    h1EventInCutflow->printCutflow();
    h1EventInCutflow->fillFractionCutflow();
    std::cout << "Output file: " << fout->GetName() << '\n';
    fout->Write();
    return 0;
}
   
