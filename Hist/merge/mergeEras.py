#!/usr/bin/env python3
import os
import sys
import json
import argparse
import multiprocessing as mp

sys.dont_write_bytecode = True
sys.path.insert(0, os.getcwd().replace("merge", ""))

from Inputs import (
    HistStages, JetAlgos, JecDerLevel, Years, 
)
from MergeInputs import (
    INPUT_JSON_DIR,
    JSON_MERGEDJOBS_FMT, JSON_MERGEDEras_FMT,
    KEY_MERGEDEras_FMT,
    DEFAULT_WORKERS,
)
from MergeUtils import parse_job_filename_from_path, eos_dir_mergederas, run_hadd, aggregate_sample

def merge_one_task(args):
    """
    Merge per-job files (from mergeJobs JSON) into per-year/per-sample MergedEras outputs.
    One task = (jet, jec, ch, year).
    """
    stage, jet, jec, ch, year = args

    in_json = os.path.join(INPUT_JSON_DIR, JSON_MERGEDJOBS_FMT.format(stage=stage, jet=jet, jec=jec, ch=ch, year=year))
    if not os.path.exists(in_json):
        return f"[Skip] Missing JSON: {in_json}"

    with open(in_json, "r") as f:
        filemap = json.load(f)

    mc_files = {}    # agg_sample -> [job files]
    data_files = {}  # dataset    -> [job files]

    # JSON value is a ROOT file path: parse it to robustly get category/sample/yearOrEra
    for _, fpath in filemap.items():
        try:
            jet_k, jec_k, ch_k, year_or_era, cat_k, raw_sample = parse_job_filename_from_path(fpath, stage)
        except Exception as e:
            return f"[Warn] {in_json}: failed to parse {fpath} ({e})"

        if jet_k != jet or jec_k != jec or ch_k != ch:
            continue
        if not year_or_era.startswith(year):
            continue

        agg = aggregate_sample(ch, cat_k, raw_sample)

        if cat_k == "MC":
            mc_files.setdefault(agg, []).append(fpath)
        elif cat_k == "Data":
            data_files.setdefault(agg, []).append(fpath)

    out_dir = eos_dir_mergederas(stage, jet, jec, ch, year)
    os.makedirs(out_dir, exist_ok=True)

    merged = {}

    # Merge MC
    for sample, files in mc_files.items():
        if not files:
            continue
        key = KEY_MERGEDEras_FMT.format(stage=stage, jet=jet, jec=jec, ch=ch, cat="MC", year=year, sample=sample)
        out_root = os.path.join(out_dir, f"{key}_Hist_Merged.root")
        run_hadd(out_root, files)
        merged[key] = out_root

    # Merge Data
    for dataset, files in data_files.items():
        if not files:
            continue
        key = KEY_MERGEDEras_FMT.format(stage=stage, jet=jet, jec=jec, ch=ch, cat="Data", year=year, sample=dataset)
        out_root = os.path.join(out_dir, f"{key}_Hist_Merged.root")
        run_hadd(out_root, files)
        merged[key] = out_root

    out_json = os.path.join(INPUT_JSON_DIR, JSON_MERGEDEras_FMT.format(stage=stage, jet=jet, jec=jec, ch=ch, year=year))
    with open(out_json, "w") as f:
        json.dump(merged, f, indent=4)

    return f"[Done] [{stage}] {jet} {jec} {ch} {year} → {out_json}"


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--workers", type=int, default=DEFAULT_WORKERS)
    args = parser.parse_args()

    tasks = []
    for stage in HistStages:
        for jet in JetAlgos:
            for jec in JecDerLevel.keys():
                for ch in JecDerLevel[jec]:
                    for year in Years:
                        tasks.append((stage, jet, jec, ch, year))

    ctx = mp.get_context("fork" if sys.platform != "darwin" else "spawn")
    with ctx.Pool(processes=args.workers) as pool:
        for msg in pool.imap_unordered(merge_one_task, tasks, chunksize=1):
            print(msg)


if __name__ == "__main__":
    main()
