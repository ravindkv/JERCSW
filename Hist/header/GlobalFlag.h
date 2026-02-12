#pragma once

#include <cstdint>
#include <iosfwd>
#include <string>
#include <string_view>
#include <vector>

class GlobalFlag {
public:
    // -----------------------------
    // Strong enums
    // -----------------------------
    enum class NanoVersion : std::uint8_t { Unknown, V9, V15 };

    enum class JetAlgo : std::uint8_t { NONE, AK4Chs, AK4Puppi, AK8Puppi };

    enum class JecDerivationLevel : std::uint8_t { 
        NONE, 
        L2Residual, 
        L3Residual,
        JerSF
        };

    enum class JecApplicationLevel {
        NONE,
        L1Rc,
        L2Rel,
        L2Res,
        L2L3Res,
        JerSF
    };

    enum class Channel : std::uint8_t { 
        NONE, 
        DiJet, 
        ZeeJet, 
        ZmmJet, 
        GamJet, 
        GamJetFake,
        MultiJet, 
        Wqqe, 
        Wqqm 
    };

    enum class Year : std::uint8_t { 
        NONE, 
        Year2016Pre, 
        Year2016Post,
        Year2017, 
        Year2018 
    };

    enum class Era : std::uint8_t {
        NONE,
        Era2016PreBCD,
        Era2016PreEF,
        Era2016PostFGH,
        Era2017B,
        Era2017C,
        Era2017D,
        Era2017E,
        Era2017F,
        Era2018A,
        Era2018B,
        Era2018C,
        Era2018D
    };

    enum class Systematic : std::uint8_t {
        NONE,
        Base,
        IsrUp,
        IsrDown,
        FsrUp,
        FsrDown,
        PdfUp,
        PdfDown,
        QsqrUp,
        QsqrDown
    };

    // -----------------------------
    // Ctors / rule-of-5
    // -----------------------------
    explicit GlobalFlag(std::string ioName);
    ~GlobalFlag() = default;

    GlobalFlag(const GlobalFlag&)            = delete;
    GlobalFlag& operator=(const GlobalFlag&) = delete;

    GlobalFlag(GlobalFlag&&) noexcept            = default;
    GlobalFlag& operator=(GlobalFlag&&) noexcept = default;

    // -----------------------------
    // User-controlled runtime flags
    // -----------------------------
    void setDebug(bool debug) noexcept { isDebug_ = debug; }
    void setNDebug(int nDebug) noexcept { nDebug_ = nDebug; }

    [[nodiscard]] bool isDebug() const noexcept { return isDebug_; }
    [[nodiscard]] int  getNDebug() const noexcept { return nDebug_; }
    [[nodiscard]] bool isClosure() const noexcept { return isClosure_; }

    // -----------------------------
    // Parsed flags (core)
    // -----------------------------
    [[nodiscard]] Year        getYear() const noexcept { return year_; }
    [[nodiscard]] Era         getEra() const noexcept { return era_; }
    [[nodiscard]] bool        isData() const noexcept { return isData_; }
    [[nodiscard]] bool        isMC() const noexcept { return isMC_; }
    [[nodiscard]] NanoVersion getNanoVersion() const noexcept { return nanoVer_; }
    [[nodiscard]] JetAlgo     getJetAlgo() const noexcept { return jetAlgo_; }
    [[nodiscard]] JecDerivationLevel     getJecDerivationLevel() const noexcept { return jecDerivationLevel_; }
    [[nodiscard]] JecApplicationLevel     getJecApplicationLevel() const noexcept { return jecApplicationLevel_; }
    [[nodiscard]] Channel     getChannel() const noexcept { return channel_; }
    [[nodiscard]] Systematic  getSystematic() const noexcept { return syst_; }

    [[nodiscard]] bool isQCD() const noexcept { return isQCD_; }
    [[nodiscard]] bool isMG() const noexcept { return isMG_; }          // token-based, future use
    [[nodiscard]] bool isRun3() const noexcept { return isRun3_; }          // token-based, future use
    [[nodiscard]] bool isBookKeep() const noexcept { return isBookKeep_; }
    [[nodiscard]] bool applyJer() const noexcept { return applyJer_; }

    // -----------------------------
    // String helpers
    // -----------------------------
    [[nodiscard]] std::string getJetAlgoStr() const;
    [[nodiscard]] std::string getJecDerivationLevelStr() const;
    [[nodiscard]] std::string getJecApplicationLevelStr() const;
    [[nodiscard]] std::string getChannelStr() const;
    [[nodiscard]] std::string getYearStr() const;
    [[nodiscard]] std::string getEraStr() const;       // preserves your 2016Pre merging choice
    [[nodiscard]] std::string getDataStr() const;      // "Data" or ""
    [[nodiscard]] std::string getMcStr() const;        // "MC" or ""
    [[nodiscard]] std::string getDirStr() const;       // systematic dir token
    [[nodiscard]] double      getLumiPerYear() const;  // Run-2 numbers you provided

    // Print all active flags (single source of truth)
    void printFlags(std::ostream& os) const;

private:
    // -----------------------------
    // Core parsing
    // -----------------------------
    void parseFlags();
    void deriveNanoVersionFromJetAlgo() noexcept;
    void deriveYearFromEraIfPossible() noexcept;
    void validateFlagsOrThrow() const;

    // Tokenization (no substring matching)
    static std::vector<std::string_view> splitTokens(std::string_view s, char delim);
    static std::string_view stripKnownExtension(std::string_view s);
    static bool hasToken(const std::vector<std::string_view>& toks, std::string_view t) noexcept;

    // “year-only MC” support: detect tokens like "2018" (no era letter)
    static Year parseYearToken(std::string_view tok) noexcept;
    static Era  parseEraToken(std::string_view tok) noexcept;

private:
    std::string ioName_;

    bool isDebug_   = false;
    bool isClosure_ = false;
    int  nDebug_    = 100;

    Year year_ = Year::NONE;
    Era  era_  = Era::NONE;

    bool isData_ = false;
    bool isMC_   = false;

    NanoVersion nanoVer_ = NanoVersion::Unknown;

    JetAlgo jetAlgo_ = JetAlgo::NONE;
    JecDerivationLevel jecDerivationLevel_ = JecDerivationLevel::NONE;
    JecApplicationLevel jecApplicationLevel_ = JecApplicationLevel::NONE;
    Channel channel_ = Channel::NONE;

    Systematic syst_ = Systematic::NONE;

    bool isQCD_      = false;
    bool isMG_       = false;
    bool isRun3_     = false;
    bool isBookKeep_ = false;
    bool applyJer_   = false;
};

