#include "VarBin.h"
#include <iostream>
#include <fstream>
#include <string>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

// Anonymous namespace: helper to extract fixed-width ranges from JSON.
// This template returns a std::vector<double> with three elements: [N, min, max].
namespace {
    template<typename T, typename U>
    std::vector<double> getRange(const json &ranges, const std::string &key) {
        std::vector<double> ret;
        ret.push_back(static_cast<double>(ranges.at(key)[0].get<T>()));
        ret.push_back(ranges.at(key)[1].get<U>());
        ret.push_back(ranges.at(key)[2].get<U>());
        return ret;
    }
}

VarBin::VarBin(const GlobalFlag& globalFlags)
    : globalFlags_(globalFlags),
      year_(globalFlags_.getYear()),
      channel_(globalFlags_.getChannel()),
      isDebug_(globalFlags_.isDebug())
{
    InitializeBins();
}

void VarBin::InitializeBins() {
    // Open and parse the JSON configuration file for VarBin settings.
    std::ifstream ifs("config/VarBin.json");
    if (!ifs.is_open()) {
        std::cerr << "Error: Could not open config/VarBin.json" << std::endl;
        return;
    }
    json j;
    try {
        ifs >> j;
    } catch (json::parse_error& e) {
        std::cerr << "JSON parse error in VarBin: " << e.what() << std::endl;
        return;
    }

    // Use getChannelStr() from GlobalFlag to select the channel configuration.
    std::string channelStr = globalFlags_.getChannelStr();

    // Set binsPt_ based on channel
    if (j["channels"].contains(channelStr)) {
        binsPt_ = j["channels"][channelStr]["binsPt"].get<std::vector<double>>();
        // For GamJet, if a binsMass override is provided, use it.
        if (channel_ == GlobalFlag::Channel::GamJet &&
            j["channels"][channelStr].contains("binsMass")) {
            binsMass_ = j["channels"][channelStr]["binsMass"].get<std::vector<double>>();
        }
    } else {
        binsPt_ = j["channels"]["default"]["binsPt"].get<std::vector<double>>();
    }

    // Global bin arrays
    binsEta_ = j["binsEta"].get<std::vector<double>>();
    binsPhi_ = j["binsPhi"].get<std::vector<double>>();  
    binsRho_ = j["binsRho"].get<std::vector<double>>();  
    binsAlpha_ = j["binsAlpha"].get<std::vector<double>>();  
    // For binsPhiRebin, if the JSON entry is "same_as_binsPhi", copy binsPhi_
    if (j["binsPhiRebin"].is_string()) {
        std::string s = j["binsPhiRebin"].get<std::string>();
        if (s == "same_as_binsPhi") {
            binsPhiRebin_ = binsPhi_;
        }
    } else {
        binsPhiRebin_ = j["binsPhiRebin"].get<std::vector<double>>();
    }
    // Use global binsMass if not set already
    if (binsMass_.empty()) {
        binsMass_ = j["binsMass"].get<std::vector<double>>();
    }

    // Use helper getRange() to load fixed-width ranges.
    json ranges = j["ranges"];
    rangePt_       = getRange<int, double>(ranges, "rangePt");
    rangeEta_      = getRange<int, double>(ranges, "rangeEta");
    rangePhi_      = getRange<int, double>(ranges, "rangePhi");
    rangePhiRebin_ = getRange<int, double>(ranges, "rangePhiRebin");
    rangeMass_     = getRange<int, double>(ranges, "rangeMass");
    rangeChiSqr_   = getRange<int, double>(ranges, "rangeChiSqr");
    rangeMassJet1_ = getRange<int, double>(ranges, "rangeMassJet1");
    rangeMassJetSum_ = getRange<int, double>(ranges, "rangeMassJetSum");
    rangeMassW_  = getRange<int, double>(ranges, "rangeMassW");
    rangeMassTop_  = getRange<int, double>(ranges, "rangeMassTop");
    rangePx_  = getRange<int, double>(ranges, "rangePx");
    rangeRun_      = getRange<int, double>(ranges, "rangeRun");
    rangeDeltaR_   = getRange<int, double>(ranges, "rangeDeltaR");
    rangeDeltaPhi_ = getRange<int, double>(ranges, "rangeDeltaPhi");
    rangeResponse_ = getRange<int, double>(ranges, "rangeResponse");
    rangeCountRef_ = getRange<int, double>(ranges, "rangeCountRef");
    rangeFraction_ = getRange<int, double>(ranges, "rangeFraction");
    rangeIndex_    = getRange<int, double>(ranges, "rangeIndex");
    rangeAlpha_       = getRange<int, double>(ranges, "rangeAlpha");
    rangeAsymmetry_       = getRange<int, double>(ranges, "rangeAsymmetry");

    // (Optional) Print bins for debugging.
    printBins("BinsPt", binsPt_);
    printBins("BinsEta", binsEta_);
    printBins("BinsPhi", binsPhi_);
    printBins("BinsRho", binsRho_);
    printBins("BinsPhiRebin", binsPhiRebin_);
    printBins("BinsMass", binsMass_);
}

// Helper method to print bin values
void VarBin::printBins(const std::string& binName, const std::vector<double>& bins) const {
    if (isDebug_) {
        std::cout << "======= VarBin::" << binName << " =======\n";
        for (const auto& bin : bins) {
            std::cout << bin << ", ";
        }
        std::cout << "\n";
    }
}

// Getters
const std::vector<double>& VarBin::getBinsPt() const {
    printBins("getBinsPt()", binsPt_);
    return binsPt_;
}
const std::vector<double>& VarBin::getBinsEta() const {
    printBins("getBinsEta()", binsEta_);
    return binsEta_;
}
const std::vector<double>& VarBin::getBinsPhi() const {
    printBins("getBinsPhi()", binsPhi_);
    return binsPhi_;
}
const std::vector<double>& VarBin::getBinsRho() const {
    printBins("getBinsRho()", binsRho_);
    return binsRho_;
}
const std::vector<double>& VarBin::getBinsPhiRebin() const {
    printBins("getBinsPhiRebin()", binsPhiRebin_);
    return binsPhiRebin_;
}
const std::vector<double>& VarBin::getBinsMass() const {
    printBins("getBinsMass()", binsMass_);
    return binsMass_;
}
const std::vector<double>& VarBin::getBinsAlpha() const {
    printBins("getBinsAlpha()", binsAlpha_);
    return binsAlpha_;
}

const std::vector<double>& VarBin::getRangePt() const {
    printBins("getRangePt()", rangePt_);
    return rangePt_;
}
const std::vector<double>& VarBin::getRangeEta() const {
    printBins("getRangeEta()", rangeEta_);
    return rangeEta_;
}
const std::vector<double>& VarBin::getRangePhi() const {
    printBins("getRangePhi()", rangePhi_);
    return rangePhi_;
}
const std::vector<double>& VarBin::getRangePhiRebin() const {
    printBins("getRangePhiRebin()", rangePhiRebin_);
    return rangePhiRebin_;
}
const std::vector<double>& VarBin::getRangeMass() const {
    printBins("getRangeMass()", rangeMass_);
    return rangeMass_;
}
const std::vector<double>& VarBin::getRangeMassJet1() const {
    printBins("getRangeMassJet1()", rangeMassJet1_);
    return rangeMassJet1_;
}
const std::vector<double>& VarBin::getRangeMassJetSum() const {
    printBins("getRangeMassJetSum()", rangeMassJetSum_);
    return rangeMassJetSum_;
}
const std::vector<double>& VarBin::getRangeMassW() const {
    printBins("getRangeMassW()", rangeMassW_);
    return rangeMassW_;
}
const std::vector<double>& VarBin::getRangeMassTop() const {
    printBins("getRangeMassTop()", rangeMassTop_);
    return rangeMassTop_;
}
const std::vector<double>& VarBin::getRangePx() const {
    printBins("getRangePx()", rangePx_);
    return rangePx_;
}
const std::vector<double>& VarBin::getRangeChiSqr() const {
    printBins("getRangeChiSqr()", rangeChiSqr_);
    return rangeChiSqr_;
}
const std::vector<double>& VarBin::getRangeRun() const {
    printBins("getRangeRun()", rangeRun_);
    return rangeRun_;
}
const std::vector<double>& VarBin::getRangeDeltaR() const {
    printBins("getRangeDeltaR()", rangeDeltaR_);
    return rangeDeltaR_;
}
const std::vector<double>& VarBin::getRangeDeltaPhi() const {
    printBins("getRangeDeltaPhi()", rangeDeltaPhi_);
    return rangeDeltaPhi_;
}
const std::vector<double>& VarBin::getRangeResponse() const {
    printBins("getRangeResponse()", rangeResponse_);
    return rangeResponse_;
}
const std::vector<double>& VarBin::getRangeCountRef() const {
    printBins("getRangeCountRef()", rangeCountRef_);
    return rangeCountRef_;
}
const std::vector<double>& VarBin::getRangeFraction() const {
    printBins("getRangeFraction()", rangeFraction_);
    return rangeFraction_;
}
const std::vector<double>& VarBin::getRangeIndex() const {
    printBins("getRangeIndex()", rangeIndex_);
    return rangeIndex_;
}

const std::vector<double>& VarBin::getRangeAlpha() const {
    printBins("getRangeAlpha()", rangeAlpha_);
    return rangeAlpha_;
}
const std::vector<double>& VarBin::getRangeAsymmetry() const {
    printBins("getRangeAsymmetry()", rangeAsymmetry_);
    return rangeAsymmetry_;
}
