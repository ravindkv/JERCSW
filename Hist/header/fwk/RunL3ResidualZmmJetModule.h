#pragma once

#include <chrono>
#include <memory>
#include <string>
#include <vector>

#include <TLorentzVector.h>

#include "GlobalFlag.h"
#include "HistAlpha.h"
#include "HistCutflow.h"
#include "HistFlavor.h"
#include "HistJecUncBand.h"
#include "HistL3Residual.h"
#include "HistL3ResidualInput.hpp"
#include "HistObjGenReco.h"
#include "HistObjP4.h"
#include "HistPfComp.h"
#include "HistTime.h"
#include "MathL3Residual.h"
#include "fwk/IModule.h"

class HemVeto;
class JecUncBand;
class PickGenJet;
class PickZmmJet;
class TDirectory;
class VarBin;

namespace fwk {
class PickEventModule;
class ScaleJetModule;
class ScaleMetModule;
class ScaleMuonModule;
}

namespace fwk {

class RunL3ResidualZmmJetModule final : public IModule {
public:
    explicit RunL3ResidualZmmJetModule(const GlobalFlag& gf);
    ~RunL3ResidualZmmJetModule() override;

    std::string name() const override { return "RunL3ResidualZmmJetModule"; }

    void beginJob(Context& ctx) override;
    bool analyze(Context& ctx, Event& ev) override;
    void endJob(Context& ctx) override;

private:
    void loadConfig(const std::string& filename);

    const GlobalFlag& globalFlags_;

    std::vector<std::string> cutflows_;
    std::vector<int> minTagPts_;
    double maxDeltaPhiTagProbe_ = 0.0;
    double maxAlpha_ = 0.0;
    double minPtJet2InAlpha_ = 0.0;
    std::vector<double> alphaCuts_;
    double minResp_ = 0.0;
    double maxResp_ = 0.0;

    TDirectory* origDir_ = nullptr;

    std::unique_ptr<HistCutflow> h1EventInCutflow_;
    std::unique_ptr<VarBin> varBin_;
    std::unique_ptr<HistAlpha> histAlpha_;
    std::unique_ptr<HistObjP4> histObjP4Lep1_;
    std::unique_ptr<HistObjP4> histObjP4Lep2_;
    std::unique_ptr<HistObjP4> histObjP4Probe_;
    std::unique_ptr<HistObjP4> histObjP4Tag_;
    std::unique_ptr<HistObjGenReco> histObjGenRecoTag_;
    std::unique_ptr<HistObjGenReco> histObjGenRecoProbe_;
    std::unique_ptr<HistPfComp> histPfCompProbeInProbe_;
    std::unique_ptr<HistFlavor> histFlavorProbe_;
    std::unique_ptr<HistJecUncBand> histJecUncBand_;
    std::unique_ptr<HistL3Residual> histL3Residual_;
    std::unique_ptr<HistTime> histTime_;

    std::shared_ptr<PickZmmJet> pickZmmJet_;
    std::unique_ptr<PickEventModule> pickEventModule_;
    std::unique_ptr<ScaleMuonModule> scaleMuonModule_;
    std::unique_ptr<ScaleJetModule> scaleJetModule_;
    std::shared_ptr<PickGenJet> pickGenJet_;
    std::unique_ptr<ScaleMetModule> scaleMetModule_;
    std::shared_ptr<JecUncBand> jecUncBand_;
    std::shared_ptr<MathL3Residual> mathL3Residual_;
    std::shared_ptr<HemVeto> hemVeto_;

    double totalTime_ = 0.0;
    std::chrono::time_point<std::chrono::high_resolution_clock> startClock_;
    long long everyN_ = 1;
};

} // namespace fwk
