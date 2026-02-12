#pragma once

#include <cstddef>
#include <cmath>
#include <iosfwd>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>

#include <TLorentzVector.h>

class PickGuard {
public:
    enum class Policy {
        Throw,     // throw std::runtime_error
        Warn,      // print every time
        WarnOnce,  // print once per (context + code)
        Silent     // do nothing
    };

    struct Config {
        Policy policy = Policy::WarnOnce;
        std::ostream* out = nullptr; // if null, defaults to std::cerr
    };

    explicit PickGuard(std::string context)
        : PickGuard(std::move(context), Config{}) {}

    PickGuard(std::string context, Config cfg)
        : context_(std::move(context)), cfg_(cfg)
    {
        if (!cfg_.out) cfg_.out = &std::cerr;
    }

    ~PickGuard() = default;

    // ---------------- core ----------------
    inline void require(bool condition, const char* code, const std::string& message) {
        if (likely_(condition)) return;
        report_(code, message);
        if (cfg_.policy == Policy::Throw) {
            throw std::runtime_error(format_(code, message));
        }
    }

    inline void warn(bool condition, const char* code, const std::string& message) {
        if (likely_(condition)) return;
        if (cfg_.policy == Policy::Throw) {
            // downgrade to warning
            Policy saved = cfg_.policy;
            cfg_.policy = Policy::WarnOnce;
            report_(code, message);
            cfg_.policy = saved;
        } else {
            report_(code, message);
        }
    }

    // ---------------- common checks ----------------
    inline void requireNonNull(const void* ptr, const char* name) {
        if (likely_(ptr != nullptr)) return;
        require(false, "NULL_PTR", std::string("null pointer: ") + (name ? name : "UNKNOWN"));
    }

    inline void requireFinite(double x, const char* name) {
        if (likely_(std::isfinite(x))) return;
        // slow path only
        std::ostringstream oss;
        oss << "non-finite value: " << (name ? name : "UNKNOWN") << "=" << x;
        require(false, "NON_FINITE", oss.str());
    }

    inline void requireIndexInRange(int idx, int n, const char* what) {
        if (likely_(idx >= 0 && idx < n)) return;
        std::ostringstream oss;
        oss << "index out of range for " << (what ? what : "UNKNOWN")
            << " idx=" << idx << " n=" << n;
        require(false, "IDX_RANGE", oss.str());
    }

    inline void requireSize(std::size_t got, std::size_t expected, const char* what) {
        if (likely_(got == expected)) return;
        std::ostringstream oss;
        oss << "unexpected size for " << (what ? what : "UNKNOWN")
            << " got=" << got << " expected=" << expected;
        require(false, "BAD_SIZE", oss.str());
    }

    inline void requireAllFinite(const std::vector<TLorentzVector>& v, const char* what) {
        for (std::size_t i = 0; i < v.size(); ++i) {
            const auto& p4 = v[i];

            const double pt  = p4.Pt();
            const double eta = p4.Eta();
            const double phi = p4.Phi();
            const double m   = p4.M();

            if (likely_(std::isfinite(pt) && std::isfinite(eta) && std::isfinite(phi) && std::isfinite(m))) {
                continue;
            }

            std::ostringstream oss;
            oss << "non-finite TLorentzVector in " << (what ? what : "UNKNOWN")
                << " i=" << i
                << " (pt,eta,phi,m)=(" << pt << "," << eta << "," << phi << "," << m << ")";
            require(false, "P4_NON_FINITE", oss.str());
            return;
        }
    }

    // ---------------- helpers ----------------
    inline TLorentzVector makeP4(double pt, double eta, double phi, double mass, const char* what) {
        // Avoid allocating "what.pt" strings; format only on failure.
        requireFiniteNamed_(pt,   what, "pt");
        requireFiniteNamed_(eta,  what, "eta");
        requireFiniteNamed_(phi,  what, "phi");
        requireFiniteNamed_(mass, what, "mass");

        TLorentzVector p4;
        p4.SetPtEtaPhiM(pt, eta, phi, mass);
        return p4;
    }

    // ---------------- common Pick* contracts ----------------
    inline void requireJetTriplet(const std::vector<TLorentzVector>& jets,
                                  const std::vector<int>& indices,
                                  int nJet)
    {
        requireSize(jets.size(), 3, "jetsP4");
        requireAllFinite(jets, "jetsP4");
        for (int idx : indices) {
            requireIndexInRange(idx, nJet, "jetIndex");
        }
    }

private:
    std::string context_;
    mutable Config cfg_;

    static inline bool likely_(bool x)   { return __builtin_expect(!!x, 1); }
    static inline bool unlikely_(bool x) { return __builtin_expect(!!x, 0); }

    // warn-once registry (header-only, process-wide)
    static inline std::mutex& onceMutex_() {
        static std::mutex m;
        return m;
    }
    static inline std::unordered_set<std::string>& onceKeys_() {
        static std::unordered_set<std::string> s;
        return s;
    }

    inline void requireFiniteNamed_(double x, const char* what, const char* field) {
        if (likely_(std::isfinite(x))) return;
        std::ostringstream oss;
        oss << "non-finite value: " << (what ? what : "obj") << "." << (field ? field : "field")
            << "=" << x;
        require(false, "NON_FINITE", oss.str());
    }

    inline void report_(const char* code, const std::string& message) {
        if (cfg_.policy == Policy::Silent) return;

        if (cfg_.policy == Policy::WarnOnce) {
            // Key only built on failure (rare)
            const std::string key = context_ + "|" + (code ? code : "UNKNOWN");
            std::lock_guard<std::mutex> lock(onceMutex_());
            auto& keys = onceKeys_();
            if (keys.find(key) != keys.end()) return;
            keys.insert(key);
        }

        (*cfg_.out) << format_(code, message) << "\n";
    }

    inline std::string format_(const char* code, const std::string& message) const {
        std::ostringstream oss;
        oss << "[PickGuard] " << context_
            << " [" << (code ? code : "UNKNOWN") << "] "
            << message;
        return oss.str();
    }
};

