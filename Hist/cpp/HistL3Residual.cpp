#include "HistL3Residual.h"
#include "HelperDir.hpp"
#include "VarBin.h"

#include <TDirectory.h>
#include <TH1D.h>
#include <TProfile.h>
#include <TProfile2D.h>

#include <cmath>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

// out-of-line destructor (empty)
HistL3Residual::~HistL3Residual() = default;

// Constructor
HistL3Residual::HistL3Residual(TDirectory* origDir,
                               const std::string& directoryName,
                               const VarBin& varBin)
{
    InitializeHistograms(origDir, directoryName, varBin);
    initialized_ = true;
}

void HistL3Residual::requireInitialized_() const
{
    if (!initialized_) {
        throw std::runtime_error("HistL3Residual::fillHistos called before histograms were initialized.");
    }

    // Guard against partially-initialized state
    if (!h1EventInTagPt ||
        !h1EventInProbePt ||
        !h1EventInMetPt ||
        !h1EventInOtherPt ||
        !h1EventInUnclusteredPt ||
        !p1TagPtInTagPt          ||
        !p1ProbePtInTagPt        ||
        !p1MetPtInTagPt          ||
        !p1OtherPtInTagPt        ||
        !p1UnclusteredPtInTagPt  ||
        !h1EventInRespDb ||
        !h1EventInRespMpf ||
        !h1EventInRespMpf1 ||
        !h1EventInRespMpfn ||
        !h1EventInRespMpfu ||
        !h1EventInRespMpfnu ||
        !p1RespDbInProbeEta ||
        !p1RespDbInTagPt ||
        !p1RespMpfInTagPt ||
        !p1RespMpf1InTagPt ||
        !p1RespMpfnInTagPt ||
        !p1RespMpfuInTagPt ||
        !p1RespMpfnuInTagPt ||
        !p2RelDbRespInTagPtProbeEta ||
        !p2RespMpfInInTagPtProbeEta ||
        !p2RespMpf1IInTagPtProbeEta ||
        !p2RespMpfnInTagPtProbeEta ||
        !p2RespMpfuInTagPtProbeEta ||
        !p2RespMpfnuInTagPtProbeEta)
    {
        throw std::runtime_error("HistL3Residual: histogram/profile pointer is null (partial initialization).");
    }
}

void HistL3Residual::InitializeHistograms(TDirectory* origDir,
                                         const std::string& directoryName,
                                         const VarBin& varBin)
{
    if (!origDir) {
        throw std::runtime_error("HistL3Residual::InitializeHistograms - origDir is null");
    }

    const std::string dirName = directoryName + "/HistL3Residual";
    TDirectory* newDir = HelperDir::createTDirectory(origDir, dirName);
    if (!newDir) {
        throw std::runtime_error("HistL3Residual::InitializeHistograms - failed to create/find directory: " + dirName);
    }
    newDir->cd();

    std::vector<double> binsPt  = varBin.getBinsPt();
    const int nPt = static_cast<int>(binsPt.size()) - 1;

    std::vector<double> binsEta = varBin.getBinsEta();
    const int nEta = static_cast<int>(binsEta.size()) - 1;

    // --- TH1 in pT (binned like VarBin pT) ---
    h1EventInTagPt          = new TH1D("h1EventInTagPt",          "", nPt, binsPt.data());
    h1EventInProbePt        = new TH1D("h1EventInProbePt",        "", nPt, binsPt.data());
    h1EventInMetPt          = new TH1D("h1EventInMetPt",          "", nPt, binsPt.data());
    h1EventInOtherPt        = new TH1D("h1EventInOtherPt",        "", nPt, binsPt.data());
    h1EventInUnclusteredPt  = new TH1D("h1EventInUnclusteredPt",  "", nPt, binsPt.data());

    p1TagPtInTagPt        = new TProfile("p1TagPtInTagPt",        "", nPt, binsPt.data());
    p1ProbePtInTagPt      = new TProfile("p1ProbePtInTagPt",      "", nPt, binsPt.data());
    p1MetPtInTagPt        = new TProfile("p1MetPtInTagPt",        "", nPt, binsPt.data());
    p1OtherPtInTagPt      = new TProfile("p1OtherPtInTagPt",      "", nPt, binsPt.data());
    p1UnclusteredPtInTagPt= new TProfile("p1UnclusteredPtInTagPt","", nPt, binsPt.data());

    // --- TH1 responses (generic safe range; adjust later if you add explicit ranges) ---
    constexpr int    nRespBins = 50;
    constexpr double respMin   = 0.0;
    constexpr double respMax   = 2.0;

    h1EventInRespDb    = new TH1D("h1EventInRespDb",    "", nRespBins, respMin, respMax);
    h1EventInRespMpf   = new TH1D("h1EventInRespMpf",   "", nRespBins, respMin, respMax);
    h1EventInRespMpf1  = new TH1D("h1EventInRespMpf1",  "", nRespBins, respMin, respMax);
    h1EventInRespMpfn  = new TH1D("h1EventInRespMpfn",  "", nRespBins, respMin, respMax);
    h1EventInRespMpfu  = new TH1D("h1EventInRespMpfu",  "", nRespBins, respMin, respMax);
    h1EventInRespMpfnu = new TH1D("h1EventInRespMpfnu", "", nRespBins, respMin, respMax);

    // store errors properly for weighted fills
    h1EventInTagPt->Sumw2();
    h1EventInProbePt->Sumw2();
    h1EventInMetPt->Sumw2();
    h1EventInOtherPt->Sumw2();
    h1EventInUnclusteredPt->Sumw2();

    h1EventInRespDb->Sumw2();
    h1EventInRespMpf->Sumw2();
    h1EventInRespMpf1->Sumw2();
    h1EventInRespMpfn->Sumw2();
    h1EventInRespMpfu->Sumw2();
    h1EventInRespMpfnu->Sumw2();

    // --- Profiles vs probe pT ---
    p1RespDbInProbeEta    = new TProfile("p1RespDbInProbeEta",     "", nEta, binsEta.data());
    p1RespDbInTagPt     = new TProfile("p1RespDbInTagPt",     "", nPt, binsPt.data());
    p1RespMpfInTagPt    = new TProfile("p1RespMpfInTagPt",    "", nPt, binsPt.data());
    p1RespMpf1InTagPt   = new TProfile("p1RespMpf1InTagPt",   "", nPt, binsPt.data());
    p1RespMpfnInTagPt   = new TProfile("p1RespMpfnInTagPt",   "", nPt, binsPt.data());
    p1RespMpfuInTagPt   = new TProfile("p1RespMpfuInTagPt",   "", nPt, binsPt.data());
    p1RespMpfnuInTagPt  = new TProfile("p1RespMpfnuInTagPt",  "", nPt, binsPt.data());

    // --- 2D profiles: x=eta, y=pt (kept consistent with existing Fill order used historically) ---
    p2RelDbRespInTagPtProbeEta     = new TProfile2D("p2RelDbRespInTagPtProbeEta",     "", nEta, binsEta.data(), nPt, binsPt.data());
    p2RespMpfInInTagPtProbeEta     = new TProfile2D("p2RespMpfInInTagPtProbeEta",     "", nEta, binsEta.data(), nPt, binsPt.data());
    p2RespMpf1IInTagPtProbeEta     = new TProfile2D("p2RespMpf1IInTagPtProbeEta",     "", nEta, binsEta.data(), nPt, binsPt.data());
    p2RespMpfnInTagPtProbeEta      = new TProfile2D("p2RespMpfnInTagPtProbeEta",      "", nEta, binsEta.data(), nPt, binsPt.data());
    p2RespMpfuInTagPtProbeEta      = new TProfile2D("p2RespMpfuInTagPtProbeEta",      "", nEta, binsEta.data(), nPt, binsPt.data());
    p2RespMpfnuInTagPtProbeEta     = new TProfile2D("p2RespMpfnuInTagPtProbeEta",     "", nEta, binsEta.data(), nPt, binsPt.data());

    // Return to original directory
    origDir->cd();
}

void HistL3Residual::fillHistos(const HistL3ResidualInput& inputs)
{
    requireInitialized_();

    const double weight = inputs.weight;
    if (weight == 0.0) return;

    auto finiteOrThrow = [&](double x, const char* name) {
        if (!std::isfinite(x)) {
            std::ostringstream oss;
            oss << "HistL3Residual::fillHistos - non-finite input: " << name
                << " = " << x
                << " (etaProbe=" << inputs.etaProbe
                << ", ptTag=" << inputs.ptTag
                << ", ptProbe=" << inputs.ptProbe
                << ", weight=" << inputs.weight << ")";
            throw std::runtime_error(oss.str());
        }
    };

    // Validate key coords + weights
    finiteOrThrow(inputs.etaProbe, "etaProbe");
    finiteOrThrow(weight,          "weight");

    // Validate filled quantities
    finiteOrThrow(inputs.ptTag,          "ptTag");
    finiteOrThrow(inputs.ptProbe,        "ptProbe");
    finiteOrThrow(inputs.ptMet,          "ptMet");
    finiteOrThrow(inputs.ptOther,        "ptOther");
    finiteOrThrow(inputs.ptUnclustered,  "ptUnclustered");

    finiteOrThrow(inputs.respDb,    "respDb");
    finiteOrThrow(inputs.respMpf,   "respMpf");
    finiteOrThrow(inputs.respMpf1,  "respMpf1");
    finiteOrThrow(inputs.respMpfn,  "respMpfn");
    finiteOrThrow(inputs.respMpfu,  "respMpfu");
    finiteOrThrow(inputs.respMpfnu, "respMpfnu");

    const double etaProbe = inputs.etaProbe;
    const double ptTag  = inputs.ptTag;

    // TH1 (event distributions)
    h1EventInTagPt->Fill(inputs.ptTag, weight);
    h1EventInProbePt->Fill(inputs.ptProbe, weight);
    h1EventInMetPt->Fill(inputs.ptMet, weight);
    h1EventInOtherPt->Fill(inputs.ptOther, weight);
    h1EventInUnclusteredPt->Fill(inputs.ptUnclustered, weight);

    p1TagPtInTagPt->Fill(ptTag, inputs.ptTag, weight);
    p1ProbePtInTagPt->Fill(ptTag, inputs.ptProbe, weight);
    p1MetPtInTagPt->Fill(ptTag, inputs.ptMet, weight);
    p1OtherPtInTagPt->Fill(ptTag, inputs.ptOther, weight);
    p1UnclusteredPtInTagPt->Fill(ptTag, inputs.ptUnclustered, weight);

    h1EventInRespDb->Fill(inputs.respDb, weight);
    h1EventInRespMpf->Fill(inputs.respMpf, weight);
    h1EventInRespMpf1->Fill(inputs.respMpf1, weight);
    h1EventInRespMpfn->Fill(inputs.respMpfn, weight);
    h1EventInRespMpfu->Fill(inputs.respMpfu, weight);
    h1EventInRespMpfnu->Fill(inputs.respMpfnu, weight);

    // Profiles vs probe pT
    p1RespDbInProbeEta->Fill(etaProbe, inputs.respDb, weight);
    p1RespDbInTagPt->Fill(ptTag, inputs.respDb, weight);
    p1RespMpfInTagPt->Fill(ptTag, inputs.respMpf, weight);
    p1RespMpf1InTagPt->Fill(ptTag, inputs.respMpf1, weight);
    p1RespMpfnInTagPt->Fill(ptTag, inputs.respMpfn, weight);
    p1RespMpfuInTagPt->Fill(ptTag, inputs.respMpfu, weight);
    p1RespMpfnuInTagPt->Fill(ptTag, inputs.respMpfnu, weight);

    // 2D profiles (x=eta, y=pt)
    p2RelDbRespInTagPtProbeEta->Fill(etaProbe, ptTag, inputs.respDb, weight);
    p2RespMpfInInTagPtProbeEta->Fill(etaProbe, ptTag, inputs.respMpf, weight);
    p2RespMpf1IInTagPtProbeEta->Fill(etaProbe, ptTag, inputs.respMpf1, weight);
    p2RespMpfnInTagPtProbeEta->Fill(etaProbe, ptTag, inputs.respMpfn, weight);
    p2RespMpfuInTagPtProbeEta->Fill(etaProbe, ptTag, inputs.respMpfu, weight);
    p2RespMpfnuInTagPtProbeEta->Fill(etaProbe, ptTag, inputs.respMpfnu, weight);
}

