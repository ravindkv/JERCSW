#include "RunL3ResidualGamJetFake.h"

#include "HistCutflow.h"
#include "HistFlavor.h"
#include "HistFlavorPair.h"
#include "HistL3ResidualInput.hpp"
#include "MathL3Residual.h"
#include "HistL3Residual.h"
#include "HistAlpha.h"
#include "HistObjGenRawReco.h"
#include "HistObjsGenRawReco.h"
#include "HistGamJetFake.h"

#include "PickGamJetFake.h"
#include "PickGenJet.h"
#include "PickEvent.h"

#include "ScaleJet.h"
#include "ScaleMet.h"
#include "HemVeto.h"
#include "ScalePhoton.h"

#include "Helper.hpp"
#include "HelperDelta.hpp"
#include "VarBin.h"
#include "MathL3Residual.h"


#include "ReadConfig.h"
   
// Constructor implementation
RunL3ResidualGamJetFake::RunL3ResidualGamJetFake(const GlobalFlag& globalFlags)
    :globalFlags_(globalFlags) {
    loadConfig("config/RunL3ResidualGamJetFake.json");
}

void RunL3ResidualGamJetFake::loadConfig(const std::string& filename) {
    ReadConfig config(filename);
    
    // Read the configuration values using getValue() for proper error handling.
    cutflows_          = config.getValue<std::vector<std::string>>({"cutflows"});
    minTagPts_         = config.getValue<std::vector<int>>({"minTagPts"});
    maxDeltaPhiTagProbe_= config.getValue<double>({"maxDeltaPhiTagProbe"});
    maxDeltaRgenJetPhoton_    = config.getValue<double>({"maxDeltaRgenJetPhoton"});
    maxAlpha_          = config.getValue<double>({"maxAlpha"});
    minPtJet2InAlpha_  = config.getValue<double>({"minPtJet2InAlpha"});
    alphaCuts_         = config.getValue<std::vector<double>>({"alphaCuts"});
    minTagPt_         = config.getValue<double>({"minTagPt"});
    maxTagEta_         = config.getValue<double>({"maxTagEta"});
    minResp_           = config.getValue<double>({"minResp"});
    maxResp_           = config.getValue<double>({"maxResp"});
}

auto RunL3ResidualGamJetFake::Run(std::shared_ptr<SkimTree>& skimT, ScaleEvent* scaleEvent,  TFile *fout) -> int{
   
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

    HistGamJetFake histGamJetFake(origDir, "passDeltaPhiTagProbe", varBin);

    HistObjGenRawReco histObjGenRawReco_Tag(origDir, "passDeltaPhiTagProbe", varBin, "Tag");
    HistObjGenRawReco histObjGenRawReco_Probe(origDir, "passDeltaPhiTagProbe", varBin, "Probe");
    HistObjsGenRawReco histObjsGenRawReco_TagProbe(origDir, "passDeltaPhiTagProbe", varBin, "Tag", "Probe");

    HistObjGenRawReco histObjGenRawReco_TagWhenPho(origDir, "passDPhiTagProbe_WhenPho", varBin, "Tag");
    HistObjGenRawReco histObjGenRawReco_ProbeWhenPho(origDir, "passDPhiTagProbe_WhenPho", varBin, "Probe");
    HistObjsGenRawReco histObjsGenRawReco_TagProbeWhenPho(origDir, "passDPhiTagProbe_WhenPho", varBin, "Tag", "Probe");
    HistObjsGenRawReco histObjsGenRawReco_TagTagPhoWhenPho(origDir, "passDPhiTagProbe_WhenPho", varBin, "Tag", "TagPho");
    HistObjGenRawReco histObjGenRawReco_TagPhoWhenPho(origDir, "passDPhiTagProbe_WhenPho", varBin, "TagPho");

    HistFlavor histFlavor_TagRecoJetInTagGenJetPt(origDir, "passDeltaPhiTagProbe", varBin,   "TagRecoJetInTagGenJetPt");
    HistFlavor histFlavor_TagRecoJetInProbeGenJetPt(origDir, "passDeltaPhiTagProbe", varBin,   "TagRecoJetInProbeGenJetPt");
    HistFlavor histFlavor_ProbeRecoJetProbeGenJetPt(origDir, "passDeltaPhiTagProbe", varBin, "ProbeRecoJetInProbeGenJetPt");
    HistFlavor histFlavor_ProbeRecoJetTagGenJetPt(origDir, "passDeltaPhiTagProbe", varBin, "ProbeRecoJetInTagGenJetPt");

    HistFlavor histFlavor_TagRecoJetInTagGenJetPt_WhenPho(origDir, "passDPhiTagProbe_WhenPho", varBin,   "TagRecoJetInTagGenJetPt");
    HistFlavor histFlavor_TagRecoJetInProbeGenJetPt_WhenPho(origDir, "passDPhiTagProbe_WhenPho", varBin,   "TagRecoJetInProbeGenJetPt");
    HistFlavor histFlavor_ProbeRecoJetProbeGenJetPt_WhenPho(origDir, "passDPhiTagProbe_WhenPho", varBin, "ProbeRecoJetInProbeGenJetPt");
    HistFlavor histFlavor_ProbeRecoJetTagGenJetPt_WhenPho(origDir, "passDPhiTagProbe_WhenPho", varBin, "ProbeRecoJetInTagGenJetPt");

    HistFlavorPair histFlavorPair_InTagGenJetPt(origDir, "passDeltaPhiTagProbe", varBin,   "TagAndProbeInTagGenJetPt");
    HistFlavorPair histFlavorPair_InProbeGenJetPt(origDir, "passDeltaPhiTagProbe", varBin,   "TagAndProbeInProbeGenJetPt");
    HistFlavorPair histFlavorPair_InTagGenJetPtWhenPho(origDir, "passDPhiTagProbe_WhenPho", varBin,   "TagAndProbeInTagGenJetPt");
    HistFlavorPair histFlavorPair_InProbeGenJetPtWhenPho(origDir, "passDPhiTagProbe_WhenPho", varBin,   "TagAndProbeInProbeGenJetPt");

    HistAlpha histAlpha(origDir, "passTagBarrel", varBin, alphaCuts_);
    HistL3Residual  histL3Residual (origDir, "passL3Residual", varBin);
    
    auto scalePhoton = std::make_shared<ScalePhoton>(globalFlags_);
    auto pickGamJetFake = std::make_shared<PickGamJetFake>(globalFlags_);
    auto pickGenJet= std::make_shared<PickGenJet>(globalFlags_);
    auto pickEvent = std::make_shared<PickEvent>(globalFlags_);
    auto scaleJet  = std::make_shared<ScaleJet>(globalFlags_);
    auto scaleMet  = std::make_shared<ScaleMet>(globalFlags_);
    auto mathL3Residual  = std::make_shared<MathL3Residual>(globalFlags_);
    auto hemVeto = std::make_shared<HemVeto>(globalFlags_);

    //------------------------------------
    // Event loop
    //------------------------------------
    TLorentzVector p4RawTag, p4GenTag;
    TLorentzVector p4Met1, p4Metn, p4Metu, p4Metnu;
    TLorentzVector p4Jeti, p4CorrMet;
    TLorentzVector p4GenJeti, p4GenProbe, p4GenJet2;
    MathHdm mathHdm(globalFlags_);

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
        //if (!pickEvent->passHlt(skimT)) continue; 
        h1EventInCutflow->fill("passHlt", weight);

        if (!pickEvent->passGoodLumi(skimT->run, skimT->luminosityBlock)) continue; 
        h1EventInCutflow->fill("passGoodLumi", weight);

        std::vector<double> rawJetPts = pickGamJetFake->getRawJetPts(*skimT);
        std::vector<double> rawPhoPts = pickGamJetFake->getRawPhoPts(*skimT);
 
        //------------------------------------------------
        // Apply jet energy scale
        //------------------------------------------------
        scaleJet->applyCorrection(skimT);
        scalePhoton->applyCorrections(skimT);

        //------------------------------------------------
        // Select jets 
        //------------------------------------------------
        if(skimT->nJet < 2) continue;
        pickGamJetFake->pickJets(*skimT);
        //Pick index of jets
        std::vector<int> jetsIndex = pickGamJetFake->getPickedJetsIndex();
        //Make sure the events have atleast 2 selected leading jets and they are 0th  and 1st
        if (jetsIndex.at(0) != 0) continue; 
        if (jetsIndex.at(1) != 1) continue; 
        h1EventInCutflow->fill("passAtleast2Jets");
        
        //Pick p4 of jets
        int iTag, iProbe, iJet2; 
        TLorentzVector p4Tag, p4Probe, p4Jet2, p4Jetn;
        std::vector<TLorentzVector> jetsP4 = pickGamJetFake->getPickedJetsP4();
        if(jentry%2 ==0){
            iTag = jetsIndex.at(0);
            p4Tag = jetsP4.at(0);
            iProbe = jetsIndex.at(1);
            p4Probe = jetsP4.at(1);
        }else{
            iTag = jetsIndex.at(1);
            p4Tag = jetsP4.at(1);
            iProbe = jetsIndex.at(0);
            p4Probe = jetsP4.at(0);
        }
        iJet2 = jetsIndex.at(2);
        p4Jet2  = jetsP4.at(2);
        p4Jetn  = jetsP4.at(3);
        p4Jetn+= p4Jet2; // except leading
    

        //------------------------------------------
        // Select objects
        //------------------------------------------
        TLorentzVector p4TagGenJet = pickGamJetFake-> getMatchedGenJetP4(*skimT, iTag);
        if (p4TagGenJet.Pt() <= 0.0) continue; 
        h1EventInCutflow->fill("passExactly1Tag");

        p4RawTag = p4TagGenJet; 
        double ptTagGenJet = p4TagGenJet.Pt();

        if (p4TagGenJet.Pt() < minTagPt_) continue; 
        if (fabs(p4TagGenJet.Eta()) > maxTagEta_) continue; 
        h1EventInCutflow->fill("passTagBarrel");

        // Gen objects
        p4GenTag.SetPtEtaPhiM(0, 0, 0, 0);
        p4GenTag = p4Tag;//dummy

        bool isHemVeto = hemVeto->isHemVeto(*skimT);
        if(isHemVeto){
            if(globalFlags_.isData()) continue;
            else weight *= hemVeto->getMcWeight();
        }

        if (!pickEvent->passJetVetoMap(*skimT)) continue; // expensive function
        h1EventInCutflow->fill("passJetVetoMap");
        
        double deltaPhi = HelperDelta::DELTAPHI(p4TagGenJet.Phi(), p4Probe.Phi());
        // Use maxDeltaPhiTagProbe_ from configuration
        if (fabs(deltaPhi - TMath::Pi()) >= maxDeltaPhiTagProbe_) continue; 
        h1EventInCutflow->fill("passDeltaPhiTagProbe");

        //------------------------------------------------
        // Set MET vectors
        //------------------------------------------------
        scaleMet->applyCorrection(skimT, scaleJet->getJetPtRaw());
        p4CorrMet = scaleMet->getP4CorrectedMet();

        // Propagate the gen-reco difference to MET since we use gen from now onward
        p4CorrMet -= (p4Tag - p4TagGenJet);

        TLorentzVector p4ProbeGenJet = pickGamJetFake->getMatchedGenJetP4(*skimT, iProbe);

        histObjGenRawReco_Tag.Fill(ptTagGenJet, rawJetPts[iTag], p4Tag.Pt(), weight);
        histObjGenRawReco_Probe.Fill(p4ProbeGenJet.Pt(), rawJetPts[iProbe], p4Probe.Pt(), weight);
        histObjsGenRawReco_TagProbe.Fill(ptTagGenJet, rawJetPts[iTag], p4Tag.Pt(),
                                               p4ProbeGenJet.Pt(), rawJetPts[iProbe], p4Probe.Pt(), 
                                               weight);

        histFlavor_TagRecoJetInTagGenJetPt.Fill(ptTagGenJet,  skimT->Jet_partonFlavour[iTag], weight);
        histFlavor_TagRecoJetInProbeGenJetPt.Fill(p4ProbeGenJet.Pt(),  skimT->Jet_partonFlavour[iTag], weight);
        histFlavor_ProbeRecoJetProbeGenJetPt.Fill(p4ProbeGenJet.Pt(),  skimT->Jet_partonFlavour[iProbe], weight);
        histFlavor_ProbeRecoJetTagGenJetPt.Fill(ptTagGenJet,  skimT->Jet_partonFlavour[iProbe], weight);

        histFlavorPair_InTagGenJetPt.Fill(ptTagGenJet,  
                        skimT->Jet_partonFlavour[iTag], skimT->Jet_partonFlavour[iProbe],weight);
        histFlavorPair_InProbeGenJetPt.Fill(p4ProbeGenJet.Pt(),  
                        skimT->Jet_partonFlavour[iTag], skimT->Jet_partonFlavour[iProbe],weight);

        //------------------------------------------------
        // For monitoring EM jet scale, flavour 
        //------------------------------------------------
        pickGamJetFake->pickPhotons(*skimT);
        std::vector<int> pickedPhotons = pickGamJetFake->getPickedPhotons();
        if(pickedPhotons.size() ==1){ 
            TLorentzVector p4Photon;
            int idx = pickedPhotons.at(0);
            p4Photon.SetPtEtaPhiM(skimT->Photon_pt[idx], skimT->Photon_eta[idx], 
                           skimT->Photon_phi[idx], skimT->Photon_mass[idx]);
            bool hasOrig = p4TagGenJet.DeltaR(p4Photon)< maxDeltaRgenJetPhoton_;
            if(hasOrig){
                auto matchedGenPhoP4 = pickGamJetFake->getMatchedGenPhotonP4(*skimT, idx);
                double matchedGenPhoPt = matchedGenPhoP4.Pt(); 
                if(matchedGenPhoPt > 0){
                    double ptRawTag = rawJetPts[iTag]; 
                    std::cout<<
                    " ptGenJet = "<<ptTagGenJet<<
                    ", ptRawJet = "<<ptRawTag<<
                    ", ptRecoJet = "<<p4Tag.Pt()<<
                    ", ptGenPhoton = "<<matchedGenPhoPt<<
                    ", ptPhoton = "<<p4Photon.Pt()<<'\n';
                    GamMatchMon mon;
                    mon.recoPt   = p4Photon.Pt();
                    mon.recoEta  = p4Photon.Eta();
                    mon.recoPhi  = p4Photon.Phi();
                    mon.r9       = skimT->Photon_r9[idx];
             
                    mon.dRmatch  = HelperDelta::DELTAR(matchedGenPhoP4.Phi(), mon.recoPhi, 
                                                  matchedGenPhoP4.Eta(), mon.recoEta);
                    mon.dEta     = matchedGenPhoP4.Eta() - mon.recoEta;
                    mon.dPhi     = HelperDelta::DELTAPHI(matchedGenPhoP4.Phi(), mon.recoPhi);
                    mon.relPtDiff= std::abs(matchedGenPhoP4.Pt() - mon.recoPt)/std::max(1.0, mon.recoPt);
             
                    //mon.R_core   = R_core;
                    //mon.R_adapt  = R_adapt;
                    //mon.eta_max  = eta_max;
                    //mon.phi_max  = phi_max;
             
                    //mon.nDauForBest = nDaughters; // -1 if not mother-based
                    mon.tag = GamMatchTag::SumSCwindow; // set appropriately
                    histGamJetFake.Fill(ptTagGenJet, p4Tag.Pt(), matchedGenPhoPt, p4Photon.Pt(), mon, weight);

                    histFlavorPair_InTagGenJetPtWhenPho.Fill(ptTagGenJet,  
                                    skimT->Jet_partonFlavour[iTag], skimT->Jet_partonFlavour[iProbe],weight);
                    histFlavorPair_InProbeGenJetPtWhenPho.Fill(p4ProbeGenJet.Pt(),  
                                    skimT->Jet_partonFlavour[iTag], skimT->Jet_partonFlavour[iProbe],weight);
                    
                    //Fill TnPs
                    histObjGenRawReco_TagWhenPho.Fill(ptTagGenJet, rawJetPts[iTag], p4Tag.Pt(), weight);
                    histObjGenRawReco_ProbeWhenPho.Fill(p4ProbeGenJet.Pt(), rawJetPts[iProbe], p4Probe.Pt(), weight);
                    histObjGenRawReco_TagPhoWhenPho.Fill(matchedGenPhoPt, rawPhoPts[idx], p4Photon.Pt(), weight);
                    histObjsGenRawReco_TagProbeWhenPho.Fill(ptTagGenJet, rawJetPts[iTag], p4Tag.Pt(),
                                               p4ProbeGenJet.Pt(), rawJetPts[iProbe], p4Probe.Pt(), 
                                               weight);
                    histObjsGenRawReco_TagTagPhoWhenPho.Fill(ptTagGenJet, rawJetPts[iTag], p4Tag.Pt(),
                                               matchedGenPhoPt, rawPhoPts[idx], p4Photon.Pt(),
                                               weight);

                    histFlavor_TagRecoJetInTagGenJetPt_WhenPho.Fill(ptTagGenJet,  skimT->Jet_partonFlavour[iTag], weight);
                    histFlavor_TagRecoJetInProbeGenJetPt_WhenPho.Fill(p4ProbeGenJet.Pt(),  skimT->Jet_partonFlavour[iTag], weight);
                    histFlavor_ProbeRecoJetProbeGenJetPt_WhenPho.Fill(p4ProbeGenJet.Pt(),  skimT->Jet_partonFlavour[iProbe], weight);
                    histFlavor_ProbeRecoJetTagGenJetPt_WhenPho.Fill(ptTagGenJet,  skimT->Jet_partonFlavour[iProbe], weight);
                }
            }
        }

        // GenJet
        auto pickedGenJets = pickGenJet->matchByRecoIndices(*skimT, iProbe, iJet2, p4Probe, p4Jet2);
 
        //------------------------------------------------
        // Compute inputs for histograms
        //------------------------------------------------
        HistL3ResidualInput histL3ResidualInput = mathL3Residual->computeResponse(
                                                    p4TagGenJet, p4Probe, 
                                                    p4CorrMet, p4Jetn);
        histL3ResidualInput.weight = weight;
        histL3Residual.fillHistos(histL3ResidualInput);

        double ptJet2 = p4Jet2.Pt();
        double alpha = ptJet2/ptTagGenJet;
        bool passAlpha = ( alpha  < maxAlpha_ || ptJet2 < minPtJet2InAlpha_); 
        histAlpha.Fill(alpha, skimT->Rho, histL3ResidualInput);
        if(!passAlpha) continue;
        h1EventInCutflow->fill("passAlpha", weight);
        
        double bal = histL3ResidualInput.respDb; 
        double mpf = histL3ResidualInput.respMpf; 
        bool passDbResp  = bal > minResp_ && bal < maxResp_;
        bool passMpfResp = mpf > minResp_ && mpf < maxResp_;

        if (!(passDbResp && passMpfResp)) continue;
        h1EventInCutflow->fill("passL3Residual", weight);

    }  // end of event loop

    h1EventInCutflow->printCutflow();
    h1EventInCutflow->fillFractionCutflow();
    std::cout << "Output file: " << fout->GetName() << '\n';
    fout->Write();
    return 0;
}
   
