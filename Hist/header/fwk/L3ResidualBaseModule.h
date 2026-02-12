#pragma once

#include <chrono>
#include <memory>
#include <string>
#include <vector>

#include <TLorentzVector.h>

#include "GlobalFlag.h"
#include "HistAlpha.h"
#include "HistCutflow.h"
#include "HistL3Residual.h"
#include "HistL3ResidualInput.hpp"
#include "fwk/IModule.h"

class HemVeto;
class JecUncBand;
class TDirectory;
class VarBin;

namespace fwk {
class PickEventModule;
class ScaleJetModule;
class ScaleMetModule;
class ScaleMuonModule;

struct L3ResidualObjects {
    TLorentzVector p4RawTag;
    TLorentzVector p4Tag;
    TLorentzVector p4Probe;
    TLorentzVector p4Jet2;
    TLorentzVector p4Jetn;

    int iProbe = -1;
    int iJet2 = -1;
};

class L3ResidualBaseModule : public IModule {
public:
    explicit L3ResidualBaseModule(const GlobalFlag& gf);
    ~L3ResidualBaseModule() override;

    void beginJob(Context& ctx) override;
    bool analyze(Context& ctx, Event& ev) override;
    void endJob(Context& ctx) override;

protected:
    virtual std::string configPath() const = 0;
    virtual std::string moduleName() const = 0;

    virtual bool pickObjects(Context& ctx,
                             L3ResidualObjects& objects,
                             double& weight) = 0;

    virtual HistL3ResidualInput computeResponse(const L3ResidualObjects& objects,
                                                const TLorentzVector& p4CorrMet) = 0;

    virtual void fillChannelSpecificHistos(Context& ctx,
                                           const L3ResidualObjects& objects,
                                           const HistL3ResidualInput& input,
                                           double alpha,
                                           double weight) = 0;

    virtual void loadChannelConfig(const std::string& filename) = 0;
    virtual void bookChannelHistograms(TDirectory* origDir) = 0;
    virtual void applyChannelWeights(Context& ctx,
                                     const L3ResidualObjects& objects,
                                     double& weight) = 0;

    const GlobalFlag& globalFlags_;

    std::unique_ptr<PickEventModule> pickEventModule_;
    std::unique_ptr<ScaleMuonModule> scaleMuonModule_;
    std::unique_ptr<ScaleJetModule> scaleJetModule_;
    std::unique_ptr<ScaleMetModule> scaleMetModule_;

    std::shared_ptr<HemVeto> hemVeto_;
    std::shared_ptr<JecUncBand> jecUncBand_;

    std::vector<std::string> cutflows_;
    double maxDeltaPhiTagProbe_ = 0.0;
    double maxAlpha_ = 0.0;
    double minPtJet2InAlpha_ = 0.0;
    std::vector<double> alphaCuts_;
    double minResp_ = 0.0;
    double maxResp_ = 0.0;

    TDirectory* origDir_ = nullptr;

    std::unique_ptr<HistCutflow> hCutflow_;
    std::unique_ptr<VarBin> varBin_;
    std::unique_ptr<HistAlpha> histAlpha_;
    std::unique_ptr<HistL3Residual> histL3Residual_;

private:
    double totalTime_ = 0.0;
    std::chrono::time_point<std::chrono::high_resolution_clock> startClock_;
    long long everyN_ = 1;
};

} // namespace fwk
