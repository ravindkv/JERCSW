// cpp/SkimAdapter.cpp
#include "SkimAdapter.h"

#include <stdexcept>
#include <string>
#include <iostream>

// -------------------------------------------------------------
// Constructor
// -------------------------------------------------------------
SkimAdapter::SkimAdapter(GlobalFlag& flags, SkimBranch& branches)
    : globalFlags_(flags)
    , br_(branches)
    , year_(flags.getYear())
    , era_(flags.getEra())
    , channel_(flags.getChannel())
    , jetAlgo_(flags.getJetAlgo())
    , nanoVer_(flags.getNanoVersion())
    , debug_(flags.isDebug())
    , isData_(flags.isData())
    , isMC_(flags.isMC())
{
    isNanoV9_  = (nanoVer_ == GlobalFlag::NanoVersion::V9);
    isNanoV15_ = (nanoVer_ == GlobalFlag::NanoVersion::V15);

    // Channel-based feature flags
    enablePhoton_ =
        (channel_ == GlobalFlag::Channel::GamJet ||
         channel_ == GlobalFlag::Channel::GamJetFake);

    enableElectron_ =
        (channel_ == GlobalFlag::Channel::ZeeJet ||
         channel_ == GlobalFlag::Channel::Wqqe);

    enableMuon_ =
        (channel_ == GlobalFlag::Channel::ZmmJet ||
         channel_ == GlobalFlag::Channel::Wqqm);

    // B-tagging: keep enabled by default; can be refined later per channel
    enableBTag_ = true;
    enableGenJet_ = true;

    enableGenPart_  = false;
    enablePSWeight_ = false;
}

// ----------------------------------------------------------------------
// Common branches
// ----------------------------------------------------------------------
void SkimAdapter::setupCommonBranches(TChain* chain) {
    if (!chain) {
        throw std::runtime_error("SkimAdapter::setupCommonBranches - chain is null");
    }

    // Disable all branches; we will explicitly enable what we need
    chain->SetBranchStatus("*", 0);

    auto setScalar = [&](const char* name, void* addr) {
        chain->SetBranchStatus(name, 1);
        chain->SetBranchAddress(name, addr);
    };

    // Event-level info
    setScalar("run",             &br_.run);
    setScalar("luminosityBlock", &br_.luminosityBlock);
    setScalar("event",           &br_.event);

    // L1 prefiring
    if (year_==GlobalFlag::Year::Year2018){
        setScalar("L1PreFiringWeight_Nom", &br_.L1PreFiringWeight);
        setScalar("L1PreFiringWeight_Dn",  &br_.L1PreFiringWeight_Dn);
        setScalar("L1PreFiringWeight_Up",  &br_.L1PreFiringWeight_Up);
    }

    // Rho (version-dependent)
    if (isNanoV9_) {
        setScalar("fixedGridRhoFastjetAll", &br_.Rho);
    } else if (isNanoV15_) {
        setScalar("Rho_fixedGridRhoFastjetAll", &br_.Rho);
    } else {
        throw std::runtime_error("SkimAdapter::setupCommonBranches - unknown NanoVersion for Rho");
    }

    // PV position
    setScalar("PV_z", &br_.PV_z);
    setScalar("GenVtx_z", &br_.GenVtx_z); 

    // PV counters (version-dependent)
    if (isNanoV9_) {
        setScalar("PV_npvs",     &br_.PV_npvs);
        setScalar("PV_npvsGood", &br_.PV_npvsGood);
    } else if (isNanoV15_) {
        setScalar("PV_npvs",     &PV_npvs_v15_);
        setScalar("PV_npvsGood", &PV_npvsGood_v15_);
    } else {
        throw std::runtime_error("SkimAdapter::setupCommonBranches - unknown NanoVersion for PV_*");
    }
}

// ----------------------------------------------------------------------
// Jet branches
// ----------------------------------------------------------------------
void SkimAdapter::setupJetBranches(TChain* chain) {
    if (!chain) {
        throw std::runtime_error("SkimAdapter::setupJetBranches - chain is null");
    }

    std::string jetPrefix;
    std::string nJetBranch;

    switch (jetAlgo_) {
        case GlobalFlag::JetAlgo::AK4Chs:
        case GlobalFlag::JetAlgo::AK4Puppi:
            jetPrefix  = "Jet";
            nJetBranch = "nJet";
            break;
        case GlobalFlag::JetAlgo::AK8Puppi:
            jetPrefix  = "FatJet";
            nJetBranch = "nFatJet";
            break;
        default:
            throw std::runtime_error("SkimAdapter::setupJetBranches - unsupported JetAlgo");
    }
    auto setJetBranch = [&](const std::string& name, void* addr) {
        const std::string fullName = jetPrefix + "_" + name;
        chain->SetBranchStatus(fullName.c_str(), 1);
        chain->SetBranchAddress(fullName.c_str(), addr);
    };

    // Version-dependent: nJet + multiplicities + lepton indices
    if (isNanoV9_) {
        chain->SetBranchStatus(nJetBranch.c_str(), 1);
        chain->SetBranchAddress(nJetBranch.c_str(), &br_.nJet);

        setJetBranch("chMultiplicity", br_.Jet_chMultiplicity);
        setJetBranch("neMultiplicity", br_.Jet_neMultiplicity);

        if(enableMuon_){
            chain->SetBranchStatus("Jet_muonIdx1", 1);
            chain->SetBranchAddress("Jet_muonIdx1", br_.Jet_muonIdx1);
            chain->SetBranchStatus("Jet_muonIdx2", 1);
            chain->SetBranchAddress("Jet_muonIdx2", br_.Jet_muonIdx2);
        }

        if(enableElectron_){
            chain->SetBranchStatus("Jet_electronIdx1", 1);
            chain->SetBranchAddress("Jet_electronIdx1", br_.Jet_electronIdx1);
            chain->SetBranchStatus("Jet_electronIdx2", 1);
            chain->SetBranchAddress("Jet_electronIdx2", br_.Jet_electronIdx2);
        }
    } else if (isNanoV15_) {
        chain->SetBranchStatus(nJetBranch.c_str(), 1);
        chain->SetBranchAddress(nJetBranch.c_str(), &nJet_v15_);

        setJetBranch("chMultiplicity", Jet_chMultiplicity_v15_);
        setJetBranch("neMultiplicity", Jet_neMultiplicity_v15_);

        if(enableMuon_){
            chain->SetBranchStatus("Jet_muonIdx1", 1);
            chain->SetBranchAddress("Jet_muonIdx1", Jet_muonIdx1_v15_);
            chain->SetBranchStatus("Jet_muonIdx2", 1);
            chain->SetBranchAddress("Jet_muonIdx2", Jet_muonIdx2_v15_);
        }
        if(enableElectron_){
            chain->SetBranchStatus("Jet_electronIdx1", 1);
            chain->SetBranchAddress("Jet_electronIdx1", Jet_electronIdx1_v15_);
            chain->SetBranchStatus("Jet_electronIdx2", 1);
            chain->SetBranchAddress("Jet_electronIdx2", Jet_electronIdx2_v15_);
        }
    } else {
        throw std::runtime_error("SkimAdapter: unknown NanoVersion for Jet_*");
    }

    // Basic jet kinematics and properties (same for all versions)
    setJetBranch("area",            br_.Jet_area);

    if (enableBTag_) {
        setJetBranch("btagDeepFlavB",   br_.Jet_btagDeepFlavB);
        setJetBranch("btagDeepFlavCvL", br_.Jet_btagDeepFlavCvL);
        setJetBranch("btagDeepFlavCvB", br_.Jet_btagDeepFlavCvB);
        // setJetBranch("btagDeepFlavG",   br_.Jet_btagDeepFlavG);
        setJetBranch("btagDeepFlavQG",  br_.Jet_btagDeepFlavQG);
        // setJetBranch("btagDeepFlavUDS", br_.Jet_btagDeepFlavUDS);
    }

    setJetBranch("chEmEF",          br_.Jet_chEmEF);
    setJetBranch("chHEF",           br_.Jet_chHEF);
    setJetBranch("eta",             br_.Jet_eta);
    setJetBranch("mass",            br_.Jet_mass);
    setJetBranch("muEF",            br_.Jet_muEF);
    setJetBranch("neEmEF",          br_.Jet_neEmEF);
    setJetBranch("neHEF",           br_.Jet_neHEF);
    setJetBranch("phi",             br_.Jet_phi);
    setJetBranch("pt",              br_.Jet_pt);
    setJetBranch("rawFactor",       br_.Jet_rawFactor);
    setJetBranch("muonSubtrFactor", br_.Jet_muonSubtrFactor);
    if (isNanoV9_) {
        setJetBranch("jetId",           br_.Jet_jetId);
    }
}

// ----------------------------------------------------------------------
// Lepton branches
// ----------------------------------------------------------------------
void SkimAdapter::setupLeptonBranches(TChain* chain) {
    if (!chain) {
        throw std::runtime_error("SkimAdapter::setupLeptonBranches - chain is null");
    }

    auto setBranch = [&](const char* name, void* addr) {
        chain->SetBranchStatus(name, 1);
        chain->SetBranchAddress(name, addr);
    };

    // Photon (for GamJet)
    if (enablePhoton_) {
        if (isNanoV9_) {
            setBranch("nPhoton",         &br_.nPhoton);
            setBranch("Photon_cutBased",  br_.Photon_cutBased);
            setBranch("Photon_jetIdx",       br_.Photon_jetIdx);
        } else if (isNanoV15_) {
            setBranch("nPhoton",         &nPhoton_v15_);
            setBranch("Photon_cutBased",  Photon_cutBased_v15_);
            setBranch("Photon_jetIdx",       Photon_jetIdx_v15_);
        } else {
            throw std::runtime_error("SkimAdapter: unknown NanoVersion for Photon_*");
        }

        setBranch("Photon_eCorr",        br_.Photon_eCorr);
        setBranch("Photon_energyErr",    br_.Photon_energyErr);
        setBranch("Photon_eta",          br_.Photon_eta);
        setBranch("Photon_hoe",          br_.Photon_hoe);
        setBranch("Photon_mass",         br_.Photon_mass);
        setBranch("Photon_phi",          br_.Photon_phi);
        setBranch("Photon_pt",           br_.Photon_pt);
        setBranch("Photon_r9",           br_.Photon_r9);
        setBranch("Photon_mvaID_WP80",   br_.Photon_mvaID_WP80);
        setBranch("Photon_seedGain",     br_.Photon_seedGain);
        setBranch("Photon_pixelSeed",    br_.Photon_pixelSeed);
        setBranch("Photon_electronVeto", br_.Photon_electronVeto);
    }

    // Electron (for ZeeJet / Wqqe)
    if (enableElectron_) {
        if (isNanoV9_) {
            setBranch("nElectron",          &br_.nElectron);
            setBranch("Electron_cutBased",   br_.Electron_cutBased);
        } else if (isNanoV15_) {
            setBranch("nElectron",          &nElectron_v15_);
            setBranch("Electron_cutBased",   Electron_cutBased_v15_);
        } else {
            throw std::runtime_error("SkimAdapter: unknown NanoVersion for Electron_*");
        }

        setBranch("Electron_charge",     br_.Electron_charge);
        setBranch("Electron_pt",         br_.Electron_pt);
        setBranch("Electron_deltaEtaSC", br_.Electron_deltaEtaSC);
        setBranch("Electron_eta",        br_.Electron_eta);
        setBranch("Electron_phi",        br_.Electron_phi);
        setBranch("Electron_mass",       br_.Electron_mass);
        setBranch("Electron_eCorr",      br_.Electron_eCorr);
        setBranch("Electron_seedGain",      br_.Electron_seedGain);
    }

    // Muon (for ZmmJet / Wqqm)
    if (enableMuon_) {
        if (isNanoV9_) {
            setBranch("nMuon",               &br_.nMuon);
            setBranch("Muon_nTrackerLayers",  br_.Muon_nTrackerLayers);
        } else if (isNanoV15_) {
            setBranch("nMuon",               &nMuon_v15_);
            setBranch("Muon_nTrackerLayers",  Muon_nTrackerLayers_v15_);
        } else {
            throw std::runtime_error("SkimAdapter: unknown NanoVersion for Muon_*");
        }

        setBranch("Muon_charge",         br_.Muon_charge);
        setBranch("Muon_pt",             br_.Muon_pt);
        setBranch("Muon_eta",            br_.Muon_eta);
        setBranch("Muon_phi",            br_.Muon_phi);
        setBranch("Muon_mass",           br_.Muon_mass);
        setBranch("Muon_mediumId",       br_.Muon_mediumId);
        setBranch("Muon_tightId",        br_.Muon_tightId);
        setBranch("Muon_highPurity",     br_.Muon_highPurity);
        setBranch("Muon_pfRelIso04_all", br_.Muon_pfRelIso04_all);
        setBranch("Muon_tkRelIso",       br_.Muon_tkRelIso);
        setBranch("Muon_dxy",            br_.Muon_dxy);
        setBranch("Muon_dz",             br_.Muon_dz);
    }
}

// ----------------------------------------------------------------------
// MC branches
// ----------------------------------------------------------------------
void SkimAdapter::setupMCBranches(TChain* chain) {
    if (!chain) {
        throw std::runtime_error("SkimAdapter::setupMCBranches - chain is null");
    }

    if (!isMC_) {
        enableGenJet_   = false;
        enableGenPart_  = false;
        enablePSWeight_ = false;
        return;
    }

    auto setBranch = [&](const char* name, void* addr) {
        chain->SetBranchStatus(name, 1);
        chain->SetBranchAddress(name, addr);
    };

    std::string jetPrefix;
    std::string genJetPrefix;
    std::string nGenJetBranch;
    std::string genJetIndx;

    switch (jetAlgo_) {
        case GlobalFlag::JetAlgo::AK4Chs:
        case GlobalFlag::JetAlgo::AK4Puppi:
            jetPrefix     = "Jet";
            genJetPrefix  = "GenJet";
            nGenJetBranch = "nGenJet";
            genJetIndx    = "Jet_genJetIdx";
            break;
        case GlobalFlag::JetAlgo::AK8Puppi:
            jetPrefix     = "FatJet";
            genJetPrefix  = "GenJetAK8";
            nGenJetBranch = "nGenJetAK8";
            genJetIndx    = "FatJet_genJetAK8Idx";
            break;
        default:
            throw std::runtime_error("SkimAdapter::setupMCBranches - unsupported JetAlgo");
    }
    auto setJetBranch = [&](const std::string& name, void* addr) {
        const std::string fullName = jetPrefix + "_" + name;
        chain->SetBranchStatus(fullName.c_str(), 1);
        chain->SetBranchAddress(fullName.c_str(), addr);
    };

    auto setGenJetBranch = [&](const std::string& name, void* addr) {
        const std::string fullName = genJetPrefix + "_" + name;
        chain->SetBranchStatus(fullName.c_str(), 1);
        chain->SetBranchAddress(fullName.c_str(), addr);
    };
    // Weights / PU
    setBranch("genWeight",       &br_.genWeight);

    if (enablePSWeight_) {
        // may segfault if missing; behaviour same as original, but now optional
        setBranch("PSWeight",        br_.PSWeight);
    }

    setBranch("Pileup_nTrueInt", &br_.Pileup_nTrueInt);

    // GenJet
    if (enableGenJet_) {
        setGenJetBranch("eta",  br_.GenJet_eta);
        setGenJetBranch("mass", br_.GenJet_mass);
        setGenJetBranch("phi",  br_.GenJet_phi);
        setGenJetBranch("pt",   br_.GenJet_pt);
    }

    // GenPart
    if (enableGenPart_) {
        setBranch("GenPart_eta",   br_.GenPart_eta);
        setBranch("GenPart_mass",  br_.GenPart_mass);
        setBranch("GenPart_phi",   br_.GenPart_phi);
        setBranch("GenPart_pt",    br_.GenPart_pt);
        setBranch("GenPart_pdgId", br_.GenPart_pdgId);
        setBranch("GenPart_status", br_.GenPart_status);
    }

    setBranch("LHE_HT", &br_.LHE_HT);

    if (isNanoV9_) {
        if (enableGenJet_) {
            setBranch(nGenJetBranch.c_str(),   &br_.nGenJet);
            setJetBranch("partonFlavour", br_.Jet_partonFlavour);
            setJetBranch("hadronFlavour", br_.Jet_hadronFlavour);
            setGenJetBranch("partonFlavour",       br_.GenJet_partonFlavour);
            setGenJetBranch("hadronFlavour",       br_.GenJet_hadronFlavour);
            setBranch(genJetIndx.c_str(),     br_.Jet_genJetIdx);
        }
        if (enablePSWeight_) {
            setBranch("nPSWeight", &br_.nPSWeight);
        }
        if (enableGenPart_) {
            setBranch("nGenPart",  &br_.nGenPart);
            setBranch("GenPart_genPartIdxMother",   br_.GenPart_genPartIdxMother);
            setBranch("GenPart_statusFlags",        br_.GenPart_statusFlags);
        }

    } else if (isNanoV15_) {
        if (enableGenJet_) {
            setBranch(nGenJetBranch.c_str(),   &nGenJet_v15_);
            setJetBranch("partonFlavour",    Jet_partonFlavour_v15_);
            setJetBranch("hadronFlavour",    Jet_hadronFlavour_v15_);
            setGenJetBranch("partonFlavour",   GenJet_partonFlavour_v15_);
            setGenJetBranch("hadronFlavour",   GenJet_hadronFlavour_v15_);
            setBranch(genJetIndx.c_str(),       Jet_genJetIdx_v15_);
        }
        if (enablePSWeight_) {
            setBranch("nPSWeight", &nPSWeight_v15_);
        }
        if (enableGenPart_) {
            setBranch("nGenPart",  &nGenPart_v15_);
            setBranch("GenPart_genPartIdxMother",    GenPart_genPartIdxMother_v15_);
            setBranch("GenPart_statusFlags",         GenPart_statusFlags_v15_);
        }

    } else {
        throw std::runtime_error("SkimAdapter:  unknown NanoVersion for Jet_* flavour");
    }

    // Gen photons (for GamJet)
    if (enablePhoton_) {
        if (isNanoV9_) {
            setBranch("Photon_genPartIdx",   br_.Photon_genPartIdx);
            setBranch("nGenIsolatedPhoton",  &br_.nGenIsolatedPhoton);
        } else if (isNanoV15_) {
            setBranch("Photon_genPartIdx",   Photon_genPartIdx_v15_);
            setBranch("nGenIsolatedPhoton",  &nGenIsolatedPhoton_v15_);
        } else {
            throw std::runtime_error("SkimAdapter: unknown NanoVersion for Photon");
        }

        setBranch("GenIsolatedPhoton_eta",  br_.GenIsolatedPhoton_eta);
        setBranch("GenIsolatedPhoton_mass", br_.GenIsolatedPhoton_mass);
        setBranch("GenIsolatedPhoton_phi",  br_.GenIsolatedPhoton_phi);
        setBranch("GenIsolatedPhoton_pt",   br_.GenIsolatedPhoton_pt);
    }

    // Gen dressed leptons (for Z->ee/mm)
    if (enableElectron_ || enableMuon_){
        if (isNanoV9_) {
            setBranch("nGenDressedLepton", &br_.nGenDressedLepton);
        } else if (isNanoV15_) {
            setBranch("nGenDressedLepton", &nGenDressedLepton_v15_);
        } else {
            throw std::runtime_error("SkimAdapter: unknown NanoVersion for nGenDress");
        }

        setBranch("GenDressedLepton_eta",   br_.GenDressedLepton_eta);
        setBranch("GenDressedLepton_mass",  br_.GenDressedLepton_mass);
        setBranch("GenDressedLepton_phi",   br_.GenDressedLepton_phi);
        setBranch("GenDressedLepton_pt",    br_.GenDressedLepton_pt);
        setBranch("GenDressedLepton_pdgId", br_.GenDressedLepton_pdgId);
    }
}

void SkimAdapter::setupMetBranches(TChain* chain) {
    if (!chain) {
        throw std::runtime_error("SkimAdapter::setupMetBranches - chain is null");
    }

    std::string rawMetPtName;
    std::string rawMetPhiName;
    std::string metPtName;
    std::string metPhiName;

    switch (jetAlgo_) {
        case GlobalFlag::JetAlgo::AK4Chs:
            // Use CHS MET
            rawMetPtName  = "RawMET_pt";
            rawMetPhiName = "RawMET_phi";
            metPtName     = "MET_pt"; 
            metPhiName    = "MET_phi";
            break;

        case GlobalFlag::JetAlgo::AK4Puppi:
        case GlobalFlag::JetAlgo::AK8Puppi:
            // Use PUPPI MET, Nanov15
            rawMetPtName  = "RawPuppiMET_pt";
            rawMetPhiName = "RawPuppiMET_phi";
            metPtName     = "PuppiMET_pt";
            metPhiName    = "PuppiMET_phi";
            break;

        default:
            throw std::runtime_error("SkimAdapter::setupMetBranches - unsupported JetAlgo in MET aliasing");
    }

    auto setMetBranch = [&](const std::string& name, float* addr) {
        chain->SetBranchStatus(name.c_str(), 1);
        chain->SetBranchAddress(name.c_str(), addr);
    };

    setMetBranch(rawMetPhiName, &br_.RawMET_phi);
    setMetBranch(rawMetPtName,  &br_.RawMET_pt);
    setMetBranch(metPhiName,    &br_.MET_phi);
    setMetBranch(metPtName,     &br_.MET_pt);
}

// ----------------------------------------------------------------------
// After GetEntry: v15 → canonical v9-like representation
// ----------------------------------------------------------------------
void SkimAdapter::afterGetEntry() {
    if (!isNanoV15_) return;
    convertV15ToV9();
}

void SkimAdapter::convertV15ToV9() {
    // Scalars
    br_.PV_npvs            = static_cast<Int_t>(PV_npvs_v15_);
    br_.PV_npvsGood        = static_cast<Int_t>(PV_npvsGood_v15_);

    // Electrons
    if(enableElectron_){
        br_.nElectron          = static_cast<UInt_t>(nElectron_v15_);
        const std::size_t nEle = static_cast<std::size_t>(br_.nElectron);
        convertArray_(br_.Electron_cutBased,
                      Electron_cutBased_v15_, nEle);
    }

    // Muons
    if(enableMuon_){
        br_.nMuon              = static_cast<UInt_t>(nMuon_v15_);
        const std::size_t nMuon   = static_cast<std::size_t>(br_.nMuon);
        convertArray_(br_.Muon_nTrackerLayers,
                      Muon_nTrackerLayers_v15_, nMuon);
    }

    // Photons
    if(enablePhoton_){
        br_.nPhoton            = static_cast<UInt_t>(nPhoton_v15_);
        const std::size_t nPhoton = static_cast<std::size_t>(br_.nPhoton);
        convertArray_(br_.Photon_cutBased,
                      Photon_cutBased_v15_,   nPhoton);
        convertArray_(br_.Photon_jetIdx,
                      Photon_jetIdx_v15_,     nPhoton);
        convertArray_(br_.Photon_genPartIdx,
                      Photon_genPartIdx_v15_, nPhoton);

        br_.nGenIsolatedPhoton =
            static_cast<UInt_t>(nGenIsolatedPhoton_v15_);
    }

    // GenJet
    if (enableGenJet_){
        br_.nGenJet = static_cast<UInt_t>(nGenJet_v15_);
        const std::size_t nGenJet = static_cast<std::size_t>(br_.nGenJet);
        convertArray_(br_.GenJet_partonFlavour,
                      GenJet_partonFlavour_v15_, nGenJet);
        convertArray_(br_.GenJet_hadronFlavour,
                      GenJet_hadronFlavour_v15_, nGenJet);
    }

    // GenPart
    if (enableGenPart_){ 
        br_.nGenPart = static_cast<UInt_t>(nGenPart_v15_);
        const std::size_t nGenPart = static_cast<std::size_t>(br_.nGenPart);
        convertArray_(br_.GenPart_genPartIdxMother,
                      GenPart_genPartIdxMother_v15_, nGenPart);
        convertArray_(br_.GenPart_statusFlags,
                      GenPart_statusFlags_v15_,      nGenPart);
    }

    //PSWeight
    if (enablePSWeight_) 
        br_.nPSWeight = static_cast<UInt_t>(nPSWeight_v15_);

    //Jet
    br_.nJet               = static_cast<UInt_t>(nJet_v15_);
    const std::size_t nJet    = static_cast<std::size_t>(br_.nJet);
    if(isNanoV15_){
        convertArray_(br_.Jet_chMultiplicity, Jet_chMultiplicity_v15_, nJet);
        convertArray_(br_.Jet_neMultiplicity, Jet_neMultiplicity_v15_, nJet);
    }
    if(enableMuon_){
        convertArray_(br_.Jet_muonIdx1,       Jet_muonIdx1_v15_,       nJet);
        convertArray_(br_.Jet_muonIdx2,       Jet_muonIdx2_v15_,       nJet);
    }
    if(enableElectron_){
        convertArray_(br_.Jet_electronIdx1,   Jet_electronIdx1_v15_,   nJet);
        convertArray_(br_.Jet_electronIdx2,   Jet_electronIdx2_v15_,   nJet);
    }
    if(isMC_){
        convertArray_(br_.Jet_genJetIdx,     Jet_genJetIdx_v15_,     nJet);
        convertArray_(br_.Jet_partonFlavour, Jet_partonFlavour_v15_, nJet);
        convertArray_(br_.Jet_hadronFlavour, Jet_hadronFlavour_v15_, nJet);
    }

    //GenLepton
    if(enableElectron_ || enableMuon_)
        br_.nGenDressedLepton  = static_cast<UInt_t>(nGenDressedLepton_v15_);
}

void SkimAdapter::printDebug() const {
    std::cout << "\n[SkimAdapter] Configuration\n"
              << "  NanoVersion      : " 
              << (isNanoV9_ ? "V9" : (isNanoV15_ ? "V15" : "Unknown")) << "\n"
              << "  Year / Era       : " << static_cast<int>(year_) 
              << " / " << static_cast<int>(era_) << "\n"
              << "  Channel          : " << static_cast<int>(channel_) << "\n"
              << "  JetAlgo          : " << static_cast<int>(jetAlgo_) << "\n"
              << "  isData / isMC    : " << isData_ << " / " << isMC_ << "\n"
              << "  Debug            : " << debug_ << "\n"
              << "\n  Feature Flags:\n"
              << "    enablePhoton   = " << enablePhoton_   << "\n"
              << "    enableElectron = " << enableElectron_ << "\n"
              << "    enableMuon     = " << enableMuon_     << "\n"
              << "    enableBTag     = " << enableBTag_     << "\n"
              << "    enableGenJet   = " << enableGenJet_   << "\n"
              << "    enableGenPart  = " << enableGenPart_  << "\n"
              << "    enablePSWeight = " << enablePSWeight_ << "\n"
              << std::endl;
}

