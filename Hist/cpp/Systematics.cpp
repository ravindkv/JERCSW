#include "Systematics.h"

Systematics::Systematics(const GlobalFlag& globalFlags)
  : globalFlags_(globalFlags),
    systFlag_(globalFlags_.getSystematic())
{}

void Systematics::compute(const SkimTree& skimT)
{
    // Reset all to unity
    qSqrUp_ = qSqrDown_ = 1.0;
    pdfUp_ = pdfDown_ = 1.0;
    isrUp_ = isrDown_ = fsrUp_ = fsrDown_ = 1.0;
    genScaleWeights_.clear();
    pdfWeights_.clear();

    switch (systFlag_) {
        case GlobalFlag::Systematic::QsqrUp:
        case GlobalFlag::Systematic::QsqrDown:
            //computeQsqr(skimT);
            break;

        case GlobalFlag::Systematic::PdfUp:
        case GlobalFlag::Systematic::PdfDown:
            //computePdf(skimT);
            break;

        case GlobalFlag::Systematic::IsrUp:
        case GlobalFlag::Systematic::IsrDown:
        case GlobalFlag::Systematic::FsrUp:
        case GlobalFlag::Systematic::FsrDown:
            computeIsrFsr(skimT);
            break;

        case GlobalFlag::Systematic::Base:
            break;
        default:
            if (globalFlags_.isDebug()) {
                std::cerr << "[Systematics] Unknown systematic flag: "
                          << static_cast<int>(systFlag_) << '\n';
            }
            break;
    }

    if (globalFlags_.isDebug()) {
        print();
    }
}
/*
void Systematics::computeQsqr(const SkimTree& skimT)
{
    if (skimT.nLHEScaleWeight == 9) {
        for (int i = 0; i < 9; ++i) {
            if (i == 2 || i == 6) continue;
            genScaleWeights_.push_back(skimT.LHEScaleWeight[i]);
        }
        double qSqrNominal = skimT.LHEScaleWeight[4];
        if (qSqrNominal != 0.0) {
            double maxW = *std::max_element(genScaleWeights_.begin(), genScaleWeights_.end());
            double minW = *std::min_element(genScaleWeights_.begin(), genScaleWeights_.end());
            qSqrUp_   = maxW / qSqrNominal;
            qSqrDown_ = minW / qSqrNominal;
        }
    }
    else if (skimT.nLHEScaleWeight == 44) {
        static constexpr int idxs[] = {0, 5, 15, 24, 34, 39};
        for (int idx : idxs) {
            genScaleWeights_.push_back(skimT.LHEScaleWeight[idx]);
        }
        qSqrUp_   = *std::max_element(genScaleWeights_.begin(), genScaleWeights_.end());
        qSqrDown_ = *std::min_element(genScaleWeights_.begin(), genScaleWeights_.end());
    }
}

void Systematics::computePdf(const SkimTree& skimT)
{
    double mean = 0.0;
    for (int i = 0; i < skimT.nLHEPdfWeight; ++i) {
        double w = skimT.LHEPdfWeight[i];
        pdfWeights_.push_back(w);
        mean += w;
    }
    if (!pdfWeights_.empty()) {
        mean /= pdfWeights_.size();
        double var = 0.0;
        for (double w : pdfWeights_) {
            var += (w - mean) * (w - mean);
        }
        double sigma = std::sqrt(var / pdfWeights_.size());
        if (mean == 0.0) mean = 1.0;
        pdfUp_   = 1.0 + sigma / mean;
        pdfDown_ = 1.0 - sigma / mean;
    }
}
*/
void Systematics::computeIsrFsr(const SkimTree& skimT)
{
    if (skimT.nPSWeight >= 4 && skimT.genWeight != 0.0) {
        double norm = skimT.PSWeight[4] == 0.0 ? 1.0 : skimT.PSWeight[4];
        isrUp_   = skimT.PSWeight[2] / norm;
        isrDown_ = skimT.PSWeight[0] / norm;
        fsrUp_   = skimT.PSWeight[3] / norm;
        fsrDown_ = skimT.PSWeight[1] / norm;
    }
}

double Systematics::getSystValue() const
{
    switch (systFlag_) {
        case GlobalFlag::Systematic::QsqrUp:      return qSqrUp_;
        case GlobalFlag::Systematic::QsqrDown:    return qSqrDown_;
        case GlobalFlag::Systematic::PdfUp:     return pdfUp_;
        case GlobalFlag::Systematic::PdfDown:   return pdfDown_;
        case GlobalFlag::Systematic::IsrUp:     return isrUp_;
        case GlobalFlag::Systematic::IsrDown:   return isrDown_;
        case GlobalFlag::Systematic::FsrUp:     return fsrUp_;
        case GlobalFlag::Systematic::FsrDown:   return fsrDown_;
        default:                                return 1.0;
    }
}

void Systematics::print() const
{
    std::cout << "=== Systematics Debug Info ===\n";
    std::cout << "  Qsqr Up/Down  = "   << qSqrUp_    << " / " << qSqrDown_    << "\n";
    std::cout << "  Pdf Up/Down = "   << pdfUp_   << " / " << pdfDown_   << "\n";
    std::cout << "  ISR Up/Down = "   << isrUp_   << " / " << isrDown_   << "\n";
    std::cout << "  FSR Up/Down = "   << fsrUp_   << " / " << fsrDown_   << "\n";
    std::cout << "==============================\n";
}
