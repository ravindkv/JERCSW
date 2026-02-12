#pragma once

#include <memory>
#include <map>
#include <string>
#include <vector>
#include <tuple>
#include <functional>

class TDirectory;
class TH1D;
class TProfile;
class TLorentzVector;
class VarBin;

enum class Obj {
    HadW, HadT, LepT,
    HadB, Jet1, Jet2,
    LepB, Lep, FullMet
};

enum class Var { Pt, Eta, Phi, Mass, Px, Py, Pz};

struct TTbarHistograms {
    // 1D histograms indexed by (object, variable)
    std::map<std::pair<Obj,Var>, std::unique_ptr<TH1D>>    h1;
    // profiles indexed by (object, x-variable, y-variable)
    std::map<std::tuple<Obj,Var,Var>, std::unique_ptr<TProfile>> p1;

    // extra single histograms
    std::unique_ptr<TH1D>    h1AvgHadPt, h1ChiSqr;;
    std::unique_ptr<TProfile> p1MassInRun;
};

class HistTTbar {
public:
    HistTTbar(TDirectory* origDir, const std::string& dirName, const VarBin& varBin);
    ~HistTTbar();

    /// Fill *any* TLorentzVector; pick which Obj it is
    void FillP4(const TLorentzVector& p4, Obj which, double run, double weight);
    /// Fill the non-P4 quantities
    void FillExtras(double avgHadPt, double chiSqr, double pzMet, double weight);

    /// Convenience: fill all in one shot
    void FillAll(const TLorentzVector& p4HadW,
                 const TLorentzVector& p4HadT,
                 const TLorentzVector& p4LepT,
                 const TLorentzVector& p4HadB,
                 const TLorentzVector& p4Jet1,
                 const TLorentzVector& p4Jet2,
                 const TLorentzVector& p4LepB,
                 const TLorentzVector& p4Lep,
                 const TLorentzVector& p4FullMet,
                 double avgHadPt,
                 double chiSqr,
                 double run,
                 double weight);

private:
    TTbarHistograms hist_;

    // bookers
    TH1D*     Book1D(Obj o, Var v, const std::vector<double>& bins);
    TH1D*     Book1D(Obj o, Var v, 
                              int nBinsX,
                              double xLow,
                              double xHigh);
    /// Variable‑width X bins, auto Y‑range
    TProfile* BookProf(Obj o, Var vx, Var vy,
                       const std::vector<double>& binsX);

    /// Uniform X bins (e.g. run), auto Y‑range
    TProfile* BookProf(Obj o, Var vx, Var vy,
                   int    nBinsX,
                   double xLow,
                   double xHigh);


    void InitializeHistograms(TDirectory* origDir, const std::string& dirName, const VarBin& varBin);

    // helpers to fill from the maps
    void Fill1D(Obj o, Var v, double x, double w);
    void FillProfile(Obj o, Var vx, Var vy, double x, double y, double w);

    // extractor lambdas
    static const std::array<Var,7>                          kVars;
    static const std::array<std::string,7>                  kVarNames;
    static const std::array<std::function<double(const TLorentzVector&)>,7> kExtract;
};

