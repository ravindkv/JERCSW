#include <memory>            // std::unique_ptr
#include <string>            // std::string
#include <unordered_map>     // std::unordered_map
#include <vector>            // std::vector

// forward declarations
class TDirectory;
class TH1D;
class TProfile;
class SkimTree;
class VarBin;

struct RunHistograms {
    std::unique_ptr<TH1D> h1EventInRun;          
    std::unique_ptr<TProfile> p1DbRespInRun;     
    std::unique_ptr<TProfile> p1MpfRespInRun;    
    std::unique_ptr<TProfile> p1Jet1ChfInRun;   
    std::unique_ptr<TProfile> p1Jet1NhfInRun;   
    std::unique_ptr<TProfile> p1Jet1NefInRun;  
    std::unique_ptr<TProfile> p1Jet1CefInRun;  
    std::unique_ptr<TProfile> p1Jet1MufInRun;  
};

class HistTime {
public:
    HistTime(TDirectory *origDir, const std::string& directoryName, const VarBin& varBin, const std::vector<int>& pTProbes);
    
    ~HistTime();
    
    void Fill(SkimTree* skimT, int iJet1, double bal, double mpf, double ptProbe, double weight);

private:
    // Map from pTProbe threshold to its corresponding RunHistograms
    std::unordered_map<int, RunHistograms> histMap_;
    
    // List of pTProbe thresholds
    std::vector<int> pTProbes_;
    
    // Run histogram parameters
    int runN_;
    double runMin_;
    double runMax_;
    
    void InitializeHistograms(TDirectory *origDir, const std::string& directoryName, const VarBin& varBin);
};

