#pragma once

#include <cmath>
#include <cstddef>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <TLorentzVector.h> // header-only requires full type

class MathGuard {
public:
    enum class Mode {
        Throw,   // throw std::runtime_error
        Warn,    // write to std::cerr and continue
        Off      // no checks
    };

    MathGuard(std::string className,
              std::string where,
              bool isDebug,
              Mode mode = Mode::Throw)
        : className_(std::move(className)),
          where_(std::move(where)),
          isDebug_(isDebug),
          mode_(mode)
    {}

    // ---- caching (for better crash messages) ----
    inline MathGuard& cacheScalar(const std::string& name, double v) {
        if (mode_ == Mode::Off) return *this;
        cachedScalars_.emplace_back(name, v);
        return *this;
    }

    struct P4Snapshot {
        double pt{0}, eta{0}, phi{0}, m{0};
        double px{0}, py{0}, pz{0}, e{0};
    };

    inline MathGuard& cacheP4(const std::string& name, const TLorentzVector& p4) {
        if (mode_ == Mode::Off) return *this;

        P4Snapshot s;
        // Include both (pt,eta,phi,m) and (px,py,pz,E) because ROOT can produce NaN in Eta()
        s.pt  = p4.Pt();
        s.eta = p4.Eta();
        s.phi = p4.Phi();
        s.m   = p4.M();
        s.px  = p4.Px();
        s.py  = p4.Py();
        s.pz  = p4.Pz();
        s.e   = p4.E();

        cachedP4s_.emplace_back(name, s);
        return *this;
    }

    // ---- core checks ----
    inline void requireFiniteScalar(double x, const std::string& name) const {
        if (mode_ == Mode::Off) return;
        if (likely_(std::isfinite(x))) return;

        // slow path only
        std::ostringstream oss;
        oss << "non-finite scalar: " << name << "=" << x;
        handleFailure_(oss.str());
    }

    inline void requireFiniteP4(const TLorentzVector& p4, const std::string& name) const {
        if (mode_ == Mode::Off) return;

        // Guard both kinematics and components.
        const double pt  = p4.Pt();
        const double eta = p4.Eta();
        const double phi = p4.Phi();
        const double px  = p4.Px();
        const double py  = p4.Py();
        const double pz  = p4.Pz();
        const double e   = p4.E();

        if (likely_(std::isfinite(pt)  && std::isfinite(eta) && std::isfinite(phi) &&
                    std::isfinite(px)  && std::isfinite(py)  && std::isfinite(pz)  && std::isfinite(e)))
        {
            return;
        }

        std::ostringstream oss;
        oss << "non-finite 4-vector: " << name
            << " (pt=" << pt << ", eta=" << eta << ", phi=" << phi
            << ", px=" << px << ", py=" << py << ", pz=" << pz << ", E=" << e << ")";
        handleFailure_(oss.str());
    }

    inline void requireDenNotSmall(double den, const std::string& denName, double eps = 1e-12) const {
        requireFiniteScalar(den, denName);
        if (mode_ == Mode::Off) return;
        if (std::fabs(den) >= eps) return;

        std::ostringstream oss;
        oss << "unsafe denominator: |" << denName << "| < " << eps << " (=" << den << ")";
        handleFailure_(oss.str());
    }

    inline void requireAbsNotSmall(double x, const std::string& name, double eps = 1e-12) const {
        requireFiniteScalar(x, name);
        if (mode_ == Mode::Off) return;
        if (std::fabs(x) >= eps) return;

        std::ostringstream oss;
        oss << "unsafe value: |" << name << "| < " << eps << " (=" << x << ")";
        handleFailure_(oss.str());
    }

    inline void requireUnitAxisOK(const TLorentzVector& axis, const std::string& name, double eps = 1e-12) const {
        if (mode_ == Mode::Off) return;

        const double ax = axis.Px();
        const double ay = axis.Py();
        const double az = axis.Pz();

        if (!std::isfinite(ax) || !std::isfinite(ay) || !std::isfinite(az)) {
            handleFailure_(name + " axis has non-finite components");
        }
        if (axis.Vect().Mag2() < eps) {
            std::ostringstream oss;
            oss << name << " axis is degenerate (|v|^2 < " << eps << ")";
            handleFailure_(oss.str());
        }
    }

    inline double safeOnePlusOverOneMinus(double a, const std::string& aName, double eps = 1e-12) const {
        requireFiniteScalar(a, aName);
        if (mode_ == Mode::Off) return (1.0 + a) / (1.0 - a);

        const double den = 1.0 - a;
        if (std::fabs(den) < eps) {
            std::ostringstream oss;
            oss << "unsafe ratio (1+" << aName << ")/(1-" << aName << "): denominator too small. "
                << aName << "=" << a;
            handleFailure_(oss.str());
        }

        const double out = (1.0 + a) / den;
        requireFiniteScalar(out, "ratio(1+" + aName + ")/(1-" + aName + ")");
        return out;
    }

    template <typename T>
    inline void requireFiniteResult(T x, const std::string& name) const {
        requireFiniteScalar(static_cast<double>(x), name);
    }

    [[noreturn]] inline void fail(const std::string& what) const {
        handleFailure_(what);
        throw std::runtime_error("MathGuard::fail internal error"); // unreachable
    }

private:
    static inline bool likely_(bool x)   { return __builtin_expect(!!x, 1); }
    static inline bool unlikely_(bool x) { return __builtin_expect(!!x, 0); }

    inline std::string header_() const {
        std::ostringstream oss;
        oss << className_ << "::" << where_;
        return oss.str();
    }

    inline std::string context_() const {
        std::ostringstream oss;

        if (!cachedScalars_.empty() || !cachedP4s_.empty()) {
            oss << "\n  cached context:";
        }
        for (const auto& kv : cachedScalars_) {
            oss << "\n    " << kv.first << " = " << kv.second;
        }
        for (const auto& kv : cachedP4s_) {
            const auto& n = kv.first;
            const auto& s = kv.second;
            oss << "\n    " << n << " = "
                << "(pt=" << s.pt << ", eta=" << s.eta << ", phi=" << s.phi << ", m=" << s.m
                << "; px=" << s.px << ", py=" << s.py << ", pz=" << s.pz << ", E=" << s.e << ")";
        }
        return oss.str();
    }

    inline void handleFailure_(const std::string& what) const {
        if (mode_ == Mode::Off) return;

        std::ostringstream msg;
        msg << header_() << " - " << what;
        msg << context_();

        if (mode_ == Mode::Warn) {
            std::cerr << msg.str() << std::endl;
            return;
        }
        throw std::runtime_error(msg.str());
    }

    std::string className_;
    std::string where_;
    bool isDebug_{false}; // kept for future expansion
    Mode mode_{Mode::Throw};

    std::vector<std::pair<std::string, double>> cachedScalars_;
    std::vector<std::pair<std::string, P4Snapshot>> cachedP4s_;
};

