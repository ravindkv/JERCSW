#!/usr/bin/env python3
import os
import sys
import json
import argparse
import itertools
import multiprocessing as mp

sys.dont_write_bytecode = True
sys.path.insert(0, os.getcwd().replace("merge",""))

from Inputs import (
    HistStages, JetAlgos, JecDerLevel, Years,
)

from MergeInputs import (
    INPUT_JSON_DIR, INPUT_FILES_HIST_DIR,
    JSON_FILES_HIST_FMT, JSON_MERGEDJOBS_FMT,
    DEFAULT_WORKERS,
)
from MergeUtils import eos_dir_mergedjobs, run_hadd


def merge_one_skey(args):
    """
    Merge one sKey group (list of input ROOT files) into one merged ROOT.
    Returns (sKey, output_path)
    """
    sKey, file_list, out_dir = args
    out_path = os.path.join(out_dir, f"{sKey}_Hist_Merged.root")
    run_hadd(out_path, file_list)
    return sKey, out_path


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--workers", type=int, default=DEFAULT_WORKERS)
    args = parser.parse_args()

    os.makedirs(INPUT_JSON_DIR, exist_ok=True)

    for stage, jet, jec, year in itertools.product(HistStages, JetAlgos, JecDerLevel.keys(), Years):
        for ch in JecDerLevel[jec]:
            print(f"\nProcessing [{stage}] JetAlgo={jet}, JecDerLevel={jec}, Channel={ch}, Year={year}\n")

            hist_json = os.path.join(INPUT_FILES_HIST_DIR, JSON_FILES_HIST_FMT.format(stage=stage, jet=jet, jec=jec, ch=ch, year=year))
            if not os.path.exists(hist_json):
                print(f"[Warning] Missing input JSON: {hist_json}")
                continue

            with open(hist_json, "r") as f:
                jHist = json.load(f)

            out_dir = eos_dir_mergedjobs(stage, jet, jec, ch, year)
            os.makedirs(out_dir, exist_ok=True)

            # tasks: each key merges its list of root files
            tasks = [(sKey, jHist[sKey], out_dir) for sKey in jHist.keys()]

            ctx = mp.get_context("fork" if sys.platform != "darwin" else "spawn")
            with ctx.Pool(processes=args.workers) as pool:
                results = pool.map(merge_one_skey, tasks)

            dMerged = {sKey: out_path for (sKey, out_path) in results}

            out_json_name = JSON_MERGEDJOBS_FMT.format(stage=stage, jet=jet, jec=jec, ch=ch, year=year)
            out_json_path = os.path.join(INPUT_JSON_DIR, out_json_name)
            with open(out_json_path, "w") as f:
                json.dump(dMerged, f, indent=4)

            print(f"[Done] Wrote {out_json_path}")


if __name__ == "__main__":
    main()
