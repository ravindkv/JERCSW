
#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <cmath>

#include "SkimTree.h"
#include "GlobalFlag.h"
#include "PickJet.h"
#include "Hlt.h"

#include "correction.h"
#include <nlohmann/json.hpp>
#include <TLorentzVector.h>

class PickEvent {
public:
    // Constructor accepting a reference to GlobalFlag
    explicit PickEvent(const GlobalFlag& globalFlags);

    // --- Trigger logic ---
    bool passHlt(const std::shared_ptr<SkimTree>& skimT);
    bool passHltWithPt(const std::shared_ptr<SkimTree>& skimT, const double& pt);
    bool passHltWithPtEta(const std::shared_ptr<SkimTree>& skimT,
                          const double& pt,
                          const double& eta);
    std::vector<std::string> getPassedHlts() { return passedHlts_; }
    std::string              getPassedHlt()  { return passedHlt_; }

    std::unordered_map<std::string, const Bool_t*> getTrigValues() const;

    // --- Event filters 
    /// Golden JSON filter (DATA only). MC always returns true.
    bool passGoodLumi(unsigned int run, unsigned int lumi) const;

    /// Jet veto using all jets in SkimTree. Returns true if event is *NOT* vetoed.
    bool passJetVetoMap(const SkimTree& skimT) const;

    /// Jet veto only on probe jet1 4-vector. Returns true if jet1 is *NOT* vetoed.
    bool passJetVetoMapOnProbe(const TLorentzVector& p4Probe) const;

    bool passPtHatFilter(const SkimTree& skimT, double leadJetPt) const;
    bool passPtHatFilterAuto(const SkimTree& skimT, double leadJetPt);

    // Returns true if event passes the the PVz - GenVtx_z < 0.2; 
    bool passMatchedGenVtx(const SkimTree& skimT) const;

    ~PickEvent();

private:
    // Reference to GlobalFlag instance
    const GlobalFlag& globalFlags_;
    const GlobalFlag::Year    year_;
    const GlobalFlag::Era     era_;
    const GlobalFlag::Channel channel_;
    const bool                isDebug_;
    const bool                isMC_;
    const bool                isData_;
    PickJet pickJet_;

    Hlt                       hlt_;
    std::vector<std::string>  passedHlts_{};
    std::string               passedHlt_;

    // --- Config and cache for lumi + jet veto ---
    // Jet veto
    std::string jetVetoJsonPath_;
    std::string jetVetoName_;
    std::string jetVetoKey_;
    std::string jetIdLabel_;
    correction::Correction::Ref loadedJetVetoRef_;

    // Golden lumi
    std::string    goldenLumiJsonPath_;
    nlohmann::json loadedGoldenLumiJson_;

    //Pt-Hat and LHE based
    bool   doPtHatFilter_;
    int countWarns_;
    int warnNtimes_ = 10;

    // PV - GenVtx matching
    double   maxDiffPVzGenVtxz_;

    // Helpers
    void printDebug(const std::string& message) const;
    void printWarn(const std::string& msg) const;

    void loadConfig(const std::string& filename);
    void loadJetVetoRef();
    void loadGoldenLumiJson();
};

