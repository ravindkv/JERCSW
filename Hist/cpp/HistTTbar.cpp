#include "HistTTbar.h"
#include "HelperDir.hpp"
#include "VarBin.h"

#include <TH1D.h>
#include <TProfile.h>
#include <TLorentzVector.h>
#include <TDirectory.h>
#include <iostream>

using PairOV = std::pair<Obj,Var>;
using TupOVV = std::tuple<Obj,Var,Var>;

// static definitions
const std::array<Var,7> HistTTbar::kVars     = {Var::Pt, Var::Eta, Var::Phi, Var::Mass, Var::Px, Var::Py, Var::Pz};
const std::array<std::string,7> HistTTbar::kVarNames = {"Pt","Eta","Phi","Mass", "Px", "Py", "Pz"};
const std::array<std::function<double(const TLorentzVector&)>,7> HistTTbar::kExtract = {
    [](auto& p){ return p.Pt(); },
    [](auto& p){ return p.Eta();},
    [](auto& p){ return p.Phi();},
    [](auto& p){ return p.M();  },
    [](auto& p){ return p.Px(); },
    [](auto& p){ return p.Py(); },
    [](auto& p){ return p.Pz(); }
};

// helper to stringify your Obj enum
static const char* ObjName(Obj o) {
    switch (o) {
      case Obj::HadW:    return "HadW";
      case Obj::HadT:    return "HadT";
      case Obj::LepT:    return "LepT";
      case Obj::HadB:    return "HadB";
      case Obj::Jet1:    return "HadJet1";
      case Obj::Jet2:    return "HadJet2";
      case Obj::LepB:    return "LepB";
      case Obj::Lep:     return "Lep";
      case Obj::FullMet: return "FullMet";
      default:           return "UnknownObj";
    }
}

HistTTbar::~HistTTbar() = default;

HistTTbar::HistTTbar(TDirectory* origDir,
                     const std::string& dirName,
                     const VarBin& varBin)
{
    InitializeHistograms(origDir, dirName, varBin);
}

void HistTTbar::InitializeHistograms(TDirectory* origDir,
                                     const std::string& directoryName,
                                     const VarBin& varBin)
{
    std::string fullDir = directoryName + "/HistTTbar";
    TDirectory* sub = HelperDir::createTDirectory(origDir, fullDir);
    sub->cd();

    // grab all bin arrays once
    auto binsPt    = varBin.getBinsPt();
    auto binsEta   = varBin.getBinsEta();
    auto binsPhi   = varBin.getBinsPhi();
    auto pxRange   = varBin.getRangePx();
    auto massRange   = varBin.getRangeMassTop();
    auto runRange  = varBin.getRangeRun();

    // list all objects
    const std::array<Obj,9> allObjs = {
        Obj::HadW, Obj::HadT, Obj::LepT,
        Obj::HadB, Obj::Jet1, Obj::Jet2,
        Obj::LepB, Obj::Lep,   Obj::FullMet
    };

    // book 1D histograms for each obj×var
    for (auto o : allObjs) {
        for (auto v : kVars) {
            if(v==Var::Mass){
                Book1D(o, v, int(massRange[0]), massRange[1], massRange[2]);
            }
            else if(v==Var::Px || v==Var::Py || v==Var::Pz){
                Book1D(o, v, int(pxRange[0]), pxRange[1], pxRange[2]);
            }
            else{
                const auto& bins = (v==Var::Pt    ? binsPt   :
                                    v==Var::Eta   ? binsEta  :
                                                binsPhi );
                Book1D(o, v, bins);
            }
        }
    }

    // 1) mass vs (Pt,Eta,Phi): var‑width X, auto Y
    for (auto o : allObjs) {
      for (auto vx : {Var::Pt,Var::Eta,Var::Phi}) {
        const auto& binsX = (vx==Var::Pt  ? binsPt :
                             vx==Var::Eta ? binsEta :
                                            binsPhi);
        BookProf(o, vx, Var::Mass, binsX);
      }
    }

    // 2) mass vs run: uniform run bins, auto Y
    for (auto o : allObjs) {
      BookProf(o, Var::Mass, Var::Mass,
               int(runRange[0]),   // nBins
               runRange[1],        // runMin
               runRange[2]         // runMax
      );
    }

    // extras
    auto avgBins = binsPt; // reuse Pt-bins for avgHadPt
    auto chiBins = varBin.getRangeChiSqr();
    hist_.h1AvgHadPt = std::make_unique<TH1D>(
        "h1EventInAvgHadPt","", int(avgBins.size()-1), avgBins.data());
    hist_.h1ChiSqr   = std::make_unique<TH1D>(
        "h1EventInChiSqr","", int(chiBins[0]), chiBins[1], chiBins[2]);

    origDir->cd();
    std::cout << "Initialized HistTTbar in " << fullDir << "\n";
}

TH1D* HistTTbar::Book1D(Obj o, Var v, const std::vector<double>& bins) {
    std::string oname = ObjName(o);
    std::string vname = kVarNames[int(v)];
    auto h = std::make_unique<TH1D>(
       ("h1EventIn"+oname+vname).c_str(), "", 
       int(bins.size()-1), bins.data()
    );
    auto ptr = h.get();
    hist_.h1[{o,v}] = std::move(h);
    return ptr;
}

TH1D* HistTTbar::Book1D(Obj o, Var v, 
                              int nBinsX,
                              double xLow,
                              double xHigh)
{
    std::string oname = ObjName(o);
    std::string vname = kVarNames[int(v)];
    auto h = std::make_unique<TH1D>(
       ("h1EventIn"+oname+vname).c_str(), "", 
       nBinsX, xLow, xHigh
    );
    auto ptr = h.get();
    hist_.h1[{o,v}] = std::move(h);
    return ptr;
}

// -- variable‑width X bins, auto Y‑range
TProfile* HistTTbar::BookProf(Obj o, Var vx, Var vy,
                              const std::vector<double>& binsX)
{
    std::string oname = ObjName(o);
    std::string xm   = kVarNames[int(vx)];
    std::string ym   = kVarNames[int(vy)];
    std::string nm   = "p1"+ oname + ym + "In" + oname + xm;

    // ROOT: TProfile(name,title,nbinsx, xbins_array)
    auto p = std::make_unique<TProfile>(
        nm.c_str(), "", 
        Int_t(binsX.size()-1), binsX.data()
    );

    TProfile* ptr = p.get();
    hist_.p1[{o,vx,vy}] = std::move(p);
    return ptr;
}

// -- uniform bins [xLow..xHigh], auto Y‑range
TProfile* HistTTbar::BookProf(Obj o, Var vx, Var vy,
                              int nBinsX,
                              double xLow,
                              double xHigh)
{
    std::string oname = ObjName(o);
    std::string xm   = kVarNames[int(vx)];
    std::string ym   = kVarNames[int(vy)];
    std::string nm   = "p1"+ oname + ym + "InRun";

    // ROOT: TProfile(name,title,nbinsx, xlow, xup)
    auto p = std::make_unique<TProfile>(
        nm.c_str(), "", 
        nBinsX, xLow, xHigh
    );

    TProfile* ptr = p.get();
    hist_.p1[{o,vx,vy}] = std::move(p);
    return ptr;
}


void HistTTbar::Fill1D(Obj o, Var v, double x, double w) {
    hist_.h1.at({o,v})->Fill(x, w);
}

void HistTTbar::FillProfile(Obj o, Var vx, Var vy, double x, double y, double w) {
    hist_.p1.at({o,vx,vy})->Fill(x, y, w);
}

void HistTTbar::FillP4(const TLorentzVector& p4,
                       Obj which, double run, double weight)
{
    // 1D
    for (size_t iv=0; iv<kVars.size(); ++iv) {
        double x = kExtract[iv](p4);
        Fill1D(which, kVars[iv], x, weight);
    }
    // profiles vs mass
    double m = p4.M();
    for (auto vx : {Var::Pt,Var::Eta,Var::Phi}) {
        double x = kExtract[int(vx)](p4);
        FillProfile(which, vx, Var::Mass, x, m, weight);
    }
    // mass vs run
    FillProfile(which, Var::Mass, Var::Mass, run, m, weight);
}

void HistTTbar::FillExtras(double avgHadPt, double chiSqr, double pzMet, double weight) {
    hist_.h1AvgHadPt->Fill(avgHadPt, weight);
    hist_.h1ChiSqr  ->Fill(chiSqr,   weight);
}

void HistTTbar::FillAll(const TLorentzVector& p4HadW,
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
                        double weight)
{
    FillP4(p4HadW,   Obj::HadW,   run, weight);
    FillP4(p4HadT,   Obj::HadT,   run, weight);
    FillP4(p4LepT,   Obj::LepT,   run, weight);
    FillP4(p4HadB,   Obj::HadB,   run, weight);
    FillP4(p4Jet1,   Obj::Jet1,   run, weight);
    FillP4(p4Jet2,   Obj::Jet2,   run, weight);
    FillP4(p4LepB,   Obj::LepB,   run, weight);
    FillP4(p4Lep,    Obj::Lep,    run, weight);
    FillP4(p4FullMet,Obj::FullMet,run, weight);

    FillExtras(avgHadPt, chiSqr, p4FullMet.Pz(), weight);
}

