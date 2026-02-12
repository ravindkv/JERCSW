#pragma once

#include <cmath>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include <TH1.h>

namespace HistGuard {

// -----------------------------
// Branch prediction helpers
// -----------------------------
inline bool likely(bool x)   { return __builtin_expect(!!x, 1); }
inline bool unlikely(bool x) { return __builtin_expect(!!x, 0); }

// Small utility
inline bool isFinite(double x) { return std::isfinite(x); }

// -----------------------------
// warnOnce (thread-safe)
// -----------------------------
inline void warnOnce(const std::string& tag, const std::string& msg)
{
    // Note: function-local statics are initialized thread-safely since C++11
    static std::mutex mtx;
    static std::unordered_set<std::string> seen;

    const std::string key = tag + "::" + msg;

    std::lock_guard<std::mutex> lk(mtx);
    if (seen.insert(key).second) {
        std::cerr << "[" << tag << "] " << msg << "\n";
    }
}

// -----------------------------
// safeFill
// -----------------------------
inline void safeFill(TH1* h, double x, const std::string& tag, const std::string& ctx)
{
    if (!h) return;
    if (likely(isFinite(x))) {
        h->Fill(x);
        return;
    }
    // slow path only
    warnOnce(tag, "Non-finite value in " + ctx);
}

// -----------------------------
// Histogram lookup (template)
// -----------------------------
template <typename THist>
inline THist* getHistOrWarn(std::unordered_map<std::string, std::unique_ptr<THist>>& m,
                            const std::string& key,
                            const std::string& tag,
                            const std::string& what)
{
    auto it = m.find(key);
    if (it == m.end() || !it->second) {
        warnOnce(tag, "Missing histogram for key='" + key + "' (" + what + ")");
        return nullptr;
    }
    return it->second.get();
}

} // namespace HistGuard

