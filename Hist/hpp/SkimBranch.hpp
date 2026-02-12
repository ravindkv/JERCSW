#pragma once

#include <RtypesCore.h>  // for Int_t, Float_t, Bool_t, UInt_t, etc.

// ------------------------------------------------------------------
// Compatible with Nano V9. Type-cast for other versions in SkimAdapter
// ------------------------------------------------------------------

// Pure data container for all branches 
// SkimTree will inherit from this to remain backwards-compatible.
class SkimBranch {
public:
    // ------------------------------------------------------------------
    // Event information
    // ------------------------------------------------------------------
    UInt_t    run{};
    UInt_t    luminosityBlock{};
    ULong64_t event{};
    UInt_t    bunchCrossing{};

    // Raw MET (starting point for Type-1 corrections)
    Float_t RawMET_phi{};
    Float_t RawMET_pt{};

    // Analysis MET after corrections (Type-1 MET)
    Float_t MET_phi{};
    Float_t MET_pt{};

    // ------------------------------------------------------------------
    // Jet variables 
    // ------------------------------------------------------------------
    static const int nJetMax = 200;

    UInt_t  nJet{};
    Float_t Jet_pt[nJetMax]{};
    Float_t Jet_eta[nJetMax]{};
    Float_t Jet_phi[nJetMax]{};
    Float_t Jet_mass[nJetMax]{};

    Float_t Jet_rawFactor[nJetMax]{};
    Float_t Jet_muonSubtrFactor[nJetMax]{};
    Float_t Jet_area[nJetMax]{};
    Int_t   Jet_jetId[nJetMax]{};

    Float_t Jet_btagDeepFlavB[nJetMax]{};
    Float_t Jet_btagDeepFlavCvL[nJetMax]{};
    Float_t Jet_btagDeepFlavCvB[nJetMax]{};
    Float_t Jet_btagDeepFlavG[nJetMax]{};
    Float_t Jet_btagDeepFlavQG[nJetMax]{};
    Float_t Jet_btagDeepFlavUDS[nJetMax]{};

    Float_t Jet_chHEF[nJetMax]{};
    Float_t Jet_neHEF[nJetMax]{};
    Float_t Jet_neEmEF[nJetMax]{};
    Float_t Jet_chEmEF[nJetMax]{};
    Float_t Jet_muEF[nJetMax]{};
    Short_t Jet_chMultiplicity[nJetMax]{};
    Short_t Jet_neMultiplicity[nJetMax]{};

    Int_t Jet_genJetIdx[nJetMax]{};
    Int_t Jet_muonIdx1[nJetMax]{};
    Int_t Jet_muonIdx2[nJetMax]{};
    Int_t Jet_electronIdx1[nJetMax]{};
    Int_t Jet_electronIdx2[nJetMax]{};
    Int_t Jet_partonFlavour[nJetMax]{};
    Int_t Jet_hadronFlavour[nJetMax]{};

    // ------------------------------------------------------------------
    // Photon variables
    // ------------------------------------------------------------------
    static const int nPhotonMax = 200;

    UInt_t  nPhoton{};
    Float_t Photon_pt[nPhotonMax]{};
    Float_t Photon_eta[nPhotonMax]{};
    Float_t Photon_phi[nPhotonMax]{};
    Float_t Photon_mass[nPhotonMax]{};
    Float_t Photon_hoe[nPhotonMax]{};
    Int_t   Photon_cutBased[nPhotonMax]{};
    Bool_t  Photon_mvaID_WP80[nPhotonMax]{};
    Int_t   Photon_jetIdx[nPhotonMax]{};
    Int_t   Photon_genPartIdx[nPhotonMax]{};
    Float_t Photon_r9[nPhotonMax]{};
    Float_t Photon_eCorr[nPhotonMax]{};
    Float_t Photon_energyErr[nPhotonMax]{};
    UChar_t Photon_seedGain[nPhotonMax]{};
    Bool_t  Photon_electronVeto[nPhotonMax]{};
    Bool_t  Photon_pixelSeed[nPhotonMax]{};

    // ------------------------------------------------------------------
    // Gen photon variables
    // ------------------------------------------------------------------
    UInt_t  nGenIsolatedPhoton{};
    Float_t GenIsolatedPhoton_pt[nPhotonMax]{};
    Float_t GenIsolatedPhoton_eta[nPhotonMax]{};
    Float_t GenIsolatedPhoton_phi[nPhotonMax]{};
    Float_t GenIsolatedPhoton_mass[nPhotonMax]{};

    // ------------------------------------------------------------------
    // Electron variables
    // ------------------------------------------------------------------
    static const int nEleMax = 150;

    UInt_t  nElectron{};
    Float_t Electron_phi[nEleMax]{};
    Float_t Electron_pt[nEleMax]{};
    Float_t Electron_eta[nEleMax]{};
    Float_t Electron_deltaEtaSC[nEleMax]{};
    Int_t   Electron_charge[nEleMax]{};
    Float_t Electron_mass[nEleMax]{};
    Float_t Electron_eCorr[nEleMax]{};
    Float_t Electron_seedGain[nEleMax]{};
    Int_t   Electron_cutBased[nEleMax]{};

    // ------------------------------------------------------------------
    // Gen lepton variables
    // ------------------------------------------------------------------
    UInt_t  nGenDressedLepton{};
    Float_t GenDressedLepton_pt[nEleMax]{};
    Float_t GenDressedLepton_phi[nEleMax]{};
    Float_t GenDressedLepton_mass[nEleMax]{};
    Float_t GenDressedLepton_eta[nEleMax]{};
    Int_t   GenDressedLepton_pdgId[nEleMax]{};

    // ------------------------------------------------------------------
    // Muon variables
    // ------------------------------------------------------------------
    static const int nMuonMax = 100;

    UInt_t  nMuon{};
    Int_t   Muon_nTrackerLayers[nMuonMax]{};
    Float_t Muon_phi[nMuonMax]{};
    Float_t Muon_pt[nMuonMax]{};
    Float_t Muon_eta[nMuonMax]{};
    Int_t   Muon_charge[nMuonMax]{};
    Float_t Muon_mass[nMuonMax]{};
    Float_t Muon_pfRelIso04_all[nMuonMax]{};
    Float_t Muon_tkRelIso[nMuonMax]{};
    Bool_t  Muon_mediumId[nMuonMax]{};
    Bool_t  Muon_tightId[nMuonMax]{};
    Bool_t  Muon_highPurity[nMuonMax]{};
    Float_t Muon_dxy[nMuonMax]{};
    Float_t Muon_dz[nMuonMax]{};

    // ------------------------------------------------------------------
    // Other variables
    // ------------------------------------------------------------------
    Float_t Rho{};

    Int_t   PV_npvs{};
    Int_t   PV_npvsGood{};
    Float_t PV_z{};
    Float_t GenVtx_z{};
    Float_t Pileup_pthatmax{};
    Float_t Generator_binvar{};

    // ------------------------------------------------------------------
    // MC-specific variables (GenJet)
    // ------------------------------------------------------------------
    static const int nGenJetMax = 200;

    UInt_t  nGenJet{};
    Float_t GenJet_eta[nGenJetMax]{};
    Float_t GenJet_mass[nGenJetMax]{};
    Float_t GenJet_phi[nGenJetMax]{};
    Float_t GenJet_pt[nGenJetMax]{};
    Int_t GenJet_partonFlavour[nGenJetMax]{};
    Int_t GenJet_hadronFlavour[nGenJetMax]{};

    // ------------------------------------------------------------------
    // MC-specific variables (GenPart)
    // ------------------------------------------------------------------
    static const int nGenPartMax = 1000;

    UInt_t  nGenPart{};
    Float_t GenPart_eta[nGenPartMax]{};
    Float_t GenPart_pt[nGenPartMax]{};
    Float_t GenPart_phi[nGenPartMax]{};
    Float_t GenPart_mass[nGenPartMax]{};
    Int_t   GenPart_genPartIdxMother[nGenPartMax]{};
    Int_t   GenPart_pdgId[nGenPartMax]{};
    Int_t   GenPart_status[nGenPartMax]{};
    Int_t   GenPart_statusFlags[nGenPartMax]{};

    // MC weights
    Float_t LHE_HT{};
    Float_t genWeight{};
    Float_t Pileup_nTrueInt{};
    UInt_t  nPSWeight{};
    static const int nPSWeightMax = 2000;
    Float_t PSWeight[nPSWeightMax]{};

    Float_t L1PreFiringWeight{};
    Float_t L1PreFiringWeight_Dn{};
    Float_t L1PreFiringWeight_Up{};
};

