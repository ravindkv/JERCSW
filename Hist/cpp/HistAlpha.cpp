#include "HistAlpha.h"
#include "HelperDir.hpp"      // for HelperDir::createTDirectory
#include "VarBin.h"      // full definition of VarBin

#include <sstream>       // std::ostringstream
#include <iomanip>       // std::setprecision
#include <iostream>      // std::cerr

// ROOT headers
#include <TH1D.h>
#include <TH2D.h>
#include <TProfile.h>
#include <TProfile2D.h>
#include <TDirectory.h>

// out‑of‑line destructor so header can just forward‑declare ROOT types
HistAlpha::~HistAlpha() = default;


HistAlpha::HistAlpha(TDirectory* origDir, const std::string& directoryName, const VarBin& varBin, const std::vector<double>& alphaCuts)
    : alphaCuts_(alphaCuts)
{
    // Create a subdirectory for HistAlpha histograms
    std::string dirName = directoryName + "/HistAlpha";
    TDirectory* newDir = HelperDir::createTDirectory(origDir, dirName);
    if (!newDir) {
        std::cerr << "Error: Could not create directory " << dirName << std::endl;
        return;
    }
    newDir->cd();
    
    // Get binning for pT from VarBin
    binsPt_ = varBin.getBinsPt();
    nPt_ = binsPt_.size() - 1;

    std::vector<double> binsAlpha = varBin.getBinsAlpha();
    int nAlphaForP = binsAlpha.size() -1;

    std::vector<double> rangeAlpha = varBin.getRangeAlpha();
    int nAlpha   = rangeAlpha.at(0);
    double minAlpha = rangeAlpha.at(1);
    double maxAlpha = rangeAlpha.at(2);
    
    h1EventInAlpha_ = std::make_unique<TH1D>("h1EventInAlpha", "", nAlpha, minAlpha, maxAlpha);
    p1DbRespInAlphaForProbePt175to230_ = std::make_unique<TProfile>("p1DbRespInAlphaForProbePt175to230", "", nAlphaForP, binsAlpha.data());
    p1DbRespInMaxAlphaForProbePt175to230_ = std::make_unique<TProfile>("p1DbRespInMaxAlphaForProbePt175to230", "", nAlphaForP, binsAlpha.data());
    p1MpfRespInAlphaForProbePt175to230_ = std::make_unique<TProfile>("p1MpfRespInAlphaForProbePt175to230", "", nAlphaForP, binsAlpha.data());
    p1MpfRespInMaxAlphaForProbePt175to230_ = std::make_unique<TProfile>("p1MpfRespInMaxAlphaForProbePt175to230", "", nAlphaForP, binsAlpha.data());
    p1MpfnRespInAlphaForProbePt175to230_ = std::make_unique<TProfile>("p1MpfnRespInAlphaForProbePt175to230", "", nAlphaForP, binsAlpha.data());
    p1MpfnRespInMaxAlphaForProbePt175to230_ = std::make_unique<TProfile>("p1MpfnRespInMaxAlphaForProbePt175to230", "", nAlphaForP, binsAlpha.data());
    p1MpfuRespInAlphaForProbePt175to230_ = std::make_unique<TProfile>("p1MpfuRespInAlphaForProbePt175to230", "", nAlphaForP, binsAlpha.data());
    p1MpfuRespInMaxAlphaForProbePt175to230_ = std::make_unique<TProfile>("p1MpfuRespInMaxAlphaForProbePt175to230", "", nAlphaForP, binsAlpha.data());

    // Initialize 2D Histograms
    h2EventInProbePtAlpha_ = std::make_unique<TH2D>(
        "h2EventInProbePtAlpha", "", nAlpha, minAlpha, maxAlpha, nPt_, binsPt_.data());
    p2DbRespInProbePtAlpha_ = std::make_unique<TProfile2D>(
        "p2DbRespInProbePtAlpha", "", nAlpha, minAlpha, maxAlpha, nPt_, binsPt_.data());
    p2MpfRespInProbePtAlpha_ = std::make_unique<TProfile2D>(
        "p2MpfRespInProbePtAlpha", "", nAlpha, minAlpha, maxAlpha, nPt_, binsPt_.data());
    p2MpfnRespInProbePtAlpha_ = std::make_unique<TProfile2D>(
        "p2MpfnRespInProbePtAlpha", "", nAlpha, minAlpha, maxAlpha, nPt_, binsPt_.data());
    p2MpfuRespInProbePtAlpha_ = std::make_unique<TProfile2D>(
        "p2MpfuRespInProbePtAlpha", "", nAlpha, minAlpha, maxAlpha, nPt_, binsPt_.data());
    
    // Initialize histograms for each α‐cut value
    for (double alphaCut : alphaCuts_) {
        InitializeHistogramsForAlpha(alphaCut, newDir);
    }
    
    // Return to the original directory
    origDir->cd();
}

void HistAlpha::InitializeHistogramsForAlpha(double alphaCut, TDirectory* dir) {
    // Format the alpha cut value as a string (e.g., "0.30")
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << alphaCut;
    std::string alphaStr = oss.str();
    std::replace(alphaStr.begin(), alphaStr.end(), '.', 'p');
    
    // Create histogram names based on the α‐cut value
    std::string h1Name     = "h1EventInProbePtForAlphaBelow" + alphaStr;
    std::string p1RhoName  = "p1RhoInProbePtForAlphaBelow" + alphaStr;
    std::string p1DpbName  = "p1DpbRespInProbePtForAlphaBelow" + alphaStr;
    std::string p1MpfName  = "p1MpfRespInProbePtForAlphaBelow" + alphaStr;
    std::string p1MpfnName  = "p1MpfnRespInProbePtForAlphaBelow" + alphaStr;
    std::string p1MpfuName  = "p1MpfuRespInProbePtForAlphaBelow" + alphaStr;
    
    // Create and store the histograms/TProfiles using the pT binning
    h1EventInProbePtForAlpha_[alphaCut] = std::make_unique<TH1D>(h1Name.c_str(), h1Name.c_str(), nPt_, binsPt_.data());
    p1RhoInProbePtForAlpha_[alphaCut]  = std::make_unique<TProfile>(p1RhoName.c_str(), p1RhoName.c_str(), nPt_, binsPt_.data());
    p1DpbRespInProbePtForAlpha_[alphaCut] = std::make_unique<TProfile>(p1DpbName.c_str(), p1DpbName.c_str(), nPt_, binsPt_.data());
    p1MpfRespInProbePtForAlpha_[alphaCut] = std::make_unique<TProfile>(p1MpfName.c_str(), p1MpfName.c_str(), nPt_, binsPt_.data());
    p1MpfnRespInProbePtForAlpha_[alphaCut] = std::make_unique<TProfile>(p1MpfnName.c_str(), p1MpfnName.c_str(), nPt_, binsPt_.data());
    p1MpfuRespInProbePtForAlpha_[alphaCut] = std::make_unique<TProfile>(p1MpfuName.c_str(), p1MpfuName.c_str(), nPt_, binsPt_.data());
}

void HistAlpha::Fill(double eventAlpha, double ptProbe, double rho, double dbResp, double mpfResp, double mpfnResp, double mpfuResp, double weight) {
    // Fill the event alpha distribution histogram.
    h1EventInAlpha_->Fill(eventAlpha, weight);
    
    if (ptProbe > 175 && ptProbe < 230) {
        // Exclusive profiles: one fill per event based on its α value.
        p1DbRespInAlphaForProbePt175to230_->Fill(eventAlpha, dbResp, weight);
        p1MpfRespInAlphaForProbePt175to230_->Fill(eventAlpha, mpfResp, weight);
        p1MpfnRespInAlphaForProbePt175to230_->Fill(eventAlpha, mpfnResp, weight);
        p1MpfuRespInAlphaForProbePt175to230_->Fill(eventAlpha, mpfuResp, weight);

        // Cumulative profiles: loop over all bins.
        auto fillCumulative = [&](TProfile* prof, double yval) {
            const int nb = prof->GetNbinsX();
            auto* ax = prof->GetXaxis();
            for (int i = 1; i <= nb; ++i) {
                const double binUpperEdge = ax->GetBinUpEdge(i);
                if (eventAlpha < binUpperEdge) {
                    const double xval = ax->GetBinCenter(i);
                    prof->Fill(xval, yval, weight);
                }
            }
        };

        fillCumulative(p1DbRespInMaxAlphaForProbePt175to230_.get(),  dbResp);
        fillCumulative(p1MpfRespInMaxAlphaForProbePt175to230_.get(), mpfResp);
        fillCumulative(p1MpfnRespInMaxAlphaForProbePt175to230_.get(), mpfnResp);
        fillCumulative(p1MpfuRespInMaxAlphaForProbePt175to230_.get(), mpfuResp);
    }

    h2EventInProbePtAlpha_->Fill(eventAlpha, ptProbe, weight);
    p2DbRespInProbePtAlpha_->Fill(eventAlpha, ptProbe, dbResp, weight);
    p2MpfRespInProbePtAlpha_->Fill(eventAlpha, ptProbe, mpfResp, weight);
    p2MpfnRespInProbePtAlpha_->Fill(eventAlpha, ptProbe, mpfnResp, weight);
    p2MpfuRespInProbePtAlpha_->Fill(eventAlpha, ptProbe, mpfuResp, weight);
    
    // Loop over each α‐cut and fill the corresponding exclusive histograms.
    for (const auto& entry : h1EventInProbePtForAlpha_) {
        double alphaCut = entry.first;
        if (eventAlpha < alphaCut) {
            entry.second->Fill(ptProbe, weight);
            p1RhoInProbePtForAlpha_[alphaCut]->Fill(ptProbe, rho, weight);
            p1DpbRespInProbePtForAlpha_[alphaCut]->Fill(ptProbe, dbResp, weight);
            p1MpfRespInProbePtForAlpha_[alphaCut]->Fill(ptProbe, mpfResp, weight);
            p1MpfnRespInProbePtForAlpha_[alphaCut]->Fill(ptProbe, mpfnResp, weight);
            p1MpfuRespInProbePtForAlpha_[alphaCut]->Fill(ptProbe, mpfuResp, weight);
        }
    }
}

void HistAlpha::Fill(double eventAlpha, double rho, const HistL3ResidualInput& in)
{
    const double ptProbe = in.ptTag;

    Fill(eventAlpha,
         ptProbe,
         rho,
         in.respDb,
         in.respMpf,
         in.respMpfn,
         in.respMpfu,
         in.weight);
}
