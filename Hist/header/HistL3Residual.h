#pragma once

#include <string>

// forward declarations of ROOT and user types
class TH1D;
class TH2D;
class TProfile;
class TProfile2D;
class TDirectory;
class VarBin;

#include "HistL3ResidualInput.hpp"

class HistL3Residual {
public:
    // Constructor
    HistL3Residual(TDirectory* origDir, const std::string& directoryName, const VarBin& varBin);

    // Destructor
    ~HistL3Residual();

    [[nodiscard]] bool isInitialized() const noexcept { return initialized_; }

    void fillHistos(const HistL3ResidualInput& inputs);

    // TH1 
    TH1D*      h1EventInTagPt          = nullptr;
    TH1D*      h1EventInProbePt        = nullptr;
    TH1D*      h1EventInMetPt          = nullptr;
    TH1D*      h1EventInOtherPt        = nullptr;
    TH1D*      h1EventInUnclusteredPt  = nullptr;

    TProfile*      p1TagPtInTagPt          = nullptr;
    TProfile*      p1ProbePtInTagPt        = nullptr;
    TProfile*      p1MetPtInTagPt          = nullptr;
    TProfile*      p1OtherPtInTagPt        = nullptr;
    TProfile*      p1UnclusteredPtInTagPt  = nullptr;

    TH1D*      h1EventInRespDb         = nullptr;
    TH1D*      h1EventInRespMpf         = nullptr;
    TH1D*      h1EventInRespMpf1         = nullptr;
    TH1D*      h1EventInRespMpfn         = nullptr;
    TH1D*      h1EventInRespMpfu         = nullptr;
    TH1D*      h1EventInRespMpfnu         = nullptr;

    TProfile*  p1RespDbInProbeEta   = nullptr;
    TProfile*  p1RespDbInTagPt   = nullptr;
    TProfile*  p1RespMpfInTagPt   = nullptr;
    TProfile*  p1RespMpf1InTagPt   = nullptr;
    TProfile*  p1RespMpfnInTagPt   = nullptr;
    TProfile*  p1RespMpfuInTagPt   = nullptr;
    TProfile*  p1RespMpfnuInTagPt   = nullptr;

    TProfile2D*  p2RelDbRespInTagPtProbeEta   = nullptr;
    TProfile2D*  p2RespMpfInInTagPtProbeEta   = nullptr;
    TProfile2D*  p2RespMpf1IInTagPtProbeEta   = nullptr;
    TProfile2D*  p2RespMpfnInTagPtProbeEta   = nullptr;
    TProfile2D*  p2RespMpfuInTagPtProbeEta   = nullptr;
    TProfile2D*  p2RespMpfnuInTagPtProbeEta   = nullptr;

private:
    void InitializeHistograms(TDirectory* origDir, const std::string& directoryName, const VarBin& varBin);

    void requireInitialized_() const;

    bool initialized_ = false;
};

