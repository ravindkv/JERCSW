// cpp/HistCutflow.cpp
#include "HistCutflow.h"
#include "HelperDir.hpp"            // for directory creation
#include <TH1D.h>
#include <TDirectory.h>
#include <iostream>
#include <iomanip>
#include <sstream>

HistCutflow::~HistCutflow() = default;


HistCutflow::HistCutflow(TDirectory* origDir, const std::string& directoryName, const std::vector<std::string>& cuts, const GlobalFlag& globalFlags)
    : cutNames_(cuts),
    globalFlags_(globalFlags),
    year_(globalFlags_.getYear()),
    channel_(globalFlags_.getChannel()),
    isDebug_(globalFlags_.isDebug()){

    // Create or retrieve the desired directory within the original directory
    std::string dirName = directoryName + "/HistCutflow";
    TDirectory* newDir = HelperDir::createTDirectory(origDir, dirName); 
    if (!newDir) {
        std::cerr << "Error: Failed to create or retrieve directory \"" << dirName << "\"." << std::endl;
        return;
    }
    newDir->cd();

    // Initialize the histogram with appropriate binning
    h1EventInCutflow_ = std::make_unique<TH1D>("h1EventInCutflow", "", 
                                              static_cast<int>(cuts.size()), 0.5, cuts.size() + 0.5);
    h1EventInCutflowWithWeight_ = std::make_unique<TH1D>("h1EventInCutflowWithWeight", "", 
                                              static_cast<int>(cuts.size()), 0.5, cuts.size() + 0.5);
    h1EventFractionInCutflow_ = std::make_unique<TH1D>("h1EventFractionInCutflow", "", 
                                              static_cast<int>(cuts.size()), 0.5, cuts.size() + 0.5);


    // Initialize the cut to bin mapping and set bin labels
    for (size_t i = 0; i < cutNames_.size(); ++i) {
        // ROOT bins are 1-based
        cutToBinMap[cutNames_[i]] = static_cast<int>(i + 1);
        h1EventInCutflow_->GetXaxis()->SetBinLabel(static_cast<int>(i + 1), cutNames_[i].c_str());
        h1EventInCutflowWithWeight_->GetXaxis()->SetBinLabel(static_cast<int>(i + 1), cutNames_[i].c_str());
        h1EventFractionInCutflow_->GetXaxis()->SetBinLabel(static_cast<int>(i + 1), cutNames_[i].c_str());
    }

    // Return to the original directory
    origDir->cd();
}

void HistCutflow::fill(const std::string& cutName, double weight) {
    auto it = cutToBinMap.find(cutName);
    if (it != cutToBinMap.end()) {
        if(isDebug_) std::cout<<"passed cutflow: "<<cutName<<"\n";
        h1EventInCutflow_->Fill(it->second);
        h1EventInCutflowWithWeight_->Fill(it->second, weight);
    } else {
        std::cerr << "Warning: Cut name \"" << cutName << "\" not found in cutToBinMap.\n";
    }
}

bool HistCutflow::validateHistogram() const {
    if (!h1EventInCutflow_) {
        std::cerr << "Error: Histogram pointer is null.\n";
        return false;
    }

    int nBins = h1EventInCutflow_->GetNbinsX();
    if (nBins < 1) {
        std::cerr << "Error: Histogram has no bins.\n";
        return false;
    }

    return true;
}

void HistCutflow::fillFractionCutflow() const {
    validateHistogram();

    // Initialize the first bin to 1.0 (100%)
    h1EventFractionInCutflow_->SetBinContent(1, 1.0);

    double firstContent = h1EventInCutflow_->GetBinContent(1);
    int nBins = h1EventInCutflow_->GetNbinsX();
    // Loop over the remaining bins to compute fractions
    for (int bin = 2; bin <= nBins; ++bin) {
        double currentContent = h1EventInCutflow_->GetBinContent(bin);
        double fraction = (firstContent != 0.0) ? (currentContent / firstContent) : 0.0;

        h1EventFractionInCutflow_->SetBinContent(bin, fraction);

    }
}

void HistCutflow::printCutflow(std::ostream& outputStream) const {
    if (!validateHistogram()) {
        std::cerr << "Error: Invalid histogram. Cannot print cutflow.\n";
        return;
    }

    int nBins = h1EventInCutflow_->GetNbinsX();

    // Print the header with updated columns
    outputStream << "---------: Cutflow Summary :--------" << '\n';
    outputStream << std::left
                << std::setw(20) << "CUT"
                << std::right
                << std::setw(15) << "ENTRIES"
                << std::setw(25) << "REMAINED w.r.t first"
                << std::setw(25) << "REMAINED w.r.t previous"
                << '\n';
    outputStream << std::string(90, '-') << '\n';

    // Store the first bin's content for reference
    double firstBinContent = h1EventInCutflow_->GetBinContent(1);
    double previousContent = firstBinContent;

    // Retrieve the first bin's label; handle empty labels
    std::string firstBinLabel = h1EventInCutflow_->GetXaxis()->GetBinLabel(1);
    if (firstBinLabel.empty()) {
        firstBinLabel = "Bin 1";
    }

    // Print the first bin's content
    outputStream << std::left
                << std::setw(20) << firstBinLabel
                << std::right
                << std::setw(15) << firstBinContent
                << std::setw(25) << "100.0 %"  // Remained w.r.t first bin
                << std::setw(25) << "N/A"       // Remained w.r.t previous bin
                << '\n';

    // Loop over the remaining bins to compute and print statistics
    for (int bin = 2; bin <= nBins; ++bin) {
        double currentContent = h1EventInCutflow_->GetBinContent(bin);
        
        // Calculate fractions
        double fractionFirst = (firstBinContent != 0.0) ? (currentContent / firstBinContent) * 100.0 : 0.0;
        double fractionPrev = (previousContent != 0.0) ? (currentContent / previousContent) * 100.0 : 0.0;

        // Retrieve the bin label; handle empty labels
        std::string binLabel = h1EventInCutflow_->GetXaxis()->GetBinLabel(bin);
        if (binLabel.empty()) {
            binLabel = "Bin " + std::to_string(bin);
        }

        // Format the fractions to one decimal place
        std::ostringstream fractionFirstStream, fractionPrevStream;
        fractionFirstStream << std::fixed << std::setprecision(2) << fractionFirst;
        fractionPrevStream << std::fixed << std::setprecision(2) << fractionPrev;

        // Print the cut name, entries, and percentage changes
        outputStream << std::left
                    << std::setw(20) << binLabel
                    << std::right
                    << std::setw(15) << currentContent
                    << std::setw(25) << (fractionFirstStream.str() + " %")
                    << std::setw(25) << (fractionPrevStream.str() + " %")
                    << '\n';

        // Update previousContent for the next iteration
        previousContent = currentContent;
    }

    outputStream << std::endl;
}


