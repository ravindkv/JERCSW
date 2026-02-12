#include "ScaleEvent.h"
#include <stdexcept>
#include <regex>

ScaleEvent::ScaleEvent(GlobalFlag& globalFlags,
                       double lumiPerYear,
                       double xsecOrLumiNano,
                       double nEventsNano,
                       Double_t normGenEventSumw)
    : globalFlags_(globalFlags),
      year_(globalFlags_.getYear()),
      era_(globalFlags_.getEra()),
      channel_(globalFlags_.getChannel()),
      isDebug_(globalFlags_.isDebug()),
      isData_(globalFlags_.isData()),
      isMC_(globalFlags_.isMC()),
      normGenEventSumw_(normGenEventSumw)
{
    loadConfig("config/ScaleEvent.json");

    if (isMC_) {
        // 1000: convert pb → fb
        lumiWeight_ = 1000.0 * lumiPerYear * xsecOrLumiNano / nEventsNano;
        loadPuRef();
    } else {
        lumiWeight_ = 1.0;
    }

        std::cout << "==> [ScaleEvent]\n"
                  << "  normGenEventSumw = " << normGenEventSumw_ << '\n'
                  << "  lumiPerYear      = " << lumiPerYear << '\n'
                  << "  xsecOrLumiNano   = " << xsecOrLumiNano << '\n'
                  << "  nEventsNano      = " << nEventsNano << '\n'
                  << "  lumiWeight       = " << lumiWeight_ << '\n';
}

void ScaleEvent::loadConfig(const std::string& filename) {
    ReadConfig config(filename);
    std::string yearKey = globalFlags_.getYearStr();

    puJsonPath_ = config.getValue<std::string>({yearKey, "puJsonPath"});
    puName_     = config.getValue<std::string>({yearKey, "puName"});
    minbXsec_   = config.getValue<double>      ({yearKey, "minbXsec"});

    if (isDebug_) {
        std::cout << "Loaded ScaleEvent config for " << yearKey << ":\n"
                  << "  puJsonPath = " << puJsonPath_ << '\n'
                  << "  puName     = " << puName_     << '\n'
                  << "  minbXsec   = " << minbXsec_   << '\n';
    }
}

void ScaleEvent::loadPuRef() {
    std::cout << "==> ScaleEvent::loadPuRef()" << '\n';
    try {
        loadedPuRef_ =
            correction::CorrectionSet::from_file(puJsonPath_)->at(puName_);
    } catch (const std::exception& e) {
        std::cout << "\nEXCEPTION: ScaleEvent::loadPuRef()\n";
        std::cout << "Check " << puJsonPath_ << " or " << puName_ << '\n';
        std::cout << e.what() << '\n';
        throw std::runtime_error("Error loading Pileup Reference.");
    }
}

double ScaleEvent::getPuCorrection(Float_t nTrueInt,
                                   const std::string& nomOrSyst) const {
    double puSf = 1.0;
    if (!isMC_) return puSf;

    try {
        puSf = loadedPuRef_->evaluate({nTrueInt, nomOrSyst.c_str()});
        if (isDebug_) {
            std::cout << "PU: nomOrSyst = " << nomOrSyst
                      << ", nTrueInt   = " << nTrueInt
                      << ", puSf       = " << puSf << '\n';
        }
    } catch (const std::exception& e) {
        std::cout << "\nEXCEPTION: ScaleEvent::getPuCorrection(): "
                  << e.what() << '\n';
        throw std::runtime_error("Error calculating Pileup Correction.");
    }
    return puSf;
}

double ScaleEvent::getEventWeight(const SkimTree& skimT) const {
    double weight = 1.0;
    if (!isMC_) return weight;

    const double genWeight = skimT.genWeight;

    weight *= genWeight / normGenEventSumw_;
    weight *= lumiWeight_;
    weight *= getPuCorrection(skimT.Pileup_nTrueInt, "nominal");
    weight *= skimT.L1PreFiringWeight;

    if (isDebug_) {
        std::cout << "Event weight breakdown:\n"
                  << "  genWeight            = " << genWeight << '\n'
                  << "  normGenEventSumw     = " << normGenEventSumw_ << '\n'
                  << "  lumiWeight           = " << lumiWeight_ << '\n'
                  << "  PU SF (nominal)      = "
                  << getPuCorrection(skimT.Pileup_nTrueInt, "nominal") << '\n'
                  << "  L1PreFiringWeight    = " << skimT.L1PreFiringWeight << '\n'
                  << "  --> total weight     = " << weight << '\n';
    }
    return weight;
}

