#pragma once

#include <vector>
#include <string>
#include <TLorentzVector.h>
#include "SkimTree.h"
#include "GlobalFlag.h"
#include "PickJet.h"

class PickMultiJet {
public:
    explicit PickMultiJet(const GlobalFlag& globalFlags);
    ~PickMultiJet();

    /// Run the jet‐selection on this event:
    void pickJets(const SkimTree& skimT);

    // getters for the results:
    int                                   getProbeIndex()     const { return iProbe_; }
    const std::vector<int>&               getRecoilIndices()    const { return recoilIndices_; }
    const TLorentzVector&                 getProbe()          const { return p4Probe_; }
    const TLorentzVector&                 getSumRecoiledJets()  const { return p4SumRecoiledJets_; }
    const TLorentzVector&                 getSumOther()         const { return p4SumOther_; }
    bool                                  vetoNearByJets()     const { return vetoNear_; }
    bool                                  vetoForwardJets()    const { return vetoFwd_; }
    double                                getHardestRecoilPt() const { return ptHardestInRecoil_; }

private:
    // configuration & helpers
    const GlobalFlag&      globalFlags_;
    PickJet pickJet_;
    const bool       isDebug_;
    void             printDebug(const std::string& msg) const;

    // selection thresholds:
    double minPtProbe_;       
    std::string jetIdLabel_;
    double maxEtaRecoil_;    
    double minPtRecoil_;     
    double minPtOther_;      
    double minDPhiRecoil_;   
    // load configuration from JSON file
    void loadConfig(const std::string& filename);

    // outputs:
    int                    iProbe_;
    std::vector<int>       recoilIndices_;
    TLorentzVector         p4Probe_;
    TLorentzVector         p4SumRecoiledJets_;
    TLorentzVector         p4SumOther_;
    bool                   vetoNear_;
    bool                   vetoFwd_;
    double                 ptHardestInRecoil_;

};

