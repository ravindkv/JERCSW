import re
import os
import sys
import json
import itertools
from ROOT import TFile
import multiprocessing
sys.dont_write_bytecode = True
sys.path.insert(0, os.getcwd().replace("condor",""))
from Inputs import *
Channels =  list(channelDetails.keys()) 

from createJobFiles import createJobs

#-------------------------------------------------
# Function to check a single ROOT file
#-------------------------------------------------
def check_file(args):
    sKey, skim = args
    f = None
    try:
        # Open the file in a way that ensures it's closed later
        f = TFile.Open(skim, "READ")
        
        # Quick exit if file is inaccessible or corrupted
        if not f or f.IsZombie() or f.GetSize() < 3000:
            print(f"Corrupted or empty file: {skim}")
            return (sKey, skim, True)
        
        # Check for the specific histogram and its validity
        hName = "Base/HistCutflow/h1EventInCutflow"
        h = f.Get(hName)
        if not h:
            print(f"{hName} not found in: {skim}")
            return (sKey, skim, True)

        # All checks passed, file is considered good
        return (sKey, skim, False)
        
    except Exception as e:
        print(f"Exception while opening {skim}: {e}")
        return (sKey, skim, True)
    
    finally:
        if f:  # Ensure file is closed even if an error occurred
            f.Close()

#-------------------------------------------------
# Open each finished file and see if they are OK
#-------------------------------------------------
def checkJobs(jsonFile):
    unfinished = {}
    print("Checking for corrupted files using multiprocessing...")

    # Prepare a list of (sKey, skim) pairs
    file_list = []
    for sKey, skims in jsonFile.items():
        for skim in skims:
            file_list.append((sKey, skim))

    # Use multiprocessing to check files in parallel
    with multiprocessing.Pool(processes=5) as pool:
        results = pool.map(check_file, file_list)

    # Process the results
    for sKey, skim, is_corrupted in results:
        if is_corrupted:
            if sKey not in unfinished:
                unfinished[sKey] = []
            unfinished[sKey].append(skim)

    return unfinished

#-------------------------------------------------
# Main execution block
#-------------------------------------------------
if __name__ == "__main__":
    logDir = "resubLog"
    dResubs = {}
    fResub = "tmpSub/resubFilesHist.json"

    # Ensure the 'tmpSub' directory exists
    os.makedirs("tmpSub", exist_ok=True)

    with open('tmpSub/resubJobs.jdl', 'w') as jdlFile_:
        if os.path.exists(fResub):
            # If resubmission file exists, check those files
            with open(fResub, "r") as fResub_:
                jsonFile = json.load(fResub_)
            dResub = checkJobs(jsonFile)
            dResubs.update(dResub)
            # Write updated list of files to resubmit
            with open(fResub, "w") as fResub__:
                json.dump(dResubs, fResub__, indent=4)
            # Create jobs for resubmission
            with open(fResub, "r") as fResub___:
                createJobs(fResub___, jdlFile_, logDir)
        else:
            # First-time check: iterate over all years and channels
            for stage, algo, jec, year in itertools.product(HistStages, JetAlgos, JecDerLevel.keys(), Years):
                for ch in JecDerLevel[jec]:
                    print(f"\nProcessing, JetAlgo: [{stage}] {algo}, JecDerLevel:{jec} Channel: {ch} Year: {year}")
                    json_file_path = f"../input/json/FilesHist{stage}_{algo}_{jec}_{ch}_{year}.json"
                    with open(json_file_path, "r") as fHist:
                        jsonFile = json.load(fHist)
                    dResub = checkJobs(jsonFile)
                    dResubs.update(dResub)
            # Write the list of files to resubmit
            with open(fResub, "w") as fResub_:
                json.dump(dResubs, fResub_, indent=4)
            # Create jobs for resubmission
            with open(fResub, "r") as fResub__:
                createJobs(fResub__, jdlFile_, logDir)

    # Calculate the total number of files to resubmit
    totResub = sum(len(files) for files in dResubs.values())
    print(f"\n=========> Total files to be resubmitted: {totResub} <==========\n")
    print(f"{fResub}\n")
    
