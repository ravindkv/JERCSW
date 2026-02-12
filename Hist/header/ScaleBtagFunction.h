#pragma once

#include <string>
#include <vector>
#include <algorithm>  // std::clamp
#include <cmath>      // std::nextafter
#include <limits>

#include "GlobalFlag.h"
#include "SkimTree.h"
#include "ScaleBtagLoader.h"

// ROOT fwd decls
class TH2;
class TAxis;

class ScaleBtagFunction {
public:
    explicit ScaleBtagFunction(const GlobalFlag& globalFlags);

    double getEventWeight(const SkimTree& skimT,
                          const std::vector<int>& jetIdx,
                          const std::string& syst = "central",
                          double btagCut = -1.0) const;

    void setEffType(const std::string& effType) { loader_.setEffType(effType); }

    const std::string& effType() const { return loader_.effType(); }
    const std::string& algo()    const { return loader_.algo(); }
    const std::string& wp()      const { return loader_.wp(); }

private:
    // helpers
    static void   parseSyst(const std::string& syst, std::string& b_syst, std::string& l_syst);
    static double clampInsideAxis_(const TAxis* ax, double x);               // [xmin, nextafter(xmax,xmin)]
    static double getBinContent2D_(const TH2* h2, double x, double y);       // axis-aware clamp
    static double clampInside_(double v, double lo, double hi_inclusive);    // [lo, nextafter(hi,lo)]

    double getPerJetEff_(int flavor, double pt, double abseta) const;
    double getPerJetSF_(int flavor, double pt, double eta,
                        const std::string& b_syst, const std::string& l_syst) const;

private:
    ScaleBtagLoader loader_; // composition: Loader used ONLY here

    const GlobalFlag& globalFlags_;
    const bool isDebug_;
    const bool isData_;
    const bool isMC_;
};

