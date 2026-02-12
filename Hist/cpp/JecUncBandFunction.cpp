#include "JecUncBandFunction.h"

#include <iostream>
#include <cmath>
#include <algorithm>
#include <cstdint>

JecUncBandFunction::JecUncBandFunction(const GlobalFlag& globalFlags)
    : globalFlags_(globalFlags),
      loader_(globalFlags),
      isDebug_(globalFlags.isDebug())
{}

double JecUncBandFunction::getJesRelUncForBand(double eta, double ptAfterJes ) const {
    const double relUnc = loader_.getJesUncBandRef()->evaluate({ eta, ptAfterJes });
    if (isDebug_) {
        std::cout<< "eta = " + std::to_string(eta) +
                    ", pt After JES = "+ std::to_string(ptAfterJes)+ 
                    ", relUnc=" + std::to_string(relUnc)
                  << '\n';
    }
    return relUnc;
}

// JerSf
double JecUncBandFunction::getJerScaleFactor(double eta, const std::string& syst) const {
    double JerSf = 1.0;
    using Var = correction::Variable::Type;
    std::vector<Var> vals;
    vals.emplace_back(static_cast<double>(eta));       // JetEta
    vals.emplace_back(static_cast<std::string>(syst)); // syst
    try {
        JerSf = loader_.getJerSfUncBandRef()->evaluate(vals);
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

double JecUncBandFunction::getJerSfRelUncForBand(double eta, const std::string& syst) const {
    double sfNom    = getJerScaleFactor(eta, "nom");
    double sfSyst   = getJerScaleFactor(eta, syst);
    double jerRelUnc = 0.0;
    if(sfNom>0.0){
        jerRelUnc = std::abs(sfNom-sfSyst)/sfNom;
    }
    if(isDebug_){
        std::cout<<"sfNom = "<<sfNom
                 <<", sfSyst = "<<sfSyst
                 <<", jerRelUnc = "<<jerRelUnc
                 <<"\n";
    }
    return jerRelUnc;
}

