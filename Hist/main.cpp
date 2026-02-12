// Common
#include "SkimFile.h"
#include "SkimTree.h"
#include "RunsTree.h"
#include "ScaleEvent.h"
#include "GlobalFlag.h"
#include "Helper.hpp"
#include "Logger.h"
#include "fwk/ConfigService.h"
#include "fwk/Context.h"
#include "fwk/CutflowService.h"
#include "fwk/Driver.h"
#include "fwk/Factory.h"
#include "fwk/LoggerService.h"
#include "fwk/OutputService.h"
#include "fwk/TimerService.h"

// system
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>                 // getopt/optind on Linux
#include <filesystem>
#include <nlohmann/json.hpp>
#include <memory>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace fs = std::filesystem;
using json = nlohmann::json;

// ----------------------------------------------
// Helpers
// ----------------------------------------------
namespace {

std::vector<std::string> listJsonFiles(const std::string& jsonDir) {
    std::vector<std::string> out;
    if (!fs::exists(jsonDir)) return out;

    for (const auto& entry : fs::directory_iterator(jsonDir)) {
        if (!entry.is_regular_file()) continue;

        const auto& p = entry.path();
        const auto fname = p.filename().string();

        if (p.extension() == ".json" && fname.rfind("FilesHist", 0) == 0) {
            out.push_back(p.string());
        }
    }
    return out;
}

void printHelpAndExamples(const std::vector<std::string>& jsonFiles) {
    for (const auto& jsonFile : jsonFiles) {
        std::ifstream file(jsonFile);
        if (!file.is_open()) {
            std::cerr << "Could not open file: " << jsonFile << "\n";
            continue;
        }

        json js;
        try {
            js = json::parse(file);
        } catch (const std::exception& e) {
            std::cerr << "Error parsing JSON file: " << jsonFile << "\n"
                      << e.what() << "\n";
            continue;
        }

        std::cout << "\nFor file: " << jsonFile << "\n";
        for (auto& element : js.items()) {
            std::cout << "./runMain " << element.key() << "_1of100.root\n";
        }
    }
}

[[noreturn]] void dieUsage(const std::string& msg) {
    std::cerr << "ERROR: " << msg << "\nUse -h for help.\n";
    std::exit(1);
}



} // namespace

// ----------------------------------------------------------------------
// main
// ----------------------------------------------------------------------
int main(int argc, char* argv[]) {
    if (argc == 1) {
        std::cerr << "Error: No arguments provided. Use -h for help.\n";
        return 1;
    }

    const std::string jsonDir = "input/json/";
    const auto jsonFiles = listJsonFiles(jsonDir);
    if (jsonFiles.empty()) {
        std::cerr << "No JSON files found in directory: " << jsonDir << "\n";
        return 1;
    }

    bool isDebug      = false;
    bool runCacheFill = false;   // -r mode
    bool forceYes     = false;   // -y to skip confirmation

    int opt;
    while ((opt = getopt(argc, argv, "hdry")) != -1) {
        switch (opt) {
            case 'd': isDebug = true; break;
            case 'r': runCacheFill = true; break;
            case 'y': forceYes = true; break;
            case 'h':
                printHelpAndExamples(jsonFiles);
                return 0;
            default:
                std::cerr << "Use -h for help\n";
                return 1;
        }
    }

    // ---------------------------------------------------------
    // -r mode: prefill RunsTree cache
    // ---------------------------------------------------------
    if (runCacheFill) {
        if (!forceYes) {
            std::cout
                << ">>> WARNING: You are about to precompute normalization\n"
                << "    for ALL MC samples found in FilesHist_*.json.\n"
                << "    This may take a long time and will rewrite config/RunsTree.json.\n\n"
                << "    Type \"yes\" to continue, anything else to abort: ";

            std::string answer;
            if (!std::getline(std::cin, answer)) {
                std::cerr << "\nInput error, aborting -r mode.\n";
                return 1;
            }
            if (answer != "yes") {
                std::cout << "Aborting -r mode (no changes made).\n";
                return 0;
            }
        }

        return RunsTree::prefillRunsTreeCache(jsonFiles, jsonDir, isDebug);
    }

    // ---------------------------------------------------------
    // Normal mode: expect one positional argument
    // ---------------------------------------------------------
    if (optind >= argc) {
        dieUsage("Output filename missing. Usage: ./runMain [-d] <ioName.root>");
    }
    const std::string ioName = argv[optind];

    try {
        Helper::printBanner("Set GlobalFlag");
        GlobalFlag globalFlag(ioName);
        globalFlag.setDebug(isDebug);
        globalFlag.setNDebug(10000);
        globalFlag.printFlags(std::cout);

        Helper::printBanner("Set and load SkimFile");
        const std::string inJsonDir = "input/json/";
        auto skimF = std::make_shared<SkimFile>(globalFlag, ioName, inJsonDir);

        Helper::printBanner("Set and load RunsTree");
        auto runsT = std::make_shared<RunsTree>(globalFlag);

        Double_t normGenEventSumw = 1.0;
        if (globalFlag.isMC()) {
            const std::string cacheFilePath = "config/RunsTree.json";
            normGenEventSumw = runsT->getCachedNormGenEventSumw(
                skimF->getSampleKey(),
                cacheFilePath,
                skimF->getAllFileNames()
            );
        }

        Helper::printBanner("Set and load SkimTree");
        auto skimT = std::make_shared<SkimTree>(globalFlag);
        skimT->loadTree(skimF->getJobFileNames());

        Helper::printBanner("Set and load ScaleEvent");
        auto scaleEvent = std::make_shared<ScaleEvent>(
            globalFlag,
            globalFlag.getLumiPerYear(),
            skimF->getXsecOrLumiNano(),
            skimF->getEventsNano(),
            normGenEventSumw
        );

        const std::string outDir = "output";
        fs::create_directories(outDir);

        auto fout = std::make_unique<TFile>((outDir + "/" + ioName).c_str(), "RECREATE");
        if (!fout || fout->IsZombie()) {
            throw std::runtime_error("Failed to create output ROOT file: " + outDir + "/" + ioName);
        }

        Helper::printBanner("Run framework module chain");

        fwk::Context ctx(globalFlag);
        ctx.skimT = skimT;
        ctx.scaleEvent = scaleEvent.get();
        ctx.out = std::make_unique<fwk::OutputService>(fout.get());
        ctx.cutflow = std::make_unique<fwk::CutflowService>();
        ctx.timer = std::make_unique<fwk::TimerService>();
        ctx.config = std::make_unique<fwk::ConfigService>();
        ctx.log = std::make_unique<fwk::LoggerService>();

        auto chain = fwk::makeChain(globalFlag);
        return fwk::Driver::run(ctx, chain);
    }
    catch (const std::exception& e) {
        std::cerr << "FATAL EXCEPTION: " << e.what() << "\n";
        return 1;
    }
}

