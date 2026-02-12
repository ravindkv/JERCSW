#pragma once 

#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>

#include "SkimTree.h"
#include "correction.h"
#include "GlobalFlag.h"
#include "ReadConfig.h"

#include <nlohmann/json.hpp>

class ScaleEvent {
public:
    ScaleEvent(GlobalFlag& globalFlags,
               double lumiPerYear,
               double xsecOrLumiNano,
               double nEventsNano,
               Double_t normGenEventSumw);

    ~ScaleEvent() = default;

    // Single entry point to get the event weight
    double getEventWeight(const SkimTree& skimT) const;

    // If you still want direct access
    Double_t getNormGenEventSumw() const { return normGenEventSumw_; }
    double   getLumiWeight()      const { return lumiWeight_; }

    // PU access if needed elsewhere
    double getPuCorrection(Float_t nTrueInt,
                           const std::string& nomOrSyst) const;

private:
    // Pileup config
    std::string puJsonPath_;
    std::string puName_;
    correction::Correction::Ref loadedPuRef_;
    double minbXsec_{};

    Double_t normGenEventSumw_{1.0};
    double lumiWeight_{1.0};

    // Reference to GlobalFlag
    GlobalFlag& globalFlags_;
    const GlobalFlag::Year    year_;
    const GlobalFlag::Era     era_;
    const GlobalFlag::Channel channel_;
    const bool isDebug_;
    const bool isData_;
    const bool isMC_;

    void loadConfig(const std::string& filename);
    void loadPuRef();
};

