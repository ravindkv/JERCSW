#pragma once

#include <memory>
#include <string>
#include <vector>

#include "HistFlavor.h"
#include "HistJecUncBand.h"
#include "HistObjGenReco.h"
#include "HistObjP4.h"
#include "HistPfComp.h"
#include "HistTime.h"
#include "MathL3Residual.h"
#include "fwk/L3ResidualBaseModule.h"

class PickGenJet;
class PickZmmJet;

namespace fwk {

class RunL3ResidualZmmJetModule final : public L3ResidualBaseModule {
public:
    explicit RunL3ResidualZmmJetModule(const GlobalFlag& gf);
    ~RunL3ResidualZmmJetModule() override;

    std::string name() const override { return "RunL3ResidualZmmJetModule"; }

protected:
    std::string configPath() const override { return "config/RunL3ResidualZmmJet.json"; }
    std::string moduleName() const override { return "RunL3ResidualZmmJetModule"; }

    bool pickObjects(Context& ctx,
                     L3ResidualObjects& objects,
                     double& weight) override;

    HistL3ResidualInput computeResponse(const L3ResidualObjects& objects,
                                        const TLorentzVector& p4CorrMet) override;

    void fillChannelSpecificHistos(Context& ctx,
                                   const L3ResidualObjects& objects,
                                   const HistL3ResidualInput& input,
                                   double alpha,
                                   double weight) override;

    void loadChannelConfig(const std::string& filename) override;
    void bookChannelHistograms(TDirectory* origDir) override;
    void applyChannelWeights(Context& ctx,
                             const L3ResidualObjects& objects,
                             double& weight) override;

private:
    std::vector<int> minTagPts_;

    std::unique_ptr<HistObjP4> histObjP4Lep1_;
    std::unique_ptr<HistObjP4> histObjP4Lep2_;
    std::unique_ptr<HistObjP4> histObjP4Probe_;
    std::unique_ptr<HistObjP4> histObjP4Tag_;
    std::unique_ptr<HistObjGenReco> histObjGenRecoTag_;
    std::unique_ptr<HistObjGenReco> histObjGenRecoProbe_;
    std::unique_ptr<HistPfComp> histPfCompProbeInProbe_;
    std::unique_ptr<HistFlavor> histFlavorProbe_;
    std::unique_ptr<HistJecUncBand> histJecUncBand_;
    std::unique_ptr<HistTime> histTime_;

    std::shared_ptr<PickZmmJet> pickZmmJet_;
    std::shared_ptr<PickGenJet> pickGenJet_;
    std::shared_ptr<MathL3Residual> mathL3Residual_;

    std::vector<int> pickedMuons_;
};

} // namespace fwk
