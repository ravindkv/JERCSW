#include "MathHadW.h"
#include <algorithm>
#include <iostream>
#include <limits> // for std::numeric_limits

MathHadW::MathHadW(const GlobalFlag& globalFlags,
                   double bTagThresh,
                   double wMass,
                   double sigmaW)
    : globalFlags_(globalFlags),
      bTagThreshold_(bTagThresh),
      wMass_(wMass),
      sigmaW_(sigmaW)
{
    index1ForW_ = -1;
    index2ForW_ = -1;
    diJet_.SetPtEtaPhiM(0, 0, 0, 0);
    bestChi2_ = std::numeric_limits<double>::max();
}

const TLorentzVector MathHadW::getP4HadW(const SkimTree& skimT,
                                         const std::vector<int>& indexJets)
{
    // Reset from any previous call
    index1ForW_ = -1;
    index2ForW_ = -1;
    diJet_.SetPtEtaPhiM(0, 0, 0, 0);
    bestChi2_ = std::numeric_limits<double>::max();

    // 1) Sort jets by pT descending
    std::vector<std::pair<float, int>> jetPtIndex;
    jetPtIndex.reserve(indexJets.size());
    for (int jIdx : indexJets) {
        jetPtIndex.emplace_back(skimT.Jet_pt[jIdx], jIdx);
    }
    std::sort(jetPtIndex.begin(), jetPtIndex.end(),
              [](auto &a, auto &b){ return a.first > b.first; });

    // Build sorted indices
    std::vector<int> sortedJets;
    sortedJets.reserve(jetPtIndex.size());
    for (auto &p : jetPtIndex) {
        sortedJets.push_back(p.second);
    }

    // 2) Take leading 8 jets if available
    size_t nLead = std::min<size_t>(sortedJets.size(), 8);
    std::vector<int> leadingJets(sortedJets.begin(), sortedJets.begin() + nLead);

    if (leadingJets.size() < 4) {
        // We need at least 2 b-jets + 2 other jets for W
        if (globalFlags_.isDebug()) {
            std::cerr << "[MathHadW] Fewer than 4 jets among leading. Returning empty.\n";
        }
        return diJet_;
    }

    // 3) Identify b-jets above threshold. Sort them by bTag value descending.
    std::vector<std::pair<double,int>> bCandidates; // (bDiscr, idx)
    std::vector<int> nonBJets;
    for (int jIdx : leadingJets) {
        double bVal = skimT.Jet_btagDeepFlavB[jIdx];
        if (bVal > bTagThreshold_) {
            bCandidates.emplace_back(bVal, jIdx);
        } else {
            nonBJets.push_back(jIdx);
        }
    }
    std::sort(bCandidates.begin(), bCandidates.end(),
              [](auto &a, auto &b){ return a.first > b.first; });

    if (bCandidates.size() < 2) {
        // Need at least 2 b-jets
        if (globalFlags_.isDebug()) {
            std::cerr << "[MathHadW] Not enough b-jets above threshold.\n";
        }
        return diJet_;
    }

    // Pick the 2 highest-bTag jets as b1, b2
    int b1 = bCandidates[0].second;
    int b2 = bCandidates[1].second;

    // The "remaining" jets = leadingJets minus these 2 b-jets
    std::vector<int> remainingJets;
    remainingJets.reserve(leadingJets.size());
    for (int jIdx : leadingJets) {
        if (jIdx != b1 && jIdx != b2) {
            remainingJets.push_back(jIdx);
        }
    }

    // 4) Among the "remainingJets", find the pair (i,j) that yields minimal chi^2 wrt W mass
    //    If we have fewer than 2 "remainingJets", we can't form the W, return empty.
    if (remainingJets.size() < 2) {
        if (globalFlags_.isDebug()) {
            std::cerr << "[MathHadW] Fewer than 2 remaining jets for W.\n";
        }
        return diJet_;
    }

    // We do a small brute-force over all pairs in remainingJets
    for (size_t i = 0; i < remainingJets.size(); ++i) {
        for (size_t j = i+1; j < remainingJets.size(); ++j) {

            int idx1 = remainingJets[i];
            int idx2 = remainingJets[j];

            // Build the 4-vector sum
            TLorentzVector p4j1, p4j2, p4W;
            p4j1.SetPtEtaPhiM(skimT.Jet_pt[idx1],
                              skimT.Jet_eta[idx1],
                              skimT.Jet_phi[idx1],
                              skimT.Jet_mass[idx1]);
            p4j2.SetPtEtaPhiM(skimT.Jet_pt[idx2],
                              skimT.Jet_eta[idx2],
                              skimT.Jet_phi[idx2],
                              skimT.Jet_mass[idx2]);
            p4W = p4j1 + p4j2;

            double mW = p4W.M();
            double chi2 = ( (mW - wMass_)*(mW - wMass_) ) / (sigmaW_*sigmaW_);

            if (chi2 < bestChi2_) {
                bestChi2_ = chi2;
                index1ForW_ = idx1;
                index2ForW_ = idx2;
                diJet_ = p4W;
            }
        }
    }

    if (globalFlags_.isDebug()) {
        print();
    }

    return diJet_;
}

void MathHadW::print() const
{
    std::cout << "=== MathHadW with Chi2 Debug Info ===" << std::endl;
    std::cout << "  bTagThreshold    = " << bTagThreshold_ << std::endl;
    std::cout << "  W mass / sigmaW  = " << wMass_ << " / " << sigmaW_ << std::endl;
    std::cout << "  Chosen jets for W: (" << index1ForW_ << ", " << index2ForW_ << ")\n";
    std::cout << "  Best chi^2       = " << bestChi2_ << std::endl;
    std::cout << "  Reconstructed W mass = " << diJet_.M() << std::endl;
    std::cout << "===============================" << std::endl;
}

