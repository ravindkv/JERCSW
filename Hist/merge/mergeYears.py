#!/usr/bin/env python3
import os
import sys
import json
import argparse
import multiprocessing as mp

sys.dont_write_bytecode = True
sys.path.insert(0, os.getcwd().replace("merge", ""))

from Inputs import (
    JetAlgos, JecDerLevel, Years,
)
from MergeInputs import (
    INPUT_JSON_DIR,
    JSON_MERGEDEras_FMT, JSON_MERGEDYEARS_FMT,
    KEY_MERGEDYEARS_FMT,
    DEFAULT_WORKERS,
)
from MergeUtils import run_hadd, eos_dir_mergedyears


def merge_one_task(args):
    """
    Merge yearly MergedEras outputs into a single Run2 file per sample.
    One task = (jet, jec, ch).
    """
    stage, jet, jec, ch = args

    mc_all = {}    # sample -> [year roots]
    data_all = {}  # dataset -> [year roots]

    for year in Years:
        in_json = os.path.join(INPUT_JSON_DIR, JSON_MERGEDEras_FMT.format(stage=stage, jet=jet, jec=jec, ch=ch, year=year))
        if not os.path.exists(in_json):
            continue

        with open(in_json, "r") as f:
            filemap = json.load(f)

        for key, fpath in filemap.items():
            parts = key.split("_")
            # Expected:
            # <jet>_<jec>_<ch>_<cat>_<year>_<sample...>
            if len(parts) < 6:
                print(f"[Warn] Bad key in {in_json}: {key}")
                continue

            jet_k, jec_k, ch_k = parts[0], parts[1], parts[2]
            cat_k, year_k = parts[3], parts[4]
            sample = "_".join(parts[5:])

            if jet_k != jet or jec_k != jec or ch_k != ch:
                continue
            if year_k != year:
                continue

            if cat_k == "MC":
                mc_all.setdefault(sample, []).append(fpath)
            elif cat_k == "Data":
                data_all.setdefault(sample, []).append(fpath)

    if not mc_all and not data_all:
        return f"[Skip] No inputs for {jet} {jec} {ch}"

    out_dir = eos_dir_mergedyears(stage, jet, jec, ch)
    os.makedirs(out_dir, exist_ok=True)

    merged = {}

    for sample, files in mc_all.items():
        if not files:
            continue
        key = KEY_MERGEDYEARS_FMT.format(stage=stage, jet=jet, jec=jec, ch=ch, cat="MC", sample=sample)
        out_root = os.path.join(out_dir, f"{key}_Hist_Merged.root")
        run_hadd(out_root, files)
        merged[key] = out_root

    for dataset, files in data_all.items():
        if not files:
            continue
        key = KEY_MERGEDYEARS_FMT.format(stage=stage, jet=jet, jec=jec, ch=ch, cat="Data", sample=dataset)
        out_root = os.path.join(out_dir, f"{key}_Hist_Merged.root")
        run_hadd(out_root, files)
        merged[key] = out_root

    out_json = os.path.join(INPUT_JSON_DIR, JSON_MERGEDYEARS_FMT.format(stage=stage, jet=jet, jec=jec, ch=ch))
    with open(out_json, "w") as f:
        json.dump(merged, f, indent=4)

    return f"[Done] {jet} {jec} {ch} → {out_json}"


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--workers", type=int, default=DEFAULT_WORKERS)
    args = parser.parse_args()

    tasks = []
    for jet in JetAlgos:
        for jec in JecDerLevel.keys():
            for ch in JecDerLevel[jec]:
                tasks.append((jet, jec, ch))

    ctx = mp.get_context("fork" if sys.platform != "darwin" else "spawn")
    with ctx.Pool(processes=args.workers) as pool:
        for msg in pool.imap_unordered(merge_one_task, tasks, chunksize=1):
            print(msg)


if __name__ == "__main__":
    main()

