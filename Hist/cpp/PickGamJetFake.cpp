#include "PickGamJetFake.h"
#include "ReadConfig.h"
#include "HelperDelta.hpp"

// Constructor implementation
PickGamJetFake::PickGamJetFake(const GlobalFlag& globalFlags) :
    globalFlags_(globalFlags),
    year_(globalFlags_.getYear()),
    channel_(globalFlags_.getChannel()),
    isDebug_(globalFlags_.isDebug())
{
    loadConfig("config/PickGamJetFake.json");
}

// Destructor
PickGamJetFake::~PickGamJetFake() {
    // Cleanup if necessary
}

// Load configuration from JSON file and store values in private members
void PickGamJetFake::loadConfig(const std::string& filename) {
    ReadConfig config(filename);

    // Photon pick configuration
    minPtPho_       = config.getValue<double>({"photonPick", "minPt"});
    maxEtaPho_      = config.getValue<double>({"photonPick", "maxEta"});
    tightIdPho_     = config.getValue<int>({"photonPick", "tightId"});
    //mvaIdWp80Pho_   = config.getValue<bool>({"photonPick", "mvaIdWp80"});
    maxR9Pho_       = config.getValue<double>({"photonPick", "maxR9"});
    minR9Pho_       = config.getValue<double>({"photonPick", "minR9"});
    maxHoePho_      = config.getValue<double>({"photonPick", "maxHoe"});
    isEleVetoPho_   = config.getValue<bool>({"photonPick", "isEleVeto"});
    hasPixelSeedPho_= config.getValue<bool>({"photonPick", "hasPixelSeed"});

    // Jet pick configuration
    minPtJet_           = config.getValue<double>({"jetPick", "minPt"});
    minPtOther_         = config.getValue<double>({"jetPick","minPtOther"});
    maxEtaJet_          = config.getValue<double>({"jetPick", "maxEta"});

    // Gen Jet pick configuration
    minPtGenJet_           = config.getValue<double>({"genJetPick", "minPt"});
    maxEtaGenJet_          = config.getValue<double>({"genJetPick", "maxEta"});
    maxDeltaRgenJet_       = config.getValue<double>({"genJetPick", "maxDeltaR"});

    maxDeltaRgenPho_ = config.getValue<double>({"genPhoPick", "maxDeltaR"});
}

// HelperDelta function for debug printing
void PickGamJetFake::printDebug(const std::string& message) const {
    if (isDebug_) {
        std::cout << message << '\n';
    }
}

void PickGamJetFake::pickJets(const SkimTree& skimT) {
    printDebug("pickJetsForFakeGamma: Starting Selection, nJet = " + std::to_string(skimT.nJet));

    pickedJetsIndex_.clear();
    pickedJetsP4_.clear();

    //-----------------------------------------
    // 1) Gather candidate jet indices 
    //    (pass minimal pT, eta),
    //-----------------------------------------
    std::vector<int> candIndices;
    candIndices.reserve(skimT.nJet);

    for (int i = 0; i < skimT.nJet; ++i) {
        float pt = skimT.Jet_pt[i];
        if (pt < minPtJet_) {
            continue; 
        }
        if (std::abs(skimT.Jet_eta[i]) >= maxEtaJet_) {
            continue; 
        }
        candIndices.push_back(i);
    }

    //-----------------------------------------
    // 2) Sort the candidate jet indices by pT
    //    in descending order
    //-----------------------------------------
    std::sort(candIndices.begin(), candIndices.end(),
              [&](int idx1, int idx2) {
                  return skimT.Jet_pt[idx1] > skimT.Jet_pt[idx2];
              });

    //-----------------------------------------
    // 3) Pick the leading three jets
    //    (if they exist)
    //-----------------------------------------
    int iJet1 = -1;
    int iJet2 = -1;
    int iJet3 = -1;

    if (!candIndices.empty()) {
        iJet1 = candIndices[0];
    }
    if (candIndices.size() > 1) {
        iJet2 = candIndices[1];
    }
    if (candIndices.size() > 2) {
        iJet3 = candIndices[2];
    }

    printDebug("After picking top-3 pT jets: iJet1 = " + std::to_string(iJet1) +
                ", iJet2 = " + std::to_string(iJet2) +
                ", iJet3 = " + std::to_string(iJet3));

    //-----------------------------------------
    // 5) Store the final picked jet indices
    //-----------------------------------------
    pickedJetsIndex_.push_back(iJet1);
    pickedJetsIndex_.push_back(iJet2);
    pickedJetsIndex_.push_back(iJet3);

    //-----------------------------------------
    // 6) Build the four-vectors: leading jet,
    //    subleading jet, and sum of the rest
    //-----------------------------------------
    TLorentzVector p4Jet1(0, 0, 0, 0);
    TLorentzVector p4Jet2(0, 0, 0, 0);
    TLorentzVector p4Jet3(0, 0, 0, 0);
    TLorentzVector p4Jetn(0, 0, 0, 0);

    for (int i = 0; i < skimT.nJet; ++i) {
        TLorentzVector p4Jeti;
        p4Jeti.SetPtEtaPhiM(skimT.Jet_pt[i],
                            skimT.Jet_eta[i],
                            skimT.Jet_phi[i],
                            skimT.Jet_mass[i]);

        // If this is the chosen leading or subleading jet, store individually
        //https://github.com/blehtela/gamjet-analysis/blob/919921427e6a1ec96df677bd774f0f819a19d108/GamHistosFill.C#L2582
        // QCD_CP5 has about 2.5 GeV/A of UE offset at FullSim level
        if (i == iJet1) {
            p4Jet1 = p4Jeti;
    	    p4Jet1 *= std::max(0.0, 1 - 2.5*skimT.Jet_area[iJet1]/p4Jet1.Pt());
        } 
        else if (i == iJet2) {
            p4Jet2 = p4Jeti;
    	    p4Jet2 *= std::max(0.0, 1 - 2.5*skimT.Jet_area[iJet2]/p4Jet2.Pt());
        } 
        else if (i == iJet3) {
            p4Jet3 = p4Jeti;
    	    p4Jet3 *= std::max(0.0, 1 - 2.5*skimT.Jet_area[iJet3]/p4Jet3.Pt());
        } 
        // Otherwise, accumulate in p4Jetn
        else {
            if(p4Jeti.Pt()> minPtOther_){
    	        p4Jeti *= std::max(0.0, 1 - 2.5*skimT.Jet_area[i]/p4Jeti.Pt());
                p4Jetn += p4Jeti;
            }
        }
    }

    pickedJetsP4_.push_back(p4Jet1);
    pickedJetsP4_.push_back(p4Jet2);
    pickedJetsP4_.push_back(p4Jet3);
    pickedJetsP4_.push_back(p4Jetn);

    printDebug("pickJetsForFakeGamma: Done.");
}


// Reference object picking 
TLorentzVector PickGamJetFake::getMatchedGenJetP4(const SkimTree& skimT, const int& iJet) {
    printDebug("\n pickRef: Starting Selection");
    TLorentzVector p4GenJet;
    if (iJet < 0 || iJet >= skimT.nJet) return p4GenJet;
    int iGenJet = skimT.Jet_genJetIdx[iJet];
    if (iGenJet >=0 && iGenJet < skimT.nGenJet) {
        p4GenJet.SetPtEtaPhiM(skimT.GenJet_pt[iGenJet], skimT.GenJet_eta[iGenJet], 
                             skimT.GenJet_phi[iGenJet], skimT.GenJet_mass[iGenJet]);
        // QCD_CP5 has about 3.5 GeV/A of UE offset at generator level
        // https://github.com/blehtela/gamjet-analysis/blob/919921427e6a1ec96df677bd774f0f819a19d108/GamHistosFill.C#L2562
        double pt = p4GenJet.Pt();
        double absEta = std::abs(p4GenJet.Eta());
        double offset = 1.0;
        if (pt > 0 ){
            offset = std::max(0.0, 1 - 3.5*0.5/pt); //0.5 is the idealstic genJet area
            p4GenJet *= offset; 
        }
        TLorentzVector p4Jet;
        p4Jet.SetPtEtaPhiM(skimT.Jet_pt[iJet], skimT.Jet_eta[iJet], 
                          skimT.Jet_phi[iJet], skimT.Jet_mass[iJet]);
        if (p4GenJet.DeltaR(p4Jet) < maxDeltaRgenJet_ &&  
            p4GenJet.Pt() > minPtGenJet_ && 
            std::abs(p4GenJet.Eta()) < maxEtaGenJet_ ) {
            printDebug("Jet index added to reference = " + std::to_string(iJet));
            printDebug("GenJet index added to reference = " + std::to_string(iGenJet));
            printDebug("Reco pt = " + std::to_string(skimT.Jet_pt[iJet]));
            printDebug("GenJet pt = " + std::to_string(pt));
            printDebug("UE offset = " + std::to_string(offset));
        }
    }
    printDebug("pickRef: Done.\n");
    return p4GenJet;
}


//For monitoring and scale of EM jet: Photon selection
void PickGamJetFake::pickPhotons(const SkimTree& skimT) {
    printDebug("Starting Selection, nPhoton = "+std::to_string(skimT.nPhoton));
    pickedPhotons_.clear();

    for (int phoInd = 0; phoInd < skimT.nPhoton; ++phoInd) {
        double pt  = skimT.Photon_pt[phoInd];
        double absEta = std::abs(skimT.Photon_eta[phoInd]);
        double r9  = skimT.Photon_r9[phoInd];
        double hoe = skimT.Photon_hoe[phoInd];
        bool eleVeto= skimT.Photon_electronVeto[phoInd];
        bool pixelSeed = skimT.Photon_pixelSeed[phoInd];
        Int_t id = skimT.Photon_cutBased[phoInd];  // Tight ID
        //Int_t mvaId = skimT.Photon_mvaID_WP80[phoInd];  // Tight ID
        if(pt > minPtPho_ && absEta < maxEtaPho_ && r9 < maxR9Pho_ && r9 > minR9Pho_ && hoe < maxHoePho_ && id==tightIdPho_ && eleVeto==isEleVetoPho_ && pixelSeed==hasPixelSeedPho_){
            pickedPhotons_.push_back(phoInd);
        }
        printDebug(
            "Photon " + std::to_string(phoInd) + 
            ", Id  = " + std::to_string(id) + 
            ", pt  = " + std::to_string(pt) + 
            ", absEta  = " + std::to_string(absEta) + 
            ", hoe  = " + std::to_string(hoe) + 
            ", r9  = " + std::to_string(r9)
       );
    }
    printDebug("Total Photons Selected: " + std::to_string(pickedPhotons_.size()));
}

TLorentzVector PickGamJetFake::getMatchedGenPhotonP4(const SkimTree& skimT, int phoInd) const {
    TLorentzVector zero;
    if (phoInd < 0 || phoInd >= skimT.nPhoton) {
        printDebug("getMatchedGenPhotonP4: invalid phoInd = " + std::to_string(phoInd));
        return zero;
    }

    // Reco photon kinematics & shower handle
    const double recoPt  = skimT.Photon_pt[phoInd];
    const double recoEta = skimT.Photon_eta[phoInd];
    const double recoPhi = skimT.Photon_phi[phoInd];
    const double r9      = skimT.Photon_r9[phoInd]; // high R9 → tighter core
    const double maxDR   = maxDeltaRgenPho_;        // e.g. 0.1–0.2 from config

    auto dR_to_reco = [&](const TLorentzVector& v) {
        return HelperDelta::DELTAR(v.Phi(), recoPhi, v.Eta(), recoEta);
    };
    auto buildP4_gen = [&](int idx, double mass=0.0) {
        TLorentzVector p4;
        p4.SetPtEtaPhiM(skimT.GenPart_pt[idx], skimT.GenPart_eta[idx], skimT.GenPart_phi[idx], mass);
        return p4;
    };
    auto wrapDPhi = [&](double a, double b){
        double d = a - b; while (d >  M_PI) d -= 2*M_PI; while (d <= -M_PI) d += 2*M_PI; return d;
    };

    // -------------------------------
    // Adaptive catchment sizes
    // -------------------------------
    // Small "core" cone: capture ultra-collimated daughters at high pT
    const double R_core = (r9 > 0.90 ? 0.035 : 0.060);

    // Wider "tail" cone for low pT: ~1/pT scaling, clamped to [R_core, maxDR]
    const double invPt   = 1.0 / std::max(20.0, recoPt);
    double R_adapt       = std::min(maxDR, std::max(R_core, 2.0 * invPt + 0.03)); // tuneable
    if (recoPt < 60.0) R_adapt = std::max(R_adapt, 0.08); // low-pT floor

    // Supercluster-like window (η narrow, ϕ wide). Grow ϕ for low R9 and very low pT.
    double eta_max = 0.020;
    double phi_max = 0.090;                 // try 0.09–0.12 at low pT
    if (r9 < 0.80) phi_max *= 1.25;
    if (recoPt < 40.0) phi_max = std::max(phi_max, 0.12);

    // Scoring: prefer angular match, allow pT agreement to break ties
    // lambdaPt ~ 0.02 → 10% pT mismatch penalizes like ΔR ≈ 0.10
    const double lambdaPt = 0.02;  // tuneable

    struct Cand { TLorentzVector p4; double dR; double dPtRel; std::string tag; };
    std::vector<Cand> cands; cands.reserve(48);

    auto push_cand = [&](const TLorentzVector& p4, const char* tag) {
        if (p4.Pt() <= 0) return;
        const double dR = dR_to_reco(p4);
        if (dR >= maxDR) return; // geometric consistency
        const double dPtRel = std::abs(p4.Pt() - recoPt) / std::max(1.0, recoPt);
        cands.push_back({p4, dR, dPtRel, tag});
    };

    // =====================================================
    // 1) Neutral-meson mapping: mother → {γ, γ} (π0=111, η=221)
    // =====================================================
    std::unordered_map<int, std::vector<int>> motherToGammas;
    motherToGammas.reserve(64);
    for (int i = 0; i < skimT.nGenPart; ++i) {
        if (skimT.GenPart_pdgId[i] != 22) continue; // photons only
        const int mom = skimT.GenPart_genPartIdxMother[i];
        if (mom < 0 || mom >= skimT.nGenPart) continue;
        const int amom = std::abs(skimT.GenPart_pdgId[mom]);
        if (amom == 111 || amom == 221) motherToGammas[mom].push_back(i);
    }

    for (const auto& kv : motherToGammas) {
        const int momIdx = kv.first;
        const auto& idxs = kv.second;

        TLorentzVector mom4 = buildP4_gen(momIdx, skimT.GenPart_mass[momIdx]);
        if (idxs.size() >= 2) {
            for (size_t a = 0; a + 1 < idxs.size(); ++a) {
                for (size_t b = a + 1; b < idxs.size(); ++b) {
                    TLorentzVector g1 = buildP4_gen(idxs[a], 0.0);
                    TLorentzVector g2 = buildP4_gen(idxs[b], 0.0);
                    push_cand(g1 + g2, "mother:pair-sum");
                }
            }
            push_cand(mom4, "mother:4vec");
        } else if (idxs.size() == 1) {
            TLorentzVector g1 = buildP4_gen(idxs[0], 0.0);
            const double dR_mom = dR_to_reco(mom4);
            const double dR_g1  = dR_to_reco(g1);
            const double dR_slack = 0.02; // prefer mother if not much worse in ΔR
            if (dR_mom < maxDR && (dR_mom <= dR_g1 + dR_slack)) push_cand(mom4, "mother:prefer");
            push_cand(g1, "mother:single-gamma");
        }
    }

    // =====================================================
    // 2) Photon sums around reco dir
    //    (a) Round cones: core & adaptive
    //    (b) SC-shaped window: η-ϕ rectangle OR within R_adapt
    // =====================================================
    auto sum_photons_in_cone = [&](double R, const char* tag) {
        TLorentzVector sum(0,0,0,0);
        for (int i = 0; i < skimT.nGenPart; ++i) {
            if (skimT.GenPart_pdgId[i] != 22) continue;
            TLorentzVector g = buildP4_gen(i, 0.0);
            if (dR_to_reco(g) < R) sum += g;
        }
        push_cand(sum, tag);
    };

    auto sum_photons_in_SCwindow = [&](double R, const char* tag) {
        TLorentzVector sum(0,0,0,0);
        for (int i = 0; i < skimT.nGenPart; ++i) {
            if (skimT.GenPart_pdgId[i] != 22) continue;
            const double dEta = skimT.GenPart_eta[i] - recoEta;
            const double dPhi = wrapDPhi(skimT.GenPart_phi[i], recoPhi);
            const double dR   = std::hypot(dEta, dPhi);
            if (dR < R || (std::abs(dEta) < eta_max && std::abs(dPhi) < phi_max)) {
                TLorentzVector g = buildP4_gen(i, 0.0);
                sum += g;
            }
        }
        push_cand(sum, tag);
    };

    sum_photons_in_cone(R_core,   "sum:core");
    sum_photons_in_cone(R_adapt,  "sum:adapt");
    sum_photons_in_SCwindow(R_adapt, "sum:SCwindow");

    // =====================================================
    // 3) Nearest any GenPart γ (non-isolated allowed)
    // =====================================================
    {
        int bestIdx = -1; double best = maxDR;
        for (int i = 0; i < skimT.nGenPart; ++i) {
            if (skimT.GenPart_pdgId[i] != 22) continue;
            TLorentzVector g = buildP4_gen(i, 0.0);
            const double dR = dR_to_reco(g);
            if (dR < best) { best = dR; bestIdx = i; }
        }
        if (bestIdx >= 0) push_cand(buildP4_gen(bestIdx, 0.0), "nearest:any-gen-gamma");
    }

    // =====================================================
    // 4) GenIsolatedPhoton (prompt isolated γ collection)
    // =====================================================
    if (skimT.nGenIsolatedPhoton > 0) {
        int bestIdx = -1; double best = maxDR;
        for (int i = 0; i < skimT.nGenIsolatedPhoton; ++i) {
            TLorentzVector p4;
            p4.SetPtEtaPhiM(skimT.GenIsolatedPhoton_pt[i],
                            skimT.GenIsolatedPhoton_eta[i],
                            skimT.GenIsolatedPhoton_phi[i], 0.0);
            const double dR = dR_to_reco(p4);
            if (dR < best) { best = dR; bestIdx = i; }
        }
        if (bestIdx >= 0) {
            TLorentzVector p4;
            p4.SetPtEtaPhiM(skimT.GenIsolatedPhoton_pt[bestIdx],
                            skimT.GenIsolatedPhoton_eta[bestIdx],
                            skimT.GenIsolatedPhoton_phi[bestIdx], 0.0);
            push_cand(p4, "nearest:genIso");
        }
    }

    // =====================================================
    // 5) Choose best candidate by score:
    //     score = ΔR + λ * |ΔpT|/pT_reco
    // =====================================================
    if (!cands.empty()) {
        auto score = [&](const Cand& c) { return c.dR + lambdaPt * c.dPtRel; };
        const Cand* best = &cands[0];
        double bestScore = score(*best);
        for (size_t i = 1; i < cands.size(); ++i) {
            const double sc = score(cands[i]);
            if (sc < bestScore) { best = &cands[i]; bestScore = sc; }
        }
        printDebug("getMatchedGenPhotonP4: picked '" + best->tag
                   + "'  dR=" + std::to_string(best->dR)
                   + "  pTgen=" + std::to_string(best->p4.Pt())
                   + "  pTreco=" + std::to_string(recoPt)
                   + "  rel|ΔpT|=" + std::to_string(best->dPtRel)
                   + "  R_core=" + std::to_string(R_core)
                   + "  R_adapt=" + std::to_string(R_adapt)
                   + "  eta_max=" + std::to_string(eta_max)
                   + "  phi_max=" + std::to_string(phi_max));
        return best->p4;
    }

    // =====================================================
    // 6) No match
    // =====================================================
    printDebug("getMatchedGenPhotonP4: no gen match within ΔR < " + std::to_string(maxDR));
    return zero;
}


std::vector<double> PickGamJetFake::getRawJetPts(const SkimTree& skimT) const{
    std::vector<double> rawPts;
    rawPts.reserve(skimT.nJet);
    for (int i = 0; i < skimT.nJet; ++i) {
        float pt = skimT.Jet_pt[i];
        float rawFact = skimT.Jet_rawFactor[i];
        rawPts.push_back((1-rawFact)*pt);
    }
    return rawPts;
}
std::vector<double> PickGamJetFake::getRawPhoPts(const SkimTree& skimT) const{
    std::vector<double> rawPts;
    rawPts.reserve(skimT.nPhoton);
    for (int i = 0; i < skimT.nPhoton; ++i) {
        float pt = skimT.Photon_pt[i];
        rawPts.push_back(pt);
    }
    return rawPts;
}

