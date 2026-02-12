#pragma once

#include <string>

// forward declarations of ROOT and user types
class TH1D;
class TH2D;
class TProfile;
class TProfile2D;
class TDirectory;
class VarBin;

#include "HistL2ResidualInput.hpp"

class HistL2Residual {
public:
    // Constructor
    HistL2Residual(TDirectory* origDir, const std::string& directoryName, const VarBin& varBin);

    // Destructor
    ~HistL2Residual();

    [[nodiscard]] bool isInitialized() const noexcept { return initialized_; }

    void fillHistos(const HistL2ResidualInput& inputs);

    // TH1 
    TH1D*      h1EventInProbeEta        = nullptr;
    TH1D*      h1EventInTagEta          = nullptr;
    TH1D*      h1EventInAsymmetryA      = nullptr;
    TH1D*      h1EventInAsymmetryB      = nullptr;

    TH1D*      h1EventInAsymmetryAPlusOne = nullptr;
    TH1D*      h1EventInAsymmetryBPlusOne = nullptr;

    TH1D*      h1EventInRelDbResp         = nullptr;
    TH1D*      h1EventInBisectorMpfResp   = nullptr;
    TH1D*      h1EventInRelMpfResp        = nullptr;
    TH1D*      h1EventInRelMpfxResp        = nullptr;

    //Profiles
    TProfile*  p1ProbePtInProbeEta      = nullptr;
    TProfile*  p1TagPtInProbeEta        = nullptr;
    TProfile*  p1TagMassInProbeEta        = nullptr;
    TProfile*  p1AvgPtInProbeEta        = nullptr;
    TProfile*  p1AvgProjPtInProbeEta    = nullptr;
    TProfile*  p1MetPtInProbeEta        = nullptr;
    TProfile*  p1OtherPtInProbeEta      = nullptr;
    TProfile*  p1UnclusteredPtInProbeEta= nullptr;

    TProfile*  p1AsymmetryAPlusOneInProbeEta   = nullptr;
    TProfile*  p1AsymmetryBPlusOneInProbeEta   = nullptr;

    TProfile*  p1RelDbRespInProbeEta   = nullptr;
    TProfile*  p1BisectorMpfRespInProbeEta = nullptr;
    TProfile*  p1RelMpfRespInProbeEta   = nullptr;
    TProfile*  p1RelMpfxRespInProbeEta   = nullptr;

    TProfile2D*  p2RelDbRespInProbePtProbeEta   = nullptr;
    TProfile2D*  p2BisectorMpfRespInProbePtProbeEta = nullptr;
    TProfile2D*  p2RelMpfRespInProbePtProbeEta   = nullptr;
    TProfile2D*  p2RelMpfxRespInProbePtProbeEta   = nullptr;

private:
    void InitializeHistograms(TDirectory* origDir, const std::string& directoryName, const VarBin& varBin);

    void requireInitialized_() const;

    bool initialized_ = false;
};

