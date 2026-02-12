#include "Hlt.h"
#include <fstream>
#include <nlohmann/json.hpp>
#include <iostream>
using json = nlohmann::json;

// Constructor implementation
Hlt::Hlt(const GlobalFlag& globalFlags)
    : globalFlags_(globalFlags),
      year_(globalFlags_.getYear()),
      channel_(globalFlags_.getChannel()),
      isDebug_(globalFlags_.isDebug()) {

    loadConfig();
}

void Hlt::loadConfig() {
    // Reserve capacity as before (optional)
    trigMapRangePt_.reserve(15);
    trigMapRangePtEta_.reserve(30);
    
    // Get string representations for year and channel.
    std::string yearStr = globalFlags_.getYearStr();
    if (year_ == GlobalFlag::Year::Year2016Pre || year_ == GlobalFlag::Year::Year2016Post){
        yearStr = "2016";
    }
    std::string channelStr = globalFlags_.getChannelStr();
    
    // Build the file name for the current channel's configuration.
    std::string configFile = "config/Hlt" + channelStr + ".json";
    if(globalFlags_.getJetAlgo()==GlobalFlag::JetAlgo::AK8Puppi && 
                    channel_ == GlobalFlag::Channel::DiJet){
        configFile = "config/Hlt" + channelStr + 
                                    globalFlags_.getJetAlgoStr()+ ".json";
    }
    std::cout<<"[Hlt]: "<<configFile<<"\n";
    std::ifstream ifs(configFile);
    if (!ifs.is_open()) {
        std::cerr << "Error: Could not open " << configFile << std::endl;
        return;
    }
    
    json j;
    try {
        ifs >> j;
    } catch (json::parse_error& e) {
        std::cerr << "JSON parse error in " << configFile << ": " << e.what() << std::endl;
        return;
    }
    
    // Ensure the JSON contains the current year.
    if (!j.contains(yearStr)) {
        std::cerr << "Error: " << configFile << " does not contain configuration for year " << yearStr << std::endl;
        return;
    }
    
    // Retrieve the configuration for the active year.
    auto yearConfig = j.at(yearStr);
    
    // For channels with a simple trigger list.
    if (channel_ == GlobalFlag::Channel::ZeeJet ||
        channel_ == GlobalFlag::Channel::ZmmJet ||
        channel_ == GlobalFlag::Channel::GamJetFake ||
        channel_ == GlobalFlag::Channel::Wqqe ||
        channel_ == GlobalFlag::Channel::Wqqm) {
        
        trigList_ = yearConfig.get<std::vector<std::string>>();
    }
    // For channels with a trigger map using only pt (e.g., GamJet).
    else if (channel_ == GlobalFlag::Channel::GamJet) {
        for (auto& item : yearConfig.items()) {
            const std::string& key = item.key();
            json value = item.value();
            double ptMin = value.at("ptMin").get<double>();
            double ptMax = value.at("ptMax").get<double>();
            double lumi  = value.at("lumi" ).get<double>();
            trigMapRangePt_[key] = TrigRangePt{ ptMin, ptMax };
            hltLumiMap_[key] = lumi;
        }
    }
    // For channels with a trigger map using pt and eta (e.g., MultiJet).
    else if (channel_ == GlobalFlag::Channel::MultiJet || 
            channel_ == GlobalFlag::Channel::DiJet) {
        for (auto& item : yearConfig.items()) {
            const std::string& key = item.key();
            json value = item.value();
            int trigPt = value.at("trigPt").get<int>();
            double ptMin = value.at("ptMin").get<double>();
            double ptMax = value.at("ptMax").get<double>();
            double absEtaMin = value.at("absEtaMin").get<double>();
            double absEtaMax = value.at("absEtaMax").get<double>();
            double lumi       = value.at("lumi"       ).get<double>();
            trigMapRangePtEta_[key] = TrigRangePtEta{ trigPt, ptMin, ptMax, absEtaMin, absEtaMax };
            hltLumiMap_[key] = lumi;
        }
    }
}

const std::vector<std::string> Hlt::getTrigNames() const {
    std::vector<std::string> tNames;
    if (channel_ == GlobalFlag::Channel::ZeeJet ||
        channel_ == GlobalFlag::Channel::ZmmJet ||
        channel_ == GlobalFlag::Channel::GamJetFake ||
        channel_ == GlobalFlag::Channel::Wqqe ||
        channel_ == GlobalFlag::Channel::Wqqm) {
        tNames = trigList_;
    } else if (channel_ == GlobalFlag::Channel::GamJet) {
        for (const auto& trig : trigMapRangePt_) {
            tNames.push_back(trig.first);
        }
    } else if (channel_ == GlobalFlag::Channel::MultiJet ||
              channel_ == GlobalFlag::Channel::DiJet) {
        for (const auto& trig : trigMapRangePtEta_) {
            tNames.push_back(trig.first);
        }
    }
    return tNames;
}

double Hlt::getHltLumi(const std::string& hltName) const {
    auto it = hltLumiMap_.find(hltName);
    if (it != hltLumiMap_.end()) {
        return it->second;
    } else {
        if (isDebug_) {
            std::cerr << "Hlt::getHltLumi — warning: no lumi found for " 
                      << hltName << std::endl;
        }
        return 0.0;
    }
}

// New: compute weight = lumiPerYear / lumiPerHlt for data, or 1.0 for MC
double Hlt::getHltLumiWeight(const std::string& passedHltName) const {
    double weightLumiPerHlt = 1.0;
    if (globalFlags_.isData()) {
        double lumiPerHlt = getHltLumi(passedHltName);
        double lumiPerYear = 1.0;
        if (year_ == GlobalFlag::Year::Year2016Pre || year_ == GlobalFlag::Year::Year2016Post) {
            lumiPerYear = 36.3;
        } else {
            lumiPerYear = globalFlags_.getLumiPerYear();
        }
        if (lumiPerHlt > 0.0 && lumiPerYear > 0.0) {
            weightLumiPerHlt = lumiPerYear / lumiPerHlt;
        }
        if (isDebug_) {
            std::cout << "passedHlt   = " << passedHltName << std::endl;
            std::cout << "lumiPerHlt  = " << lumiPerHlt << std::endl;
            std::cout << "lumiPerYear = " << globalFlags_.getLumiPerYear() << std::endl;
            std::cout << "weightLumiPerHlt = " << weightLumiPerHlt << std::endl;
        }
    }
    return weightLumiPerHlt;
}

// Destructor
Hlt::~Hlt() {
    // No dynamic memory to clean up in this class
}

