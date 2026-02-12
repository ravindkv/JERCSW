#include "ScaleJetFunction.h"

#include <iostream>
#include <cmath>
#include <algorithm>
#include <cstdint>

#include "HelperDelta.hpp"
#include "ScaleFunctionGuard.hpp"

ScaleJetFunction::ScaleJetFunction(const GlobalFlag& globalFlags)
    : globalFlags_(globalFlags),
      loader_(globalFlags),
      jetAlgo_(globalFlags.getJetAlgo()),
      isDebug_(globalFlags.isDebug())
{}

// L1 Offset
double ScaleJetFunction::getL1FastJetCorrection(double jetArea, double jetEta,
                                               double jetPt, double rho) const {
    // NOTE: no skimT here; keep event context empty (0s)
    ScaleFunctionGuard guard(globalFlags_, "ScaleJetFunction::getL1FastJetCorrection");

    guard.checkFinite("jetArea", jetArea);
    guard.checkFinite("jetEta",  jetEta);
    guard.checkFinite("jetPt",   jetPt);
    guard.checkFinite("rho",     rho);

    // Basic sanity for jet-like inputs
    guard.checkPtEta(jetPt, jetEta, "jet");

    if (jetArea < 0.0) guard.warn("jetArea is negative");
    if (rho < 0.0)     guard.warn("rho is negative");

    double corr = 1.0;
    try {
        corr = loader_.jetL1FastJetRef()->evaluate({jetArea, jetEta, jetPt, rho});

        guard.checkFinite("corrL1FastJet", corr);
        guard.checkSf("corrL1FastJet", corr, 0.0, 10.0);

        if (isDebug_) {
            std::cout << " pt= " << jetPt
                      << ", eta= " << jetEta
                      << ", area = " << jetArea
                      << ", rho = " << rho
                      << "\n corrL1FastJet = " << corr << '\n';
        }
    } catch (const std::exception& e) {
        std::cerr << "\nEXCEPTION: in getL1FastJetCorrection(): " << e.what() << '\n';
        throw;
    }
    return corr;
}

// L2Relative
double ScaleJetFunction::getL2RelativeCorrection(double jetEta, double jetPt) const {
    ScaleFunctionGuard guard(globalFlags_, "ScaleJetFunction::getL2RelativeCorrection");

    guard.checkFinite("jetEta", jetEta);
    guard.checkFinite("jetPt",  jetPt);
    guard.checkPtEta(jetPt, jetEta, "jet");

    double corr = 1.0;
    try {
        corr = loader_.jetL2RelativeRef()->evaluate({jetEta, jetPt});

        guard.checkFinite("corrL2Relative", corr);
        guard.checkSf("corrL2Relative", corr, 0.0, 10.0);

        if (isDebug_) {
            std::cout << " pt= " << jetPt
                      << ", eta= " << jetEta
                      << "\n corrL2Relative = " << corr << '\n';
        }
    } catch (const std::exception& e) {
        std::cerr << "\nEXCEPTION: in getL2RelativeCorrection(): " << e.what() << '\n';
        throw;
    }
    return corr;
}

// L2Residual
double ScaleJetFunction::getL2ResidualCorrection(double jetEta, double jetPt) const {
    ScaleFunctionGuard guard(globalFlags_, "ScaleJetFunction::getL2ResidualCorrection");

    guard.checkFinite("jetEta", jetEta);
    guard.checkFinite("jetPt",  jetPt);
    guard.checkPtEta(jetPt, jetEta, "jet");

    double corr = 1.0;
    try {
        corr = loader_.jetL2ResidualRef()->evaluate({jetEta, jetPt});

        guard.checkFinite("corrL2Residual", corr);
        guard.checkSf("corrL2Residual", corr, 0.0, 10.0);

        if (isDebug_) {
            std::cout << " pt= " << jetPt
                      << ", eta= " << jetEta
                      << "\n corrL2Residual = " << corr << '\n';
        }
    } catch (const std::exception& e) {
        std::cerr << "\nEXCEPTION: in getL2ResidualCorrection(): " << e.what() << '\n';
        throw;
    }
    return corr;
}

// L2L3Residual
double ScaleJetFunction::getL2L3ResidualCorrection(double jetEta, double jetPt) const {
    ScaleFunctionGuard guard(globalFlags_, "ScaleJetFunction::getL2L3ResidualCorrection");

    guard.checkFinite("jetEta", jetEta);
    guard.checkFinite("jetPt",  jetPt);
    guard.checkPtEta(jetPt, jetEta, "jet");

    double corr = 1.0;
    try {
        corr = loader_.jetL2L3ResidualRef()->evaluate({jetEta, jetPt});

        guard.checkFinite("corrL2L3Residual", corr);
        guard.checkSf("corrL2L3Residual", corr, 0.0, 10.0);

        if (isDebug_) {
            std::cout << " pt= " << jetPt
                      << ", eta= " << jetEta
                      << "\n corrL2L3Residual = " << corr << '\n';
        }
    } catch (const std::exception& e) {
        std::cerr << "\nEXCEPTION: in getL2L3ResidualCorrection(): " << e.what() << '\n';
        throw;
    }
    return corr;
}

// JerReso
double ScaleJetFunction::getJerResolution(const SkimTree& skimT, int index) const {
    ScaleFunctionGuard guard(globalFlags_, "ScaleJetFunction::getJerResolution",
                             skimT.run, skimT.luminosityBlock, skimT.event);

    guard.checkIndex("Jet", index);

    const double eta = skimT.Jet_eta[index];
    const double pt  = skimT.Jet_pt[index];
    const double rho = skimT.Rho;

    guard.checkPtEta(pt, eta, "jet");
    guard.checkFinite("rho", rho);
    if (rho < 0.0) guard.warn("rho is negative");

    double JerReso = 1.0;
    try {
        JerReso = loader_.jerResoRef()->evaluate({eta, pt, rho});

        guard.checkFinite("JerReso", JerReso);
        // Resolution should be >= 0; allow wide upper range
        guard.checkSf("JerReso", JerReso, 0.0, 5.0);

        if (isDebug_) {
            std::cout << " pt= " << pt
                      << ", eta= " << eta
                      << ", rho = " << rho
                      << ", JerReso = " << JerReso << '\n';
        }
    } catch (const std::exception& e) {
        std::cout << "\nEXCEPTION: in getJerResolution(): " << e.what() << '\n';
        throw;
    }
    return JerReso;
}

// JerSf
double ScaleJetFunction::getJerScaleFactor(const SkimTree& skimT, int index,
                                          const std::string& syst) const {
    ScaleFunctionGuard guard(globalFlags_, "ScaleJetFunction::getJerScaleFactor",
                             skimT.run, skimT.luminosityBlock, skimT.event);

    guard.checkIndex("Jet", index);

    const double eta = skimT.Jet_eta[index];
    const double pt  = skimT.Jet_pt[index];
    const double rho = skimT.Rho;

    guard.checkPtEta(pt, eta, "jet");
    guard.checkFinite("rho", rho);

    double JerSf = 1.0;
    using Var = correction::Variable::Type;
    std::vector<Var> vals;

    // Keep your current branching as-is
    /*
    if (jetAlgo_ == GlobalFlag::JetAlgo::AK4Chs) {
        vals.emplace_back(static_cast<double>(eta)); // JetEta
        vals.emplace_back(static_cast<double>(pt));  // JetPt
        vals.emplace_back(static_cast<double>(rho)); // Rho
    } else {
        vals.emplace_back(static_cast<double>(eta));       // JetEta
        vals.emplace_back(static_cast<std::string>(syst)); // syst
    }
    *///FIXME
        vals.emplace_back(static_cast<double>(eta));       // JetEta
        vals.emplace_back(static_cast<std::string>(syst)); // syst

    try {
        // Summer20UL style:
        JerSf = loader_.jerSfRef()->evaluate(vals);

        guard.checkFinite("JerSf", JerSf);
        guard.checkSf("JerSf", JerSf, 0.0, 5.0);

        if (isDebug_) {
            std::cout << "eta= " << eta
                      << ", syst  = " << syst
                      << "\n JerSf = " << JerSf << '\n';
        }
    } catch (const std::exception& e) {
        std::cout << "\nEXCEPTION: in getJerScaleFactor(): " << e.what() << '\n';
        throw;
    }
    return JerSf;
}

double ScaleJetFunction::getJerCorrection(const SkimTree& skimT,
                                         int index,
                                         const std::string& syst,
                                         double pt_corr) const {
    ScaleFunctionGuard guard(globalFlags_, "ScaleJetFunction::getJerCorrection",
                             skimT.run, skimT.luminosityBlock, skimT.event);

    guard.checkIndex("Jet", index);
    guard.checkFinite("pt_corr", pt_corr);

    // 1) Get resolution and scale factor like before
    const double resoJer = getJerResolution(skimT, index);
    const double sfJer   = getJerScaleFactor(skimT, index, syst);

    guard.checkFinite("resoJer", resoJer);
    guard.checkSf("resoJer", resoJer, 0.0, 5.0);
    guard.checkFinite("sfJer", sfJer);
    guard.checkSf("sfJer", sfJer, 0.0, 5.0);

    const double jetEta = skimT.Jet_eta[index];
    const double jetPt  = pt_corr;
    const double jetPhi = skimT.Jet_phi[index];
    const double rho    = skimT.Rho;

    guard.checkPtEtaPhi(jetPt, jetEta, jetPhi, "jet");
    guard.checkFinite("rho", rho);

    // 2) Gen matching logic
    const int genIdx = skimT.Jet_genJetIdx[index];

    bool   matched       = false;
    double genPtForSmear = -1.0;

    if (genIdx > -1 && genIdx < static_cast<int>(skimT.nGenJet)) {
        const double genEta = skimT.GenJet_eta[genIdx];
        const double genPhi = skimT.GenJet_phi[genIdx];
        const double genPt  = skimT.GenJet_pt[genIdx];

        guard.checkFinite("genEta", genEta);
        guard.checkFinite("genPhi", genPhi);
        guard.checkFinite("genPt",  genPt);

        const double dR  = HelperDelta::DELTAR(jetPhi, genPhi, jetEta, genEta);
        const double dPt = std::abs(jetPt - genPt);

        guard.checkFinite("dR", dR);
        guard.checkFinite("dPt", dPt);

        constexpr double maxDR = 0.2;
        if (dR < maxDR && dPt < 3.0 * resoJer * jetPt) {
            matched       = true;
            genPtForSmear = genPt;
        }

        if (isDebug_) {
            std::cout << "JER match: hasGen=1, dR=" << dR
                      << ", dPt=" << dPt
                      << ", matched=" << (matched ? 1 : 0) << '\n';
        }
    } else {
        if (isDebug_) {
            std::cout << "JER match: hasGen=0\n";
        }
    }

    guard.checkFinite("genPtForSmear", genPtForSmear);
    // If matched, genPtForSmear should be >0; if unmatched, it's -1.0 (by design)
    if (matched && genPtForSmear <= 0.0) {
        guard.warn("matched=true but genPtForSmear <= 0 (unexpected)");
    }

    // 3) Build inputs for JERSmear (hashPRNG lives in JSON)
    using Var = correction::Variable::Type;

    std::vector<Var> vals;
    vals.reserve(7);

    // Order MUST match JSON:
    // JetPt, JetEta, GenPt, Rho, EventID, JER, JERSF
    vals.emplace_back(static_cast<double>(jetPt));         // JetPt
    vals.emplace_back(static_cast<double>(jetEta));        // JetEta
    vals.emplace_back(static_cast<double>(genPtForSmear)); // GenPt (or -1)
    vals.emplace_back(static_cast<double>(rho));           // Rho

    // EventID: deterministic entropy source from (run, lumi, event)
    const std::int64_t rawEvent = static_cast<std::int64_t>(skimT.event);
    const std::int64_t rawRun   = static_cast<std::int64_t>(skimT.run);
    const std::int64_t rawLumi  = static_cast<std::int64_t>(skimT.luminosityBlock);

    std::uint64_t h = static_cast<std::uint64_t>(rawEvent);
    h ^= static_cast<std::uint64_t>(rawRun)  + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);
    h ^= static_cast<std::uint64_t>(rawLumi) + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);
    const int eventID = static_cast<int>(h & 0x7fffffff); // positive 32-bit

    guard.checkFinite("eventID", static_cast<double>(eventID));
    if (eventID <= 0) {
        guard.warn("eventID is <= 0 (unexpected for hash&mask)");
    }

    vals.emplace_back(eventID);                            // EventID
    vals.emplace_back(static_cast<double>(resoJer));       // JER
    vals.emplace_back(static_cast<double>(sfJer));         // JERSF

    // 4) Evaluate the correction
    double smear = 1.0;
    try {
        smear = loader_.jerSmearRef()->evaluate(vals);
    } catch (const std::exception& e) {
        std::cerr << "\nEXCEPTION: in getJerCorrection()/JERSmear: " << e.what()
                  << "\nFalling back to corr = 1.0\n";
        smear = 1.0;
    }

    const double corrJer =
        (std::isfinite(smear) && smear > 0.0) ? smear : 1.0;

    // Guard checks on smear/correction
    guard.checkFinite("smear", smear);
    guard.checkFinite("corrJer", corrJer);
    guard.checkSf("corrJer", corrJer, 0.0, 10.0);

    if (isDebug_) {
        const double genPt = (genIdx > -1 && genIdx < skimT.nGenJet)
                                 ? skimT.GenJet_pt[genIdx]
                                 : -1.;
        std::cout << "input JES-corrected pt = " << jetPt
                  << '\n'
                  << ", gen  pt = " << genPt
                  << ", Resolution = " << resoJer
                  << ", sfJer = " << sfJer
                  << ", corrJer = " << corrJer
                  << ", matched = " << (matched ? 1 : 0)
                  << ", eventID = " << eventID
                  << '\n'
                  << ", JER-corrected pT would be = " << jetPt*corrJer
                  << '\n';
    }

    return corrJer;
}

