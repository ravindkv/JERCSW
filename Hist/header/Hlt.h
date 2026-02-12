
#pragma once

#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "GlobalFlag.h"
// Define the TrigRangePt struct
struct TrigRangePt {
    double ptMin;
    double ptMax;
};

// Define the TrigRangePtEta struct
struct TrigRangePtEta {
    int trigPt;
    double ptMin;
    double ptMax;
    double absEtaMin;
    double absEtaMax;
};

// Define the Hlt class
class Hlt {
public:
    explicit Hlt(const GlobalFlag& globalFlags);

    // Destructor
    ~Hlt();

    // Method to get the trigNames
    const std::vector<std::string> getTrigNames() const;

    // Method to get the trigList 
    const std::vector<std::string>& getTrigList() const {return trigList_;}
    
    /// Return the integrated luminosity for a given HLT path (or 0 if not found)
    double getHltLumi(const std::string& hltName) const;

    /// Return a weight based on per-HLT and per-year luminosities (1.0 on MC)
    double getHltLumiWeight(const std::string& passedHltName) const;

    // Method to get the trigMapRangePt std::unordered_map
    const std::unordered_map<std::string, TrigRangePt>& getTrigMapRangePt() const {return trigMapRangePt_;}

    // Method to get the trigMapRangePtEta std::unordered_map
    const std::unordered_map<std::string, TrigRangePtEta>& getTrigMapRangePtEta() const {return trigMapRangePtEta_;}

private:
    // Reference to GlobalFlag instance
    const GlobalFlag& globalFlags_;
    const GlobalFlag::Year year_;
    const GlobalFlag::Channel channel_;
    const bool isDebug_;
    // trigList std::unordered_map
    std::vector<std::string> trigList_;

    // Map each HLT name to its lumi
    std::unordered_map<std::string, double>  hltLumiMap_;

    // trigMapRangePt std::unordered_map
    std::unordered_map<std::string, TrigRangePt> trigMapRangePt_;

    // trigMapRangePtEta std::unordered_map
    std::unordered_map<std::string, TrigRangePtEta> trigMapRangePtEta_;

    void loadConfig();
};

