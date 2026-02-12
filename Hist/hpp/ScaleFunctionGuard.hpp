#pragma once

#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <mutex>
#include <sstream>
#include <string>
#include <unordered_map>

#include "GlobalFlag.h" // header-only: need full type

class ScaleFunctionGuard {
public:
    struct Cfg {
        int maxPrintPerKey = 10;
        bool printInfoInDebug = true;

        double maxAbsEta = 6.0;
        double maxAbsPhi = 3.5;
        double minPt     = 0.0;

        double sfMinDefault = 0.0;
        double sfMaxDefault = 5.0;
    };

    ScaleFunctionGuard(const GlobalFlag& flags,
                       std::string where,
                       uint32_t run = 0,
                       uint32_t lumi = 0,
                       uint64_t event = 0)
        : flags_(flags),
          where_(std::move(where)),
          run_(run),
          lumi_(lumi),
          event_(event),
          isDebug_(flags_.isDebug())
    {}

    // Simple checks
    inline void checkIndex(const char* coll, int idx) const {
        if (idx >= 0) return;
        const std::string key = where_ + ":idxneg:" + (coll ? coll : "coll");
        if (allowPrint_(key, cfg_.maxPrintPerKey)) {
            print_("WARN", std::string(coll ? coll : "coll") + " index is negative: idx=" + std::to_string(idx));
        }
    }

    inline void checkFinite(const char* name, double v) const {
        if (finite_(v)) return;
        const std::string key = where_ + ":nonfinite:" + (name ? name : "var");
        if (allowPrint_(key, cfg_.maxPrintPerKey)) {
            // Note: only runs on failure
            print_("WARN", std::string(name ? name : "var") + " is non-finite: v=" + std::to_string(v));
        }
    }

    inline void checkPtEta(double pt, double eta, const char* objLabel) const {
        const char* obj = objLabel ? objLabel : "obj";

        if (!finite_(pt) || !finite_(eta)) {
            const std::string key = where_ + ":kin_nonfinite:" + obj;
            if (allowPrint_(key, cfg_.maxPrintPerKey)) {
                print_("WARN", std::string(obj) + " has non-finite kinematics: pt=" +
                               std::to_string(pt) + " eta=" + std::to_string(eta));
            }
            return;
        }

        if (pt <= cfg_.minPt) {
            const std::string key = where_ + ":pt_nonpos:" + obj;
            if (allowPrint_(key, cfg_.maxPrintPerKey)) {
                print_("WARN", std::string(obj) + " has non-positive pt: pt=" + std::to_string(pt));
            }
        }

        if (std::abs(eta) > cfg_.maxAbsEta) {
            const std::string key = where_ + ":eta_large:" + obj;
            if (allowPrint_(key, cfg_.maxPrintPerKey)) {
                print_("WARN", std::string(obj) + " has |eta| too large: eta=" + std::to_string(eta));
            }
        }
    }

    inline void checkPtEtaPhi(double pt, double eta, double phi, const char* objLabel) const {
        checkPtEta(pt, eta, objLabel);

        const char* obj = objLabel ? objLabel : "obj";
        if (!finite_(phi)) {
            const std::string key = where_ + ":phi_nonfinite:" + obj;
            if (allowPrint_(key, cfg_.maxPrintPerKey)) {
                print_("WARN", std::string(obj) + " has non-finite phi: phi=" + std::to_string(phi));
            }
            return;
        }

        if (std::abs(phi) > cfg_.maxAbsPhi) {
            const std::string key = where_ + ":phi_large:" + obj;
            if (allowPrint_(key, cfg_.maxPrintPerKey)) {
                print_("WARN", std::string(obj) + " has |phi| too large: phi=" + std::to_string(phi));
            }
        }
    }

    inline void checkSf(const char* name, double sf,
                        double lo = std::numeric_limits<double>::quiet_NaN(),
                        double hi = std::numeric_limits<double>::quiet_NaN()) const
    {
        const char* nm = name ? name : "sf";
        const double minv = std::isnan(lo) ? cfg_.sfMinDefault : lo;
        const double maxv = std::isnan(hi) ? cfg_.sfMaxDefault : hi;

        if (!finite_(sf)) {
            const std::string key = where_ + ":sf_nonfinite:" + nm;
            if (allowPrint_(key, cfg_.maxPrintPerKey)) {
                print_("WARN", std::string(nm) + " is non-finite: sf=" + std::to_string(sf));
            }
            return;
        }

        if (sf < minv || sf > maxv) {
            const std::string key = where_ + ":sf_oob:" + nm;
            if (allowPrint_(key, cfg_.maxPrintPerKey)) {
                print_("WARN", std::string(nm) + " out of expected range [" +
                               std::to_string(minv) + "," + std::to_string(maxv) + "]: sf=" +
                               std::to_string(sf));
            }
        }
    }

    inline void noteDefaultedToOne(const char* name, const char* reason) const {
        const char* nm = name ? name : "var";
        const std::string key = where_ + ":default1:" + nm + ":" + (reason ? reason : "");
        if (allowPrint_(key, cfg_.maxPrintPerKey)) {
            print_("WARN", std::string(nm) + " defaulted to 1.0 (" + (reason ? reason : "unknown") + ")");
        }
    }

    inline void noteClamp(const char* name, double before, double after, const char* reason = nullptr) const {
        if (!finite_(before) || !finite_(after)) return;
        if (before == after) return;

        const double rel = (std::abs(before) > 0.0) ? std::abs((after - before) / before) : std::abs(after - before);
        const bool big = (rel > 0.20);

        const char* nm = name ? name : "var";
        const std::string key = where_ + ":clamp:" + nm + (big ? ":big" : ":small");
        if (!allowPrint_(key, cfg_.maxPrintPerKey)) return;

        std::string msg = std::string(nm) + " clamped: " +
                          std::to_string(before) + " -> " + std::to_string(after);
        if (reason) msg += std::string(" (") + reason + ")";

        if (big) print_("WARN", msg);
        else if (isDebug_ && cfg_.printInfoInDebug) print_("INFO", msg);
    }

    inline void note(const char* msg) const {
        if (!isDebug_ || !cfg_.printInfoInDebug) return;
        const std::string key = where_ + ":note:" + (msg ? msg : "");
        if (allowPrint_(key, cfg_.maxPrintPerKey)) {
            print_("INFO", msg ? msg : "");
        }
    }

    inline void warn(const char* msg) const {
        const std::string key = where_ + ":warn:" + (msg ? msg : "");
        if (allowPrint_(key, cfg_.maxPrintPerKey)) {
            print_("WARN", msg ? msg : "");
        }
    }

private:
    const GlobalFlag& flags_;
    std::string where_;
    uint32_t run_{0};
    uint32_t lumi_{0};
    uint64_t event_{0};
    Cfg cfg_{};
    bool isDebug_{false};

    static inline bool finite_(double x) { return std::isfinite(x); }

    static inline bool allowPrint_(const std::string& key, int maxPrint) {
        static std::mutex mtx;
        static std::unordered_map<std::string, int> counts;

        std::lock_guard<std::mutex> lk(mtx);
        int& c = counts[key];
        if (c < maxPrint) { ++c; return true; }
        return false;
    }

    inline void print_(const char* level, const std::string& text) const {
        std::ostringstream os;
        os << "[" << (level ? level : "INFO") << "] " << where_;
        if (run_ || lumi_ || event_) {
            os << " (run=" << run_ << ", lumi=" << lumi_ << ", event=" << event_ << ")";
        }
        os << ": " << text;
        std::cerr << os.str() << '\n';
    }
};

