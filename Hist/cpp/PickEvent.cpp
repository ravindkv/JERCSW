#include "PickEvent.h"
#include "ReadConfig.h"
#include <fstream>
#include <stdexcept>

// Constructor implementation
PickEvent::PickEvent(const GlobalFlag& globalFlags)
    : globalFlags_(globalFlags),
      pickJet_(globalFlags_),
      year_(globalFlags_.getYear()),
      era_(globalFlags_.getEra()),
      channel_(globalFlags_.getChannel()),
      isDebug_(globalFlags_.isDebug()),
      isMC_(globalFlags_.isMC()),
      isData_(globalFlags_.isData()),
      hlt_(globalFlags_)  // HLT helper
{
    std::cout<<"[PickEvent]\n"<<'\n';
    // Load config for jet veto & golden lumi.
    loadConfig("config/PickEvent.json");

    // Jet veto map (MC & data)
    loadJetVetoRef();

    // Golden JSON (data only)
    if (isData_) {
        loadGoldenLumiJson();
    }
}

// Destructor
PickEvent::~PickEvent() = default;

// Helper function for printing debug messages
void PickEvent::printDebug(const std::string& message) const {
    if (isDebug_) {
        std::cout << message << '\n';
    }
}

void PickEvent::printWarn(const std::string& msg) const
{
    std::cerr << "[WARN] " << msg << "\n";
}

//============================================================
// Config for jet veto + golden lumi
//============================================================

void PickEvent::loadConfig(const std::string& filename) {
    ReadConfig config(filename);
    std::cout<<"===> loadConfig: "<<filename<<'\n';
    std::string yearKey = globalFlags_.getYearStr();

    jetVetoJsonPath_    = config.getValue<std::string>({yearKey, "jetVetoJsonPath"});
    jetVetoName_        = config.getValue<std::string>({yearKey, "jetVetoName"});
    jetVetoKey_         = config.getValue<std::string>({yearKey, "jetVetoKey"});
    jetIdLabel_         = config.getValue<std::string>({yearKey, "jetIdLabel"});
    goldenLumiJsonPath_ = config.getValue<std::string>({yearKey, "goldenLumiJsonPath"});
    doPtHatFilter_ = config.getValue<bool>({"pthatFilter", "doPtHatFilter"});
    maxDiffPVzGenVtxz_ = config.getValue<double>({"diffPVzGenVtxz", "maxDiff"});

    if (isDebug_) {
        std::cout << "PickEvent config for " << yearKey << ":\n"
                  << "  jetVetoJsonPath    = " << jetVetoJsonPath_    << '\n'
                  << "  jetVetoName        = " << jetVetoName_        << '\n'
                  << "  jetVetoKey         = " << jetVetoKey_         << '\n'
                  << "  goldenLumiJsonPath = " << goldenLumiJsonPath_ << '\n';
    }
}

//============================================================
// HLT
//============================================================

// Small helper to format a vector<string>
static std::string joinStrings(const std::vector<std::string>& v,
                               const std::string& sep = ", ")
{
    std::ostringstream os;
    for (size_t i = 0; i < v.size(); ++i) {
        if (i) os << sep;
        os << v[i];
    }
    return os.str();
}

bool PickEvent::passHlt(const std::shared_ptr<SkimTree>& skimT)
{
    printDebug("<- PickEvent::passHlt ->");
    passedHlts_.clear();

    const std::string* names = skimT->getTrigNames();
    const size_t n = skimT->getNumTrigNames();

    for (size_t i = 0; i < n; ++i) {
        const auto& trigName = names[i];
        const int trigValue  = static_cast<int>(skimT->getTrigValue(trigName));
        if (trigValue) passedHlts_.push_back(trigName);
    }

    return !passedHlts_.empty();
}

auto PickEvent::passHltWithPt(const std::shared_ptr<SkimTree>& skimT,
                              const double& pt) -> bool
{
    printDebug("<- PickEvent::passHltWithPt ->");

    passedHlt_.clear();
    std::vector<std::string> matched;

    const auto& trigs = hlt_.getTrigMapRangePt();

    for (const auto& [trigName, trigRangePt] : trigs) {
        const int trigValue = static_cast<int>(skimT->getTrigValue(trigName));
        if (!trigValue) continue;

        if (pt >= trigRangePt.ptMin && pt < trigRangePt.ptMax) {
            matched.push_back(trigName);
            printDebug(trigName + ", pt = " + std::to_string(pt) +
                       " : " + std::to_string(trigValue));
        }
    }

    if (matched.empty()) return false;

    if (matched.size() > 1) {
        std::ostringstream warn;
        warn << "PickEvent::passHltWithPt - Multiple HLTs matched. "
             << "pt=" << pt
             << ", matched: [" << joinStrings(matched)
             << "]. Using the first one.";
        printWarn(warn.str());
    }

    passedHlt_ = matched.front();
    return true;
}

auto PickEvent::passHltWithPtEta(const std::shared_ptr<SkimTree>& skimT,
                                 const double& pt,
                                 const double& eta) -> bool
{
    printDebug("<- PickEvent::passHltWithPtEta ->");

    passedHlt_.clear();
    std::vector<std::string> matched;

    const auto& trigs = hlt_.getTrigMapRangePtEta();
    const double absEta = std::abs(eta);

    for (const auto& [trigName, trigRangePtEta] : trigs) {
        const int trigValue = static_cast<int>(skimT->getTrigValue(trigName));
        if (!trigValue) continue;

        if (pt >= trigRangePtEta.ptMin &&
            pt <  trigRangePtEta.ptMax &&
            absEta >= trigRangePtEta.absEtaMin &&
            absEta <  trigRangePtEta.absEtaMax) {

            matched.push_back(trigName);
            printDebug(trigName + ", pt = " + std::to_string(pt) +
                       ", eta = " + std::to_string(eta) +
                       " : " + std::to_string(trigValue));
        }
    }

    if (matched.empty()) return false;

    if (matched.size() > 1) {
        std::ostringstream warn;
        warn << "PickEvent::passHltWithPtEta - Multiple HLTs matched. "
             << "pt=" << pt << ", eta=" << eta
             << ", matched: [" << joinStrings(matched)
             << "]. Using the first one.";
        printWarn(warn.str());
    }

    passedHlt_ = matched.front();
    return true;
}

void PickEvent::loadJetVetoRef() {
    std::cout << "==> PickEvent::loadJetVetoRef()" << '\n';
    try {
        loadedJetVetoRef_ =
            correction::CorrectionSet::from_file(jetVetoJsonPath_)->at(jetVetoName_);
    } catch (const std::exception& e) {
        std::cerr << "\nEXCEPTION: PickEvent::loadJetVetoRef()\n";
        std::cerr << "Check " << jetVetoJsonPath_ << " or " << jetVetoName_ << '\n';
        std::cerr << e.what() << '\n';
        throw std::runtime_error("Failed to load Jet Veto Reference");
    }
}

void PickEvent::loadGoldenLumiJson() {
    std::cout << "==> PickEvent::loadGoldenLumiJson()" << '\n';
    std::ifstream file(goldenLumiJsonPath_);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open golden lumi JSON: " + goldenLumiJsonPath_);
    }
    file >> loadedGoldenLumiJson_;
}

//============================================================
// Golden lumi
//============================================================

bool PickEvent::passGoodLumi(unsigned int run, unsigned int lumi) const {
    if (!isData_) return true; // MC always passes

    if (isDebug_) {
        std::cout<<"PickEvent::passGoodLumi:\n";
        std::cout << "Run = " << run << ", Lumi = " << lumi<<"\n";
    }
    try {
        auto it = loadedGoldenLumiJson_.find(std::to_string(run));
        if (it == loadedGoldenLumiJson_.end()) {
            if (isDebug_) {
                std::cout << "Run " << run << " not in golden JSON\n";
            }
            return false;
        }
        for (const auto& lumiBlock : it.value()) {
            if (lumi >= lumiBlock[0] && lumi <= lumiBlock[1]) {
                return true;
            }
        }
    } catch (const std::exception& e) {
        std::cout << "\nEXCEPTION: PickEvent::passGoodLumi()\n";
        std::cout << e.what() << '\n';
        throw std::runtime_error("Error checking good luminosity.");
    }

    return false;
}

//============================================================
// Jet veto map
//============================================================

bool PickEvent::passJetVetoMap(const SkimTree& skimT) const {
    const double maxEtaInMap = 5.191;
    const double maxPhiInMap = 3.1415926;

    try {
        for (int i = 0; i != skimT.nJet; ++i) {
            if (std::abs(skimT.Jet_eta[i]) > maxEtaInMap) continue;
            if (std::abs(skimT.Jet_phi[i]) > maxPhiInMap) continue;
            if (!pickJet_.passId(skimT, i, jetIdLabel_)) continue;

            auto jvNumber =
                loadedJetVetoRef_->evaluate({jetVetoKey_,
                                             skimT.Jet_eta[i],
                                             skimT.Jet_phi[i]});

            if (isDebug_) {
                std::cout << jetVetoKey_
                          << ", jetEta = " << skimT.Jet_eta[i]
                          << ", jetPhi = " << skimT.Jet_phi[i]
                          << ", jetVetoNumber = " << jvNumber << '\n';
            }

            if (jvNumber > 0) {
                // veto the event
                return false;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "\nEXCEPTION: PickEvent::passJetVetoMap(): "
                  << e.what() << '\n';
        throw std::runtime_error("Failed to check Jet Veto Map");
    }

    // not vetoed
    return true;
}

bool PickEvent::passJetVetoMapOnProbe(const TLorentzVector& p4Probe) const {
    try {
        const auto jvNumber =
            loadedJetVetoRef_->evaluate({jetVetoKey_,
                                         p4Probe.Eta(),
                                         p4Probe.Phi()});

        if (isDebug_) {
            std::cout << jetVetoKey_
                      << ", jetEta = " << p4Probe.Eta()
                      << ", jetPhi = " << p4Probe.Phi()
                      << ", jetVetoNumber = " << jvNumber << '\n';
        }

        // true = NOT vetoed
        return (jvNumber <= 0);
    }
    catch (const std::exception& e) {
        std::cerr << "\nWARNING: PickEvent::passJetVetoMapOnProbe(): "
                  << e.what()
                  << "\n→ Gracefully allowing event (returning true)\n";

        // Fail-open: do NOT veto the event if map evaluation fails
        return true;
    }
}


bool PickEvent::passPtHatFilter(const SkimTree& skimT, double leadJetPt) const {
    if (!doPtHatFilter_ || !globalFlags_.isMC()) {
        return true;
    }

    const bool isMG   = globalFlags_.isMG();
    const bool isRun3 = globalFlags_.isRun3();

    const double pthatmax = skimT.Pileup_pthatmax;
    const double lheHT    = skimT.LHE_HT;
    const double genBin   = skimT.Generator_binvar;

    // Protect divisions / pathological values
    if (isMG && isRun3) {
        if (lheHT <= 0.0) {
            printDebug("Run3 MG: LHE_HT <= 0, failing filter for safety.");
            return false;
        }
    }

    // --- Your logic, verbatim in structure ---
    if (isMG && !isRun3 && (2.0 * pthatmax > lheHT)) {
        printDebug("Fail: MG !Run3: 2*Pileup_pthatmax > LHE_HT");
        return false;
    }

    if (isMG && isRun3) {
        const double lhs = 2.0 * leadJetPt / lheHT;
        const double rhs = 2.5 / std::pow(lheHT / 40.0, 2) + 1.5;

        if (lhs > rhs) {
            printDebug("Fail: MG Run3 patch: 2*leadJetPt/LHE_HT > 2.5/pow(LHE_HT/40,2)+1.5");
            return false; // Run3 MG patch for missing Pileup_pthatmax
        }
    }

    if (!isMG && (pthatmax > genBin)) {
        printDebug("Fail: !MG: Pileup_pthatmax > Generator_binvar");
        return false;
    }

    return true;
}

bool PickEvent::passPtHatFilterAuto(const SkimTree& skimT, double leadJetPt) {
    if (!doPtHatFilter_ || !globalFlags_.isMC()) return true;

    const double pthatmax = skimT.Pileup_pthatmax;
    const double lheHT    = skimT.LHE_HT;
    const double genBin   = skimT.Generator_binvar;

    countWarns_++;
    auto dbg = [&](const std::string& msg) {
        if(countWarns_ < warnNtimes_ ){
        std::cout
            << "[PickEvent::passPtHatFilter] " << msg
            << " | leadJetPt=" << leadJetPt
            << " pthatmax=" << pthatmax
            << " lheHT=" << lheHT
            << " genBin=" << genBin
            << "\n";
        }
    };

    // Treat "usable" as positive and finite
    const bool hasPthat  = std::isfinite(pthatmax) && (pthatmax > 0.0);
    const bool hasLheHT  = std::isfinite(lheHT)    && (lheHT > 0.0);
    const bool hasGenBin = std::isfinite(genBin)   && (genBin > 0.0);

    // ------------------------------------------------------------
    // Branching WITHOUT isMG / isRun3:
    // Decide from which inputs are actually available/meaningful
    // ------------------------------------------------------------

    // Case 1: Generator binning is present → non-MG-style filter
    // (matches your original: !isMG && (pthatmax > genBin))
    if (hasGenBin && hasPthat) {
        dbg("Branch: genBin+pthatmax present → using pthatmax > genBin");
        if (pthatmax > genBin) {
            dbg("Fail: pthatmax > genBin");
            return false;
        }
        return true;
    }

    // Case 2: pthatmax and LHE_HT are present → MG legacy filter
    // (matches: isMG && !isRun3 && (2*pthatmax > lheHT))
    else if (hasPthat && hasLheHT) {
        dbg("Branch: pthatmax+LHE_HT present → using 2*pthatmax > LHE_HT");
        if (2.0 * pthatmax > lheHT) {
            dbg("Fail: 2*pthatmax > LHE_HT");
            return false;
        }
        return true;
    }

    // Case 3: LHE_HT present but pthatmax missing/invalid → MG Run3 patch
    // (matches: isMG && isRun3 patch)
    else if (hasLheHT && !hasPthat) {
        dbg("[WARN] Branch: LHE_HT present but pthatmax missing → using Run3 MG patch");

        // Safety: if leadJetPt is pathological, you can decide to pass or fail.
        // I prefer "pass with warning" unless you want strict behavior.
        if (!std::isfinite(leadJetPt) || leadJetPt <= 0.0) {
            dbg("Warn: leadJetPt not usable; passing filter (cannot apply patch safely).");
            return true;
        }

        const double lhs = 2.0 * leadJetPt / lheHT;
        const double rhs = 2.5 / std::pow(lheHT / 40.0, 2) + 1.5;

        if (lhs > rhs) {
            dbg("Fail: patch (2*leadJetPt/LHE_HT > 2.5/pow(LHE_HT/40,2)+1.5)");
            return false;
        }
        return true;
    }

    // Case 4: Not enough info to apply any filter
    else if(countWarns_ < warnNtimes_ ){
        std::cout<<"[WARN] insufficient/invalid inputs to apply pThat filter; passing.\n";
        return true;
    }
    return true;
}

bool PickEvent::passMatchedGenVtx(const SkimTree& skimT) const{
    bool passDiff = true;
    if(isMC_){
        if(std::fabs(skimT.PV_z - skimT.GenVtx_z) >= maxDiffPVzGenVtxz_){
            passDiff = false;
        }
    }
    return passDiff;
}


