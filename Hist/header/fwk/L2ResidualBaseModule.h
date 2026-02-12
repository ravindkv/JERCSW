#pragma once

#include <memory>
#include <string>
#include <vector>

#include <TLorentzVector.h>

#include "GlobalFlag.h"
#include "HistCutflow.h"
#include "HistL2Residual.h"
#include "HistL2ResidualInput.hpp"
#include "fwk/IModule.h"

class HemVeto;
class TDirectory;
class VarBin;

namespace fwk {
class PickEventModule;
class ScaleJetModule;
class ScaleMetModule;
class ScaleMuonModule;

struct L2ResidualObjects {
    TLorentzVector p4RawTag;
    TLorentzVector p4Tag;
    TLorentzVector p4Probe;
    TLorentzVector p4Jet2;
    TLorentzVector p4Jetn;

    int iProbe = -1;
    int iJet2 = -1;
};

class L2ResidualBaseModule : public IModule {
public:
    explicit L2ResidualBaseModule(const GlobalFlag& gf);
    ~L2ResidualBaseModule() override;

    void beginJob(Context& ctx) override;
    bool analyze(Context& ctx, Event& ev) override;
    void endJob(Context& ctx) override;

protected:
    virtual std::string configPath() const = 0;
    virtual std::string moduleName() const = 0;

    virtual bool pickObjects(Context& ctx,
                             L2ResidualObjects& objects,
                             double& weight) = 0;

    virtual HistL2ResidualInput computeResponse(const L2ResidualObjects& objects,
                                                const TLorentzVector& p4CorrMet) = 0;

    virtual void fillChannelSpecificHistos(Context& ctx,
                                           const L2ResidualObjects& objects,
                                           const HistL2ResidualInput& input,
                                           double weight) = 0;

    virtual void loadChannelConfig(const std::string& filename) = 0;
    virtual void bookChannelHistograms(TDirectory* origDir) = 0;
    virtual void applyChannelWeights(Context& ctx,
                                     const L2ResidualObjects& objects,
                                     double& weight) = 0;

    const GlobalFlag& globalFlags_;

    std::unique_ptr<PickEventModule> pickEventModule_;
    std::unique_ptr<ScaleMuonModule> scaleMuonModule_;
    std::unique_ptr<ScaleJetModule> scaleJetModule_;
    std::unique_ptr<ScaleMetModule> scaleMetModule_;

    std::shared_ptr<HemVeto> hemVeto_;

    std::vector<std::string> cutflows_;
    double minResp_ = 0.0;
    double maxResp_ = 0.0;

    TDirectory* origDir_ = nullptr;

    std::unique_ptr<HistCutflow> hCutflow_;
    std::unique_ptr<VarBin> varBin_;
    std::unique_ptr<HistL2Residual> histL2Residual_;
};

} // namespace fwk
