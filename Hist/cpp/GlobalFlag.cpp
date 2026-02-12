#include "GlobalFlag.h"

#include <ostream>
#include <stdexcept>
#include <utility>

namespace {
template <typename EnumT>
[[nodiscard]] constexpr bool isNone(EnumT v, EnumT noneVal) noexcept {
    return v == noneVal;
}
}  // namespace

GlobalFlag::GlobalFlag(std::string ioName)
    : ioName_(std::move(ioName))
{
    parseFlags();
    deriveNanoVersionFromJetAlgo();
    validateFlagsOrThrow();
}

// ---------------------------------------------------------
// Token helpers
// ---------------------------------------------------------
std::string_view GlobalFlag::stripKnownExtension(std::string_view s) {
    constexpr std::string_view kRoot = ".root";
    if (s.size() >= kRoot.size() && s.substr(s.size() - kRoot.size()) == kRoot) {
        s.remove_suffix(kRoot.size());
    }
    return s;
}

std::vector<std::string_view> GlobalFlag::splitTokens(std::string_view s, char delim) {
    s = stripKnownExtension(s);

    std::vector<std::string_view> out;
    out.reserve(16);

    std::size_t start = 0;
    while (start < s.size()) {
        std::size_t end = s.find(delim, start);
        if (end == std::string_view::npos) end = s.size();
        out.emplace_back(s.substr(start, end - start));
        start = end + 1;
    }
    return out;
}

bool GlobalFlag::hasToken(const std::vector<std::string_view>& toks, std::string_view t) noexcept {
    for (auto x : toks) {
        if (x == t) return true;
    }
    return false;
}

// ---------------------------------------------------------
// Year / Era token parsing (whole-token only)
// ---------------------------------------------------------
GlobalFlag::Year GlobalFlag::parseYearToken(std::string_view tok) noexcept {
    if (tok == "2016Pre")  return Year::Year2016Pre;
    if (tok == "2016Post") return Year::Year2016Post;
    if (tok == "2017")     return Year::Year2017;
    if (tok == "2018")     return Year::Year2018;
    return Year::NONE;
}

GlobalFlag::Era GlobalFlag::parseEraToken(std::string_view tok) noexcept {
    // 2016Pre/2016Post era tokens (your naming style)
    if (tok == "2016PreB" || tok == "2016PreC" || tok == "2016PreD") return Era::Era2016PreBCD;
    if (tok == "2016PreE" || tok == "2016PreF")                       return Era::Era2016PreEF;

    if (tok == "2016PostF" || tok == "2016PostG" || tok == "2016PostH") return Era::Era2016PostFGH;

    // 2017
    if (tok == "2017B") return Era::Era2017B;
    if (tok == "2017C") return Era::Era2017C;
    if (tok == "2017D") return Era::Era2017D;
    if (tok == "2017E") return Era::Era2017E;
    if (tok == "2017F") return Era::Era2017F;

    // 2018
    if (tok == "2018A") return Era::Era2018A;
    if (tok == "2018B") return Era::Era2018B;
    if (tok == "2018C") return Era::Era2018C;
    if (tok == "2018D") return Era::Era2018D;

    return Era::NONE;
}

// ---------------------------------------------------------
// Parsing: strict token-based
// ---------------------------------------------------------
void GlobalFlag::parseFlags() {
    const auto toks = splitTokens(ioName_, '_');

    if(ioName_.find("Closure") != std::string::npos){
        isClosure_ = true;
    }else{
        isClosure_ = false;
    }

    // JetAlgo
    if      (hasToken(toks, "AK4Chs"))   jetAlgo_ = JetAlgo::AK4Chs;
    else if (hasToken(toks, "AK4Puppi")) jetAlgo_ = JetAlgo::AK4Puppi;
    else if (hasToken(toks, "AK8Puppi")) jetAlgo_ = JetAlgo::AK8Puppi;

    // JecDerivationLevel
    if      (hasToken(toks, "L2Residual")) jecDerivationLevel_ = JecDerivationLevel::L2Residual;
    else if (hasToken(toks, "L3Residual")) jecDerivationLevel_ = JecDerivationLevel::L3Residual;
    else if (hasToken(toks, "JerSF")) jecDerivationLevel_ = JecDerivationLevel::JerSF;

    // Channel
    if      (hasToken(toks, "DiJet"))      channel_ = Channel::DiJet;
    else if (hasToken(toks, "ZeeJet"))     channel_ = Channel::ZeeJet;
    else if (hasToken(toks, "ZmmJet"))     channel_ = Channel::ZmmJet;
    else if (hasToken(toks, "GamJetFake")) channel_ = Channel::GamJetFake;
    else if (hasToken(toks, "GamJet"))     channel_ = Channel::GamJet;
    else if (hasToken(toks, "MultiJet"))   channel_ = Channel::MultiJet;
    else if (hasToken(toks, "Wqqe"))       channel_ = Channel::Wqqe;
    else if (hasToken(toks, "Wqqm"))       channel_ = Channel::Wqqm;

    // Data/MC: mutually exclusive parse (validation later)
    if      (hasToken(toks, "Data")) isData_ = true;
    else if (hasToken(toks, "MC"))   isMC_   = true;

    // Systematic: allow both "Base" and your "HistBase"
    if      (hasToken(toks, "IsrUp"))    syst_ = Systematic::IsrUp;
    else if (hasToken(toks, "IsrDown"))  syst_ = Systematic::IsrDown;
    else if (hasToken(toks, "FsrUp"))    syst_ = Systematic::FsrUp;
    else if (hasToken(toks, "FsrDown"))  syst_ = Systematic::FsrDown;
    else if (hasToken(toks, "PdfUp"))    syst_ = Systematic::PdfUp;
    else if (hasToken(toks, "PdfDown"))  syst_ = Systematic::PdfDown;
    else if (hasToken(toks, "QsqrUp"))   syst_ = Systematic::QsqrUp;
    else if (hasToken(toks, "QsqrDown")) syst_ = Systematic::QsqrDown;
    else if (hasToken(toks, "HistBase") || hasToken(toks, "Base")) syst_ = Systematic::Base;
    else syst_ = Systematic::NONE;

    // Era / Year tokens:
    // Prefer an explicit era token (2018A etc.). If absent (common in MC), accept year-only token (2018).
    for (auto t : toks) {
        if (era_ == Era::NONE) {
            const Era e = parseEraToken(t);
            if (e != Era::NONE) era_ = e;
        }
        if (year_ == Year::NONE) {
            const Year y = parseYearToken(t);
            if (y != Year::NONE) year_ = y;
        }
    }

    // If era gives us year, prefer that (more specific)
    deriveYearFromEraIfPossible();

    // Samples: token-based (no substring traps)
    if (hasToken(toks, "QCD")) isQCD_ = true;

    // Future use: MG token should be explicit
    // (You can expand later to "MadGraph", "MG", "amcatnlo", etc. as separate tokens.)
    if (hasToken(toks, "MG") || hasToken(toks, "MadGraph") || hasToken(toks, "Madgraph")) isMG_ = true;

    // Currently manual bookkeeping flag, keep behavior
    isBookKeep_ = true;

    if(jecDerivationLevel_==JecDerivationLevel::L2Residual){
        jecApplicationLevel_ = JecApplicationLevel::L2Rel;
        if(isClosure_) jecApplicationLevel_ = JecApplicationLevel::L2Res;
    }else if(jecDerivationLevel_==JecDerivationLevel::L3Residual){
        jecApplicationLevel_ = JecApplicationLevel::L2Res;
        if(isClosure_) jecApplicationLevel_ = JecApplicationLevel::L2L3Res;
    }else if(jecDerivationLevel_==JecDerivationLevel::JerSF){
        jecApplicationLevel_ = JecApplicationLevel::L2L3Res;
        if(isClosure_){
            jecApplicationLevel_ = JecApplicationLevel::JerSF;
            if(isMC_) applyJer_ = true;
        }
    }else jecApplicationLevel_ = JecApplicationLevel::NONE;
    
}

// ---------------------------------------------------------
// Derived info
// ---------------------------------------------------------
void GlobalFlag::deriveNanoVersionFromJetAlgo() noexcept {
    switch (jetAlgo_) {
        case JetAlgo::AK4Chs:   nanoVer_ = NanoVersion::V9;  break;
        case JetAlgo::AK4Puppi:
        case JetAlgo::AK8Puppi: nanoVer_ = NanoVersion::V15; break;
        default:                nanoVer_ = NanoVersion::Unknown; break;
    }
}

void GlobalFlag::deriveYearFromEraIfPossible() noexcept {
    switch (era_) {
        case Era::Era2016PreBCD:
        case Era::Era2016PreEF:   year_ = Year::Year2016Pre;  break;

        case Era::Era2016PostFGH: year_ = Year::Year2016Post; break;

        case Era::Era2017B:
        case Era::Era2017C:
        case Era::Era2017D:
        case Era::Era2017E:
        case Era::Era2017F:       year_ = Year::Year2017;     break;

        case Era::Era2018A:
        case Era::Era2018B:
        case Era::Era2018C:
        case Era::Era2018D:       year_ = Year::Year2018;     break;

        default: break;  // keep whatever year-only token gave us
    }
}

// ---------------------------------------------------------
// Validation: fail fast (core framework)
// ---------------------------------------------------------
void GlobalFlag::validateFlagsOrThrow() const {
    // Exactly one of Data/MC must be true
    if (isData_ == isMC_) {
        throw std::runtime_error("GlobalFlag: expected exactly one token among {Data, MC}. ioName=" + ioName_);
    }

    // Required fields
    if (isNone(jetAlgo_, JetAlgo::NONE)) {
        throw std::runtime_error("GlobalFlag: JetAlgo token not found. ioName=" + ioName_);
    }
    if (isNone(jecDerivationLevel_, JecDerivationLevel::NONE)) {
        throw std::runtime_error("GlobalFlag: JecDerivationLevel token not found. ioName=" + ioName_);
    }
    if (isNone(jecApplicationLevel_, JecApplicationLevel::NONE)) {
        throw std::runtime_error("GlobalFlag: JecApplicationLevel token not found. ioName=" + ioName_);
    }
    if (isNone(channel_, Channel::NONE)) {
        throw std::runtime_error("GlobalFlag: Channel token not found. ioName=" + ioName_);
    }

    // Year must exist for all (MC can be year-only, Data usually has era)
    if (isNone(year_, Year::NONE)) {
        throw std::runtime_error("GlobalFlag: Year token not found (need e.g. 2018 or 2018A...). ioName=" + ioName_);
    }

    // Era rules:
    // - Data SHOULD have an era (2018A...), because you typically run per-era datasets.
    // - MC MAY be year-only (2018), allow era NONE.
    if (isData_ && isNone(era_, Era::NONE)) {
        throw std::runtime_error("GlobalFlag: Data requires an era token (e.g. 2018A). ioName=" + ioName_);
    }

    // NanoVersion should be determinable for supported algos
    if (nanoVer_ == NanoVersion::Unknown) {
        throw std::runtime_error("GlobalFlag: NanoVersion is Unknown (unexpected JetAlgo token). ioName=" + ioName_);
    }

    // Debug safety: avoid negative nDebug
    if (nDebug_ < 0) {
        throw std::runtime_error("GlobalFlag: nDebug < 0 is invalid. ioName=" + ioName_);
    }
}

// ---------------------------------------------------------
// String helpers (single source of truth)
// ---------------------------------------------------------
std::string GlobalFlag::getJetAlgoStr() const {
    switch (jetAlgo_) {
        case JetAlgo::AK4Chs:   return "AK4Chs";
        case JetAlgo::AK4Puppi: return "AK4Puppi";
        case JetAlgo::AK8Puppi: return "AK8Puppi";
        default:                return "NONE";
    }
}

std::string GlobalFlag::getJecDerivationLevelStr() const {
    switch (jecDerivationLevel_) {
        case JecDerivationLevel::L2Residual: return "L2Residual";
        case JecDerivationLevel::L3Residual: return "L3Residual";
        case JecDerivationLevel::JerSF: return "JerSF";
        default:                  return "NONE";
    }
}

std::string GlobalFlag::getJecApplicationLevelStr() const {
    switch (jecApplicationLevel_) {
        case JecApplicationLevel::L1Rc:     return "L1Rc";
        case JecApplicationLevel::L2Rel:    return "L2Rel";
        case JecApplicationLevel::L2Res:    return "L2Res";
        case JecApplicationLevel::L2L3Res:  return "L2L3Res";
        case JecApplicationLevel::JerSF:  return "JerSF";
        default:                 return "Unknown";
    }
}
std::string GlobalFlag::getChannelStr() const {
    switch (channel_) {
        case Channel::DiJet:      return "DiJet";
        case Channel::ZeeJet:     return "ZeeJet";
        case Channel::ZmmJet:     return "ZmmJet";
        case Channel::GamJetFake: return "GamJetFake";
        case Channel::GamJet:     return "GamJet";
        case Channel::MultiJet:   return "MultiJet";
        case Channel::Wqqe:       return "Wqqe";
        case Channel::Wqqm:       return "Wqqm";
        default:                  return "NONE";
    }
}

std::string GlobalFlag::getYearStr() const {
    switch (year_) {
        case Year::Year2016Pre:  return "2016Pre";
        case Year::Year2016Post: return "2016Post";
        case Year::Year2017:     return "2017";
        case Year::Year2018:     return "2018";
        default:                 return "NONE";
    }
}

std::string GlobalFlag::getEraStr() const {
    // Preserving your intentional merging choice for 2016Pre.
    switch (era_) {
        case Era::Era2016PreBCD:  return "Era2016PreBCDEF";
        case Era::Era2016PreEF:   return "Era2016PreBCDEF";
        case Era::Era2016PostFGH: return "Era2016PostFGH";
        case Era::Era2017B:       return "Era2017B";
        case Era::Era2017C:       return "Era2017C";
        case Era::Era2017D:       return "Era2017D";
        case Era::Era2017E:       return "Era2017E";
        case Era::Era2017F:       return "Era2017F";
        case Era::Era2018A:       return "Era2018A";
        case Era::Era2018B:       return "Era2018B";
        case Era::Era2018C:       return "Era2018C";
        case Era::Era2018D:       return "Era2018D";
        default:                  return "NONE";
    }
}

std::string GlobalFlag::getDataStr() const { return isData_ ? "Data" : ""; }
std::string GlobalFlag::getMcStr() const { return isMC_ ? "MC" : ""; }

std::string GlobalFlag::getDirStr() const {
    switch (syst_) {
        case Systematic::IsrUp:    return "IsrUp";
        case Systematic::IsrDown:  return "IsrDown";
        case Systematic::FsrUp:    return "FsrUp";
        case Systematic::FsrDown:  return "FsrDown";
        case Systematic::PdfUp:    return "PdfUp";
        case Systematic::PdfDown:  return "PdfDown";
        case Systematic::QsqrUp:   return "QsqrUp";
        case Systematic::QsqrDown: return "QsqrDown";
        case Systematic::Base:     return "Base";
        case Systematic::NONE:     return "NONE";
        default:                   return "NONE";
    }
}

double GlobalFlag::getLumiPerYear() const {
    // Keep your values; use 1.0 only as a last-resort fallback.
    switch (year_) {
        case Year::Year2016Pre:  return 19.5;
        case Year::Year2016Post: return 16.8;
        case Year::Year2017:     return 41.5;
        case Year::Year2018:     return 59.8;
        default:                 return 1.0;
    }
}

// ---------------------------------------------------------
// Printing (no duplicated switches)
// ---------------------------------------------------------
void GlobalFlag::printFlags(std::ostream& os) const {
    //os << "---- GlobalFlag ----\n";
    os << "ioName     = " << ioName_ << "\n";

    os << "debug      = " << (isDebug_ ? "true" : "false") << "\n";
    os << "nDebug     = " << nDebug_ << "\n";
    os << "Data/MC    = " << (isData_ ? "Data" : "MC") << "\n";

    os << "JetAlgo    = " << getJetAlgoStr() << "\n";
    os << "JecDerivationLevel    = " << getJecDerivationLevelStr() << "\n";
    os << "closure    = " << (isClosure_ ? "true" : "false") << "\n";
    os << "JecApplicationLevel   = " << getJecApplicationLevelStr() << "\n";
    os << "Channel    = " << getChannelStr() << "\n";

    os << "Year       = " << getYearStr() << "\n";
    os << "Era        = " << getEraStr() << "\n";

    os << "Systematic = " << getDirStr() << "\n";

    os << "NanoAOD    = ";
    switch (nanoVer_) {
        case NanoVersion::V9:  os << "V9";  break;
        case NanoVersion::V15: os << "V15"; break;
        default:               os << "Unknown"; break;
    }
    os << "\n";

    if (isQCD_) os << "Sample     = QCD\n";
    if (isMG_)  os << "Sample     = MG (token matched)\n";
    if (isRun3_)  os << "Run      = Run3\n";

    os << "BookKeep   = " << (isBookKeep_ ? "true" : "false") << "\n";
    os << "--------------------\n";
}

