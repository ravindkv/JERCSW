import os
import sys
import json
import itertools
sys.dont_write_bytecode = True
sys.path.insert(0, os.getcwd().replace("input",""))
from Inputs import *

#Reduce number of condor jobs w.r.t Skim by a factor of rData and rMC
def reducedJob(nJob, sKey):
    rData   = reduceJobsDataBy
    rMC     = reduceJobsMCBy
    if "Data" in sKey:
        if nJob>rData:
            n = nJob/rData
        else:
            n = 1
    else:
        if nJob>rMC:
            n = nJob/rMC
        else:
            n = 1
    return int(n)

if __name__=="__main__":
    skimDir = "../../Skim/input/json/"
    os.system("mkdir -p json")
    allJobs = 0
    for algo, jec, year, stage in itertools.product(JetAlgos, JecDerLevel.keys(), Years, HistStages):
        for ch in JecDerLevel[jec]: 
            fSkim = open(f"{skimDir}/FilesSkim_{algo}_{ch}_{year}.json", "r")
            jSkim = json.load(fSkim)

            #Replace the keys (DiJet, IncJet, etc to use same Skims)
            keyMap = {}
            for oldSkimKey in jSkim.keys():
                newSkimKey = oldSkimKey.replace(f"{algo}_", f"{algo}_{jec}_")
                keyMap[oldSkimKey] = newSkimKey
            for oldSkimKey, newSkimKey in keyMap.items():
                if oldSkimKey in jSkim:
                    jSkim[newSkimKey] = jSkim.pop(oldSkimKey)

            fHist = open(f"json/FilesHist{stage}_{algo}_{jec}_{ch}_{year}.json", "w")
            dHist = {}
            print(f"{eosHistDir}{stage}/{algo}/{jec}/{ch}/{year}")
            yJobs = 0
            for sKey in jSkim.keys():
                if year not in sKey:
                    continue
                if "Residual" in jec and "GamJet" in ch and "QCD" in sKey:
                    continue #FIXME ( maybe QCD is not needed, confirm with Mikko)
                systs = []
                if "Data" in sKey:
                    systs = channelDetails[ch][year]['systOnData']
                else:
                    systs = channelDetails[ch][year]['systOnMc']
                lSkim = jSkim[sKey][1]
                nJob  = reducedJob(len(lSkim), sKey)
                for syst in systs:
                    yJobs = yJobs+nJob
                    lHist = []
                    for i in range(nJob):
                        lHist.append("%s%s/%s/%s/%s/%s/%s_Hist%s%s_%sof%s.root"%(eosHistDir, stage, algo, jec, ch, year, sKey, stage, syst, i+1, nJob))
                    dHist["%s_Hist%s%s"%(sKey, stage, syst)] = lHist
                    print(f"{algo}: {ch}: {year}:  {sKey}: {syst}: nJob = {nJob}")
            print(f"\n{algo}: {jec}: {ch}: {year} :  nJobs  = {yJobs}\n")
            allJobs = allJobs + yJobs
            fSkimNew = open(f"json/FilesSkim_{algo}_{jec}_{ch}_{year}.json", "w")
            json.dump(jSkim, fSkimNew, indent=4) 
            json.dump(dHist, fHist, indent=4) 
    print(f"All jobs =  {allJobs}")
    
