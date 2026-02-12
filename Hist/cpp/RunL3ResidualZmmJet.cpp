#include "RunL3ResidualZmmJet.h"

#include "HistCutflow.h"
#include "HistL3ResidualInput.hpp"
#include "MathL3Residual.h"
#include "HistL3Residual.h"
#include "HistTime.h"
#include "HistAlpha.h"
#include "HistFlavor.h"
#include "HistObjP4.h"
#include "HistObjGenReco.h"
#include "HistPfComp.h"
#include "HistJecUncBand.h"

#include "PickZmmJet.h"
#include "PickGenJet.h"
#include "PickEvent.h"

#include "ScaleMuon.h"
#include "ScaleJet.h"
#include "ScaleMet.h"
#include "JecUncBand.h"
#include "HemVeto.h"

#include "Helper.hpp"
#include "HelperDelta.hpp"
#include "VarBin.h"

#include "ReadConfig.h"

// Constructor implementation: load configuration from JSON file
RunL3ResidualZmmJet::RunL3ResidualZmmJet(const GlobalFlag& globalFlags)
    : globalFlags_(globalFlags) {
    loadConfig("config/RunL3ResidualZmmJet.json");
}

void RunL3ResidualZmmJet::loadConfig(const std::string& filename) {
    ReadConfig config(filename);
    
    // Read the configuration values using getValue() for proper error handling.
    cutflows_          = config.getValue<std::vector<std::string>>({"cutflows"});
    minTagPts_         = config.getValue<std::vector<int>>({"minTagPts"});
    maxDeltaPhiTagProbe_= config.getValue<double>({"maxDeltaPhiTagProbe"});
    minPtJet2InAlpha_  = config.getValue<double>({"minPtJet2InAlpha"});
    maxAlpha_          = config.getValue<double>({"maxAlpha"});
    alphaCuts_         = config.getValue<std::vector<double>>({"alphaCuts"});
    minResp_           = config.getValue<double>({"minResp"});
    maxResp_           = config.getValue<double>({"maxResp"});
}

// Main run method updated to use configuration parameters
auto RunL3ResidualZmmJet::Run(std::shared_ptr<SkimTree>& skimT, ScaleEvent* scaleEvent, TFile *fout) -> int{

    assert(fout && !fout->IsZombie());
    fout->mkdir("Base");
    fout->cd("Base");
 
    TDirectory *origDir = gDirectory;
    //------------------------------------
    // Initialise histograms using config cutflows
    //------------------------------------
    
    auto h1EventInCutflow = std::make_unique<HistCutflow>(origDir, "", cutflows_, globalFlags_);
      
    // Variable binning
    VarBin varBin(globalFlags_);
    
    // Define histos in cutflows directories 
    HistAlpha histAlpha(origDir, "passDeltaPhiTagProbe", varBin, alphaCuts_);
    HistObjP4 histObjP4Lep1(origDir, "passAlpha", varBin, "LeadLep");
    HistObjP4 histObjP4Lep2(origDir, "passAlpha", varBin, "SubLeadLep");
    HistObjP4 histObjP4Probe(origDir, "passAlpha", varBin, "Probe");
    HistObjP4 histObjP4Tag(origDir, "passAlpha", varBin, "Tag");

    HistObjGenReco histObjGenReco_Tag(origDir,   "passL3Residual", varBin, "Tag");
    HistObjGenReco histObjGenReco_Probe(origDir, "passL3Residual", varBin, "Probe");

    HistPfComp histPfCompProbeInProbe(origDir, "passL3Residual", 
                                            varBin, "Probe", "InProbe");
    HistFlavor histFlavorProbe(origDir, "passL3Residual", varBin, "Probe");
    HistJecUncBand  histJecUncBand (origDir, "passL3Residual", varBin, "Probe");
    HistL3Residual  histL3Residual (origDir, "passL3Residual", varBin);
    HistTime histTime(origDir, "passL3Residual", varBin, minTagPts_);

    auto pickZmmJet = std::make_shared<PickZmmJet>(globalFlags_);
    auto pickEvent = std::make_shared<PickEvent>(globalFlags_);

    // Scale constructor
    auto scaleMuon = std::make_shared<ScaleMuon>(globalFlags_);
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
        //Double_t weight = scaleEvent->getEventWeight(*skimT);
        Double_t weight = 1.0;
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

        //------------------------------------------
        // Correct and select muons and Z-boson 
        //------------------------------------------
        scaleMuon->applyCorrections(skimT);

        pickZmmJet->pickMuons(*skimT);
        auto pickedMuons = pickZmmJet->getPickedMuons();
        pickZmmJet->pickTags(*skimT);
        std::vector<TLorentzVector> p4Tags = pickZmmJet->getPickedTags();
        if (p4Tags.size() != 1) continue; 
        h1EventInCutflow->fill("passExactly1Tag", weight);

        p4Tag = p4Tags.at(0);
        p4RawTag = p4Tags.at(0);
        double ptTag = p4Tag.Pt();

        //------------------------------------------------
        // Correct and select jets 
        //------------------------------------------------
        scaleJet->applyCorrection(skimT);
        pickZmmJet->pickJets(*skimT, p4Tag);

        // Pick index of jets
        std::vector<int> jetsIndex = pickZmmJet->getPickedJetsIndex();
        int iProbe = jetsIndex.at(0);
        int iJet2 = jetsIndex.at(1);
        if (iProbe == -1) continue; // make sure the events have a valid Probe jet
        h1EventInCutflow->fill("passExactly1Probe", weight);

        // Pick p4 of jets
        std::vector<TLorentzVector> jetsP4 = pickZmmJet->getPickedJetsP4();
        p4Probe = jetsP4.at(0);
        p4Jet2 = jetsP4.at(1);
        p4Jetn = jetsP4.at(2);
        p4Jetn += p4Jet2; // except Probe jet

        double ptProbe = p4Probe.Pt();
        double etaProbe = p4Probe.Eta();

        //------------------------------------------------
        // Phi and Alpha cut
        //------------------------------------------------
        // Require Tag(=Z) and Probe (jet1) back-to-back
        double deltaPhi = HelperDelta::DELTAPHI(p4Tag.Phi(), p4Probe.Phi());
        if (fabs(deltaPhi - TMath::Pi()) >= maxDeltaPhiTagProbe_) continue; 
        h1EventInCutflow->fill("passDeltaPhiTagProbe", weight);

        // JVM cut
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

        //------------------------------------------------
        // Event weights and histogram filling 
        //------------------------------------------------
        // HEM veto and compute weights
        if(hemVeto->isHemVeto(*skimT)){
            if(globalFlags_.isData()) continue;
            else weight *= hemVeto->getMcWeight();
        }
        if (globalFlags_.isMC()){
            auto muSFs1 =  scaleMuon->getMuonSfs(*skimT, pickedMuons[0], 
                                                     ScaleMuon::SystLevel::Nominal);
            auto muSFs2 =  scaleMuon->getMuonSfs(*skimT, pickedMuons[1], 
                                                     ScaleMuon::SystLevel::Nominal);
            weight *= muSFs1.total;
            weight *= muSFs2.total;
        }
        histL3ResidualInput.weight = weight;
        histL3Residual.fillHistos(histL3ResidualInput);

        // Lepton hist
        TLorentzVector p4Lep1, p4Lep2;
        int iLep1, iLep2;
        iLep1 = pickedMuons.at(0);
        iLep2 = pickedMuons.at(1);
        p4Lep1.SetPtEtaPhiM(skimT->Muon_pt[iLep1], skimT->Muon_eta[iLep1], 
                            skimT->Muon_phi[iLep1], skimT->Muon_mass[iLep1]);
        p4Lep2.SetPtEtaPhiM(skimT->Muon_pt[iLep2], skimT->Muon_eta[iLep2], 
                            skimT->Muon_phi[iLep2], skimT->Muon_mass[iLep2]);
        histObjP4Lep1.Fill(p4Lep1, weight);
        histObjP4Lep2.Fill(p4Lep2, weight);
        histObjP4Probe.Fill(p4Probe, weight);
        histObjP4Tag.Fill(p4Tag, weight);

        histPfCompProbeInProbe.Fill(skimT.get(), iProbe, ptProbe, etaProbe, weight);
        // Gen objects
        p4GenTag.SetPtEtaPhiM(0, 0, 0, 0);
        if (globalFlags_.isMC()) {
            double jesRelUnc = jecUncBand->getJesRelUncForBand(etaProbe, ptProbe);
            double jerRelUnc = jecUncBand->getJerSfRelUncForBand(etaProbe, "up");
            histJecUncBand.Fill(etaProbe, ptProbe, jesRelUnc, jerRelUnc);
            pickZmmJet->pickGenMuons(*skimT);
            pickZmmJet->pickGenTags(*skimT, p4Tag);
            std::vector<TLorentzVector> p4GenTags = pickZmmJet->getPickedGenTags();
            if (p4GenTags.empty()) continue;
            p4GenTag = p4GenTags.at(0);
            auto pickedGenJets = pickGenJet->matchByRecoIndices(
                                *skimT, iProbe, iJet2, p4Probe, p4Jet2);
            double ptGenProbe  =  pickedGenJets.p4GenJet1.Pt();
            double etaGenProbe =  pickedGenJets.p4GenJet1.Eta();
            histFlavorProbe.Fill(ptProbe, skimT->Jet_partonFlavour[iProbe], weight);
            histObjGenReco_Tag.Fill(p4GenTag.Pt(), ptTag, weight);
            histObjGenReco_Probe.Fill(ptGenProbe, ptProbe,weight);
        }
        else{
            histTime.Fill(skimT.get(), iProbe, bal, mpf, ptProbe, weight);
        }
        
    }  // end of event loop

    h1EventInCutflow->printCutflow();
    h1EventInCutflow->fillFractionCutflow();
    std::cout << "Output file: " << fout->GetName() << '\n';
    fout->Write();
    return 0;
}

