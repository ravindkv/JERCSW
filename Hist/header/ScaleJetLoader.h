#pragma once

#include <string>
#include "GlobalFlag.h"
#include "correction.h"

/**
 * @brief Load JERC config and hold all Correction::Ref objects.
 *
 * This class owns the correction references and exposes them
 * to the function class.
 */
class ScaleJetLoader {
public:
    explicit ScaleJetLoader(const GlobalFlag& globalFlags);

    // Access to flags
    const GlobalFlag& globalFlags() const { return globalFlags_; }
    const GlobalFlag::JetAlgo jetAlgo() const { return jetAlgo_; }
    bool isDebug() const { return isDebug_; }
    bool isData() const { return isData_; }
    bool isMC() const { return isMC_; }
    bool isBookKeep() const { return isBookKeep_; }
    bool applyJer() const { return applyJer_; }
    const std::string& jercJsonPath() const { return jercJsonPath_; }

    // Access to correction refs
    const correction::Correction::Ref& jetL1FastJetRef() const { return loadedJetL1FastJetRef_; }
    const correction::Correction::Ref& jetL2RelativeRef() const { return loadedJetL2RelativeRef_; }
    const correction::Correction::Ref& jetL2ResidualRef() const { return loadedJetL2ResidualRef_; }
    const correction::Correction::Ref& jetL2L3ResidualRef() const { return loadedJetL2L3ResidualRef_; }
    const correction::Correction::Ref& jerResoRef() const { return loadedJerResoRef_; }
    const correction::Correction::Ref& jerSfRef() const { return loadedJerSfRef_; }

    const correction::Correction::Ref& jerSmearRef() const { return jerSmearRef_; }

private:
    const GlobalFlag& globalFlags_;
    const GlobalFlag::Year year_;
    const GlobalFlag::Era era_;
    const GlobalFlag::JetAlgo jetAlgo_;
    const GlobalFlag::Channel channel_;
    const GlobalFlag::JecApplicationLevel level_;
    const bool isDebug_;
    const bool isData_;
    const bool isMC_;
    const bool isBookKeep_;
    bool applyJer_;
    correction::Correction::Ref jerSmearRef_;

    std::string jercJsonPath_;

    // names
    std::string jetL1FastJetName_;
    std::string jetL2RelativeName_;
    std::string jetL3AbsoluteName_;
    std::string jetL2ResidualName_;
    std::string jetL2L3ResidualName_;
    std::string JerResoName_;
    std::string JerSfName_;

    // refs
    correction::Correction::Ref loadedJetL1FastJetRef_;
    correction::Correction::Ref loadedJetL2RelativeRef_;
    correction::Correction::Ref loadedJetL3AbsoluteRef_;
    correction::Correction::Ref loadedJetL2ResidualRef_;
    correction::Correction::Ref loadedJetL2L3ResidualRef_;
    correction::Correction::Ref loadedJerResoRef_;
    correction::Correction::Ref loadedJerSfRef_;

    void loadConfig(const std::string& filename);
    void loadRefs();

    void loadJetL1FastJetRef();
    void loadJetL2RelativeRef();
    void loadJetL2ResidualRef();
    void loadJetL2L3ResidualRef();
    void loadJerResoRef();
    void loadJerSfRef();
};

