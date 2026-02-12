
#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>   // for std::unique_ptr
#include "HistL3ResidualInput.hpp"

// forward declarations
class TH1D;
class TH2D;
class TProfile;
class TProfile2D;
class TDirectory;
class VarBin;

class HistAlpha {
public:
    HistAlpha(TDirectory* origDir, const std::string& directoryName, const VarBin& varBin, const std::vector<double>& alphaCuts);
    
    ~HistAlpha();
    
    void Fill(double eventAlpha, double rho, const HistL3ResidualInput& in);
    void Fill(double eventAlpha, double ptProbe, double rho, double dbResp, double mpfResp, double mpfnResp, double mpfuResp,double weight);

private:
    // Maps keyed by the α‐cut value.
    std::map<double, std::unique_ptr<TH1D>> h1EventInProbePtForAlpha_;

    std::unique_ptr<TProfile> p1DbRespInAlphaForProbePt175to230_;
    std::unique_ptr<TProfile> p1MpfRespInAlphaForProbePt175to230_;
    std::unique_ptr<TProfile> p1MpfnRespInAlphaForProbePt175to230_;
    std::unique_ptr<TProfile> p1MpfuRespInAlphaForProbePt175to230_;

    std::unique_ptr<TProfile> p1DbRespInMaxAlphaForProbePt175to230_;
    std::unique_ptr<TProfile> p1MpfRespInMaxAlphaForProbePt175to230_;
    std::unique_ptr<TProfile> p1MpfnRespInMaxAlphaForProbePt175to230_;
    std::unique_ptr<TProfile> p1MpfuRespInMaxAlphaForProbePt175to230_;

    std::map<double, std::unique_ptr<TProfile>> p1RhoInProbePtForAlpha_;
    std::map<double, std::unique_ptr<TProfile>> p1DpbRespInProbePtForAlpha_;
    std::map<double, std::unique_ptr<TProfile>> p1MpfRespInProbePtForAlpha_;
    std::map<double, std::unique_ptr<TProfile>> p1MpfnRespInProbePtForAlpha_;
    std::map<double, std::unique_ptr<TProfile>> p1MpfuRespInProbePtForAlpha_;

    // Histogram for the event α distribution.
    std::unique_ptr<TH1D> h1EventInAlpha_;
    std::unique_ptr<TH2D> h2EventInProbePtAlpha_;
    std::unique_ptr<TProfile2D> p2DbRespInProbePtAlpha_;
    std::unique_ptr<TProfile2D> p2MpfRespInProbePtAlpha_;
    std::unique_ptr<TProfile2D> p2MpfnRespInProbePtAlpha_;
    std::unique_ptr<TProfile2D> p2MpfuRespInProbePtAlpha_;

    // Binning configuration (assumed common for all histograms)
    std::vector<double> binsPt_;
    int nPt_;

    // The α‐cut values used for creating the histograms
    std::vector<double> alphaCuts_;

    void InitializeHistogramsForAlpha(double alphaCut, TDirectory* dir);
};
