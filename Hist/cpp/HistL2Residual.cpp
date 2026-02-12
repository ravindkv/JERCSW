#include "HistL2Residual.h"
#include "HelperDir.hpp"
#include "VarBin.h"

#include <TDirectory.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TProfile.h>
#include <TProfile2D.h>

#include <cmath>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

// out-of-line destructor (empty)
HistL2Residual::~HistL2Residual() = default;

// Constructor
HistL2Residual::HistL2Residual(TDirectory* origDir,
                               const std::string& directoryName,
                               const VarBin& varBin)
{
    InitializeHistograms(origDir, directoryName, varBin);
    initialized_ = true;
}

void HistL2Residual::requireInitialized_() const
{
    if (!initialized_) {
        throw std::runtime_error("HistL2Residual::fillHistos called before histograms were initialized.");
    }
    // Also guard against partially-initialized state:
    if (!h1EventInTagEta || !h1EventInProbeEta ||
        !h1EventInAsymmetryA || !h1EventInAsymmetryB ||
        !h1EventInAsymmetryAPlusOne || !h1EventInAsymmetryBPlusOne ||
        !p1RelDbRespInProbeEta || !p1AsymmetryAPlusOneInProbeEta || 
        !p1AsymmetryBPlusOneInProbeEta ||
        !p1BisectorMpfRespInProbeEta || !p1RelMpfRespInProbeEta ||!p1RelMpfxRespInProbeEta ||
        !p2RelDbRespInProbePtProbeEta ||
        !p2BisectorMpfRespInProbePtProbeEta || !p2RelMpfRespInProbePtProbeEta|| !p2RelMpfxRespInProbePtProbeEta)

    {
        throw std::runtime_error("HistL2Residual: histogram/profile pointer is null (partial initialization).");
    }
}

// Method to initialize histograms
void HistL2Residual::InitializeHistograms(TDirectory* origDir,
                                         const std::string& directoryName,
                                         const VarBin& varBin)
{
    if (!origDir) {
        throw std::runtime_error("HistL2Residual::InitializeHistograms - origDir is null");
    }

    // Use the HelperDir method to get or create the directory
    const std::string dirName = directoryName + "/HistL2Residual";
    TDirectory* newDir = HelperDir::createTDirectory(origDir, dirName);
    if (!newDir) {
        throw std::runtime_error("HistL2Residual::InitializeHistograms - failed to create/find directory: " + dirName);
    }
    newDir->cd();

    std::vector<double> binsPt = varBin.getBinsPt();
    const int nPt = static_cast<int>(binsPt.size()) - 1;

    std::vector<double> binsEta = varBin.getBinsEta();
    const int nEta = static_cast<int>(binsEta.size()) - 1;

    // Initialize histograms and profiles
    h1EventInTagEta   = new TH1D("h1EventInTagEta",   "", nEta, binsEta.data());
    h1EventInProbeEta   = new TH1D("h1EventInProbeEta",   "", nEta, binsEta.data());
    // responses: choose a safe generic range (adjust later if you add VarBin ranges)
    constexpr int    nRespBins = 50;
    constexpr double respMin   = 0.0;
    constexpr double respMax   = 2.0;

    std::vector<double> rangeAsym = varBin.getRangeAsymmetry();
    const int nAsym = static_cast<int>(rangeAsym.at(0));
    const double minAsym = rangeAsym.at(1);
    const double maxAsym = rangeAsym.at(2);

    h1EventInAsymmetryA = new TH1D("h1EventInAsymmetryA", "", nAsym, minAsym, maxAsym);
    h1EventInAsymmetryB = new TH1D("h1EventInAsymmetryB", "", nAsym, minAsym, maxAsym);
    h1EventInAsymmetryAPlusOne = new TH1D("h1EventInAsymmetryAPlusOne", "", nAsym, 1.0 + minAsym, 1.0 + maxAsym);
    h1EventInAsymmetryBPlusOne = new TH1D("h1EventInAsymmetryBPlusOne", "", nAsym, 1.0 + minAsym, 1.0 + maxAsym);


    h1EventInRelDbResp        = new TH1D("h1EventInRelDbResp",        "", nRespBins, respMin, respMax);
    h1EventInBisectorMpfResp  = new TH1D("h1EventInBisectorMpfResp",  "", nRespBins, respMin, respMax);
    h1EventInRelMpfResp       = new TH1D("h1EventInRelMpfResp",       "", nRespBins, respMin, respMax);
    h1EventInRelMpfxResp       = new TH1D("h1EventInRelMpfxResp",       "", nRespBins, respMin, respMax);


    // (6) For weighted fills, ensure proper error storage
    h1EventInTagEta->Sumw2();
    h1EventInProbeEta->Sumw2();
    h1EventInAsymmetryA->Sumw2();
    h1EventInAsymmetryB->Sumw2();
    h1EventInAsymmetryAPlusOne->Sumw2();
    h1EventInAsymmetryBPlusOne->Sumw2();

    h1EventInRelDbResp->Sumw2();
    h1EventInBisectorMpfResp->Sumw2();
    h1EventInRelMpfResp->Sumw2();
    h1EventInRelMpfxResp->Sumw2();

    p1AsymmetryAPlusOneInProbeEta       = new TProfile("p1AsymmetryAPlusOneInProbeEta",   "", nEta, binsEta.data());
    p1AsymmetryBPlusOneInProbeEta       = new TProfile("p1AsymmetryBPlusOneInProbeEta",   "", nEta, binsEta.data());

    p1ProbePtInProbeEta       = new TProfile("p1ProbePtInProbeEta",   "", nEta, binsEta.data());
    p1TagPtInProbeEta         = new TProfile("p1TagPtInProbeEta",     "", nEta, binsEta.data());
    p1TagMassInProbeEta         = new TProfile("p1TagMassInProbeEta",     "", nEta, binsEta.data());
    p1AvgPtInProbeEta         = new TProfile("p1AvgPtInProbeEta",     "", nEta, binsEta.data());
    p1AvgProjPtInProbeEta     = new TProfile("p1AvgProjPtInProbeEta", "", nEta, binsEta.data());
    p1MetPtInProbeEta         = new TProfile("p1MetPtInProbeEta",     "", nEta, binsEta.data());
    p1OtherPtInProbeEta       = new TProfile("p1OtherPtInProbeEta",   "", nEta, binsEta.data());
    p1UnclusteredPtInProbeEta = new TProfile("p1UnclusteredPtInProbeEta", "", nEta, binsEta.data());

    p1RelDbRespInProbeEta       = new TProfile("p1RelDbRespInProbeEta",       "", nEta, binsEta.data());
    p1BisectorMpfRespInProbeEta  = new TProfile("p1BisectorMpfRespInProbeEta",  "", nEta, binsEta.data());
    p1RelMpfRespInProbeEta       = new TProfile("p1RelMpfRespInProbeEta",       "", nEta, binsEta.data());
    p1RelMpfxRespInProbeEta       = new TProfile("p1RelMpfxRespInProbeEta",       "", nEta, binsEta.data());

    p2RelDbRespInProbePtProbeEta       = new TProfile2D("p2RelDbRespInProbePtProbeEta",         "", nEta, binsEta.data(), nPt, binsPt.data());
    p2BisectorMpfRespInProbePtProbeEta  = new TProfile2D("p2BisectorMpfRespInProbePtProbeEta",  "", nEta, binsEta.data(), nPt, binsPt.data());
    p2RelMpfRespInProbePtProbeEta       = new TProfile2D("p2RelMpfRespInProbePtProbeEta",       "", nEta, binsEta.data(), nPt, binsPt.data());
    p2RelMpfxRespInProbePtProbeEta       = new TProfile2D("p2RelMpfxRespInProbePtProbeEta",       "", nEta, binsEta.data(), nPt, binsPt.data());
    // Return to original directory
    origDir->cd();
}

void HistL2Residual::fillHistos(const HistL2ResidualInput& inputs)
{
    requireInitialized_();

    // (3) quick skip
    const double weight = inputs.weight;
    if (weight == 0.0) return;

    auto finiteOrThrow = [&](double x, const char* name) {
        if (!std::isfinite(x)) {
            std::ostringstream oss;
            oss << "HistL2Residual::fillHistos - non-finite input: " << name
                << " = " << x
                << " (etaProbe=" << inputs.etaProbe
                << ", ptTag=" << inputs.ptTag
                << ", ptProbe=" << inputs.ptProbe
                << ", weight=" << inputs.weight << ")";
            throw std::runtime_error(oss.str());
        }
    };

    // Validate key x-axis coordinate and weight
    finiteOrThrow(inputs.etaProbe, "etaProbe");
    finiteOrThrow(weight, "weight");

    // Validate commonly filled quantities (y-values)
    finiteOrThrow(inputs.ptTag, "ptTag");
    finiteOrThrow(inputs.ptProbe, "ptProbe");
    finiteOrThrow(inputs.ptMet, "ptMet");
    finiteOrThrow(inputs.ptOther, "ptOther");
    finiteOrThrow(inputs.ptUnclustered, "ptUnclustered");

    finiteOrThrow(inputs.relDbResp, "relDbResp");
    finiteOrThrow(inputs.respMetOnBisector, "respMetOnBisector");
    finiteOrThrow(inputs.relMpfResp, "relMpfResp");
    finiteOrThrow(inputs.relMpfxResp, "relMpfxResp");

    // Fill
    const double etaProbe = inputs.etaProbe;
    const double ptProbe = inputs.ptProbe;

    h1EventInTagEta->Fill(inputs.etaTag, weight);
    h1EventInProbeEta->Fill(etaProbe, weight);
    h1EventInAsymmetryA->Fill(inputs.asymmA, weight);
    h1EventInAsymmetryB->Fill(inputs.asymmB, weight);
    h1EventInAsymmetryAPlusOne->Fill(1.0 + inputs.asymmA, weight);
    h1EventInAsymmetryBPlusOne->Fill(1.0 + inputs.asymmB, weight);

    h1EventInRelDbResp->Fill(inputs.relDbResp, weight);
    h1EventInBisectorMpfResp->Fill(inputs.respMetOnBisector, weight);
    h1EventInRelMpfResp->Fill(inputs.relMpfResp, weight);
    h1EventInRelMpfxResp->Fill(inputs.relMpfxResp, weight);

    p1AsymmetryAPlusOneInProbeEta->Fill(etaProbe, 1.0 + inputs.asymmA,  weight);
    p1AsymmetryBPlusOneInProbeEta->Fill(etaProbe, 1.0 + inputs.asymmB,  weight);

    p1ProbePtInProbeEta->Fill(etaProbe, inputs.ptProbe,  weight);
    p1TagPtInProbeEta->Fill(etaProbe,   inputs.ptTag,    weight);
    p1TagMassInProbeEta->Fill(etaProbe,   inputs.massTag,    weight);
    p1AvgPtInProbeEta->Fill(etaProbe,   inputs.ptAverage,weight);
    p1AvgProjPtInProbeEta->Fill(etaProbe, inputs.ptAvgProj, weight);
    p1MetPtInProbeEta->Fill(etaProbe,   inputs.ptMet,    weight);
    p1OtherPtInProbeEta->Fill(etaProbe, inputs.ptOther,  weight);
    p1UnclusteredPtInProbeEta->Fill(etaProbe, inputs.ptUnclustered, weight);

    p1RelDbRespInProbeEta->Fill(etaProbe, inputs.relDbResp, weight);
    p1BisectorMpfRespInProbeEta->Fill(etaProbe, inputs.respMetOnBisector, weight);
    p1RelMpfRespInProbeEta->Fill(etaProbe, inputs.relMpfResp, weight);
    p1RelMpfxRespInProbeEta->Fill(etaProbe, inputs.relMpfxResp, weight);

    p2RelDbRespInProbePtProbeEta->Fill(etaProbe, ptProbe, inputs.relDbResp, weight);
    p2BisectorMpfRespInProbePtProbeEta->Fill(etaProbe, ptProbe, inputs.respMetOnBisector, weight);
    p2RelMpfRespInProbePtProbeEta->Fill(etaProbe, ptProbe, inputs.relMpfResp, weight);
    p2RelMpfxRespInProbePtProbeEta->Fill(etaProbe, ptProbe, inputs.relMpfxResp, weight);
}

