import os
import sys
import json
import itertools

sys.dont_write_bytecode = True
sys.path.insert(0, os.getcwd().replace("condor", ""))

from Inputs import *
Channels = list(channelDetails.keys())

def createJobs(jsonFile, jdlFile, logDir="log"):
    os.system(f"mkdir -p tmpSub/{logDir}")

    common_command = (
        "Universe   = vanilla\n"
        "should_transfer_files = YES\n"
        "when_to_transfer_output = ON_EXIT\n"
        "Transfer_Input_Files = Hist.tar.gz, runMain.sh, libcorrectionlib.so\n"
        f"x509userproxy        = {vomsProxy}\n"
        "+MaxRuntime = 60*60*24\n"
        "max_retries = 2\n"
        f"Output = {logDir}/log_$(cluster)_$(process).stdout\n"
        f"Log    = {logDir}/log_$(cluster)_$(process).log\n"
        f"Error  = {logDir}/log_$(cluster)_$(process).stderr\n\n"
    )
    # ---------------------------------------------
    # Create jdl (job description language) files
    # ---------------------------------------------
    data = json.load(jsonFile)

    jdlFile.write("Executable = runMain.sh\n")
    jdlFile.write(common_command)

    # One Arguments line using per-job macros
    jdlFile.write("Arguments = $(infile) $(outdir)\n\n")

    # Collect all (infile, outdir) pairs
    pairs = []
    for sKey, hists in data.items():
        for hist in hists:
            outDir  = hist.split(sKey)[0]
            restStr = hist.split(sKey)[1]
            oName   = f"{sKey}{restStr}"
            pairs.append((oName, outDir))

    # Single queue statement with inline "from ( ... )"
    jdlFile.write("queue infile, outdir from (\n")
    for infile, outdir in pairs:
        # If you ever expect spaces in filenames/paths, switch to quoted fields
        jdlFile.write(f"  {infile} {outdir}\n")
    jdlFile.write(")\n")

    jdlFile.close()

def ask_yes_no(msg):
    reply = input(f"{msg} [y/N]: ").strip().lower()
    return reply in ("y", "yes")

if __name__ == "__main__":
    # --------------------------------------------------
    # Local tmpSub deletion
    # --------------------------------------------------
    if os.path.exists("tmpSub"):
        if ask_yes_no("Directory 'tmpSub' already exists. Delete it?"):
            os.system("rm -r tmpSub")
            print("Deleted dir: tmpSub")
        else:
            print("Aborted by user.")
            sys.exit(0)

    os.system("mkdir -p tmpSub")
    tarFile = "tmpSub/Hist.tar.gz"

    print("Tarring... this should take a few seconds")
    # removed -v to avoid printing filenames
    os.system(
        "tar --exclude condor --exclude corrlib/.git --exclude tmp --exclude output "
        "-zcf %s ../../Hist" % tarFile
    )

    os.system("cp runMain.sh tmpSub/")
    libPath = "./../corrlib/lib/libcorrectionlib.so"
    os.system(f"ls {libPath}")
    os.system(f"cp {libPath} tmpSub/")
    os.system("cp /tmp/%s tmpSub" % vomsProxy)

    print("Created dir: tmpSub")

    submitAll = open("tmpSub/submitAll.sh", "w")
    # --------------------------------------------------
    # Ask ONCE before deleting EOS output directories
    # --------------------------------------------------
    confirm_eos_delete = ask_yes_no(
        "WARNING: This will DELETE existing EOS output directories. Continue?"
    )

    for stage, algo, jec, year in itertools.product(HistStages, JetAlgos, JecDerLevel.keys(), Years):
        for ch in JecDerLevel[jec]:
            outPath = f"{eosHistDir}{stage}/{algo}/{jec}/{ch}/{year}"
            if confirm_eos_delete:
                print(f"[Deleting] Output dir: {outPath}")
                os.system(f"rm -r {outPath}")
            else:
                print(f"[Skip delete] {outPath}")

            os.system(f"mkdir -p {outPath}")
            print(f" [Created] Output dir: {outPath}")

            jsonFile = open(f"../input/json/FilesHist{stage}_{algo}_{jec}_{ch}_{year}.json", "r")
            jdlName  = f"submitJobs{stage}_{algo}_{jec}_{ch}_{year}.jdl"
            jdlFile  = open(f"tmpSub/{jdlName}", "w")

            createJobs(jsonFile, jdlFile, logDir="log")
            submitAll.write(f"condor_submit {jdlName}\n")

