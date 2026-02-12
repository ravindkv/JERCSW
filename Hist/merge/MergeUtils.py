# MergeUtils.py
import os
import sys
import subprocess

sys.dont_write_bytecode = True
sys.path.insert(0, os.getcwd().replace("merge",""))
from Inputs import eosHistDir

from MergeInputs import (
    FILENAME_JOB_TOKEN,
    group_mc_sample,
    EOS_DIR_SEPARATE_FMT,
    EOS_DIR_MERGEDJOBS_FMT,
    EOS_DIR_MERGEDEras_FMT,
    EOS_DIR_MERGEDYEARS_FMT,
)

def parse_job_filename_from_path(file_path, stage):
    """
    Parse from:
      <JetAlgo>_<JecDerLevel>_<Channel>_<YearOrEra>_<Category>_<Sample>_HistBase_<job>.root
    Returns: (jet, jec, ch, yearOrEra, cat, sample)
    """
    fname = os.path.basename(file_path)
    if fname.endswith(".root"):
        fname = fname[:-5]

    parts = fname.split("_")
    if len(parts) < 7:
        raise ValueError(f"Unexpected job ROOT filename: {fname}")

    jet = parts[0]
    jec = parts[1]
    ch = parts[2]
    yearOrEra = parts[3]
    cat = parts[4]

    try:
        idx = parts.index(FILENAME_JOB_TOKEN.format(stage=stage))
    except ValueError:
        idx = len(parts)

    sample = "_".join(parts[5:idx]) if idx > 5 else ""
    return jet, jec, ch, yearOrEra, cat, sample


def eos_dir_separate(stage, jet, jec, ch, year):
    return EOS_DIR_SEPARATE_FMT.format(base=eosHistDir, stage=stage, jet=jet, jec=jec, ch=ch, year=year)

def eos_dir_mergedjobs(stage, jet, jec, ch, year):
    return EOS_DIR_MERGEDJOBS_FMT.format(base=eosHistDir, stage=stage, jet=jet, jec=jec, ch=ch, year=year)

def eos_dir_mergederas(stage, jet, jec, ch, year):
    return EOS_DIR_MERGEDEras_FMT.format(base=eosHistDir, stage=stage, jet=jet, jec=jec, ch=ch, year=year)

def eos_dir_mergedyears(stage, jet, jec, ch):
    return EOS_DIR_MERGEDYEARS_FMT.format(base=eosHistDir, stage=stage, jet=jet, jec=jec, ch=ch)


def _eos_to_xrd(path: str) -> str:
    """
    Convert a local EOS path to an absolute XRootD URL.

    /eos/cms/...  ->  root://eoscms.cern.ch//eos/cms/...
    root://...    ->  unchanged
    other local   ->  unchanged
    """
    if path.startswith("root://"):
        return path
    if path.startswith("/eos/"):
        # IMPORTANT: double slash after host, and absolute /eos/... path
        return "root://eoscms.cern.ch//" + path.lstrip("/")
    return path


def run_hadd(output_file, input_files):
    """
    Robust hadd wrapper:
      - ensures EOS output directory exists (using eos mkdir -p)
      - forces absolute XRootD URLs for EOS paths to avoid "relative path is disallowed"
    """
    if not input_files:
        raise ValueError("run_hadd: empty input_files")

    outdir = os.path.dirname(output_file)
    if outdir:
        if outdir.startswith("/eos/"):
            subprocess.run(["eos", "mkdir", "-p", outdir], check=True)
        else:
            os.makedirs(outdir, exist_ok=True)

    out_arg = _eos_to_xrd(output_file)
    in_args = [_eos_to_xrd(p) for p in input_files]

    cmd = ["hadd", "-f", "-v", "0", "-k", out_arg] + in_args
    subprocess.run(cmd, check=True)


def aggregate_sample(channel, category, raw_sample):
    """
    Apply grouping rules only for MC. Data should remain dataset name.
    """
    if category == "MC":
        return group_mc_sample(channel, raw_sample)
    return raw_sample

