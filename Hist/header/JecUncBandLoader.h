#pragma once

#include <string>
#include "GlobalFlag.h"
#include "correction.h"

class JecUncBandLoader {
public:
    explicit JecUncBandLoader(const GlobalFlag& globalFlags);

    // Access to flags
    const GlobalFlag& globalFlags() const { return globalFlags_; }
    bool isDebug() const { return isDebug_; }
    bool isData() const { return isData_; }
    bool isMC() const { return isMC_; }

    const correction::Correction::Ref& getJesUncBandRef() const { return loadedJesUncBandRef_; }
    const correction::Correction::Ref& getJerSfUncBandRef() const { return loadedJerSfUncBandRef_; }
private:
    const GlobalFlag& globalFlags_;
    const GlobalFlag::Year year_;
    const GlobalFlag::Era era_;
    const GlobalFlag::Channel channel_;
    const bool isDebug_;
    const bool isData_;
    const bool isMC_;

    std::string jesUncBandJsonPath_;
    std::string jerSfUncBandJsonPath_;

    // names
    std::string jesUncBandName_;//JEC Total for Unc Band
    std::string jerSfUncBandName_;

    // refs
    correction::Correction::Ref loadedJesUncBandRef_;
    correction::Correction::Ref loadedJerSfUncBandRef_;

    void loadConfig(const std::string& filename);

    void loadJesUncBandRef();
    void loadJerSfUncBandRef();
};

