#!/usr/bin/env python3
import os
import sys
import json
import glob
import argparse
import ROOT

ROOT.gROOT.SetBatch(True)

sys.dont_write_bytecode = True
sys.path.insert(0, os.getcwd().replace("merge", ""))

from Inputs import (
    JetAlgos, JecDerLevel, Years,
)
from MergeInputs import (
    INPUT_JSON_DIR,
    JSON_MERGEDJOBS_FMT, JSON_MERGEDEras_FMT, JSON_MERGEDYEARS_FMT,
    VALIDATION_HISTS,
)
from MergeUtils import (
    parse_job_filename_from_path,
    eos_dir_separate,
    aggregate_sample,
)


# ---------------- ROOT helpers ---------------- #

def get_hist_events(file_path, hist_path):
    f = ROOT.TFile.Open(file_path)
    if not f or f.IsZombie():
        print(f"[ERROR] Could not open file: {file_path}")
        return 0.0
    h = f.Get(hist_path)
    if not h:
        print(f"[ERROR] Histogram '{hist_path}' not found in: {file_path}")
        f.Close()
        return 0.0
    val = float(h.Integral())
    f.Close()
    return val


def sum_events(files, hist_path, verbose=False):
    total = 0.0
    for fp in sorted(files):
        ev = get_hist_events(fp, hist_path)
        if verbose:
            print(f"  {fp}  -> {ev}")
        total += ev
    return total


# -------------- discovery from MergedEras -------- #

def discover_combos_from_mergederas(jet=None, jec=None, ch=None):
    combos = set()
    for j in JetAlgos:
        if jet and j != jet:
            continue
        for jl in JecDerLevel.keys():
            if jec and jl != jec:
                continue
            for c in JecDerLevel[jl]:
                if ch and c != ch:
                    continue
                for year in Years:
                    path = os.path.join(INPUT_JSON_DIR, JSON_MERGEDEras_FMT.format(jet=j, jec=jl, ch=c, year=year))
                    if not os.path.exists(path):
                        continue
                    with open(path) as f:
                        filemap = json.load(f)
                    for key in filemap.keys():
                        parts = key.split("_")
                        # <jet>_<jec>_<ch>_<cat>_<year>_<sample...>
                        if len(parts) < 6:
                            continue
                        jet_k, jec_k, ch_k = parts[0], parts[1], parts[2]
                        cat_k, year_k = parts[3], parts[4]
                        sample = "_".join(parts[5:])
                        if jet_k != j or jec_k != jl or ch_k != c:
                            continue
                        if year_k != year:
                            continue
                        combos.add((j, jl, c, cat_k, sample))
    return combos


# ---------------- stage totals ----------------- #

def total_separate(jet, jec, ch, cat, sample, hist, verbose=False):
    total = 0.0
    for year in Years:
        year_dir = eos_dir_separate(jet, jec, ch, year)
        patt = os.path.join(year_dir, f"{jet}_{jec}_{ch}_{year}*_{cat}_*_HistBase_*.root")
        files = glob.glob(patt)

        selected = []
        for fp in files:
            try:
                jet_k, jec_k, ch_k, yoera, cat_k, raw_sample = parse_job_filename_from_path(fp)
            except Exception:
                continue
            if jet_k != jet or jec_k != jec or ch_k != ch:
                continue
            if not yoera.startswith(year):
                continue
            if cat_k != cat:
                continue
            agg = aggregate_sample(ch, cat, raw_sample)
            if agg != sample:
                continue
            selected.append(fp)

        if verbose:
            print(f"[Separate] {year} selected {len(selected)} files")
        total += sum_events(selected, hist, verbose=verbose)
    return total


def total_mergedjobs(jet, jec, ch, cat, sample, hist, verbose=False):
    selected = []
    for year in Years:
        path = os.path.join(INPUT_JSON_DIR, JSON_MERGEDJOBS_FMT.format(jet=jet, jec=jec, ch=ch, year=year))
        if not os.path.exists(path):
            continue
        with open(path) as f:
            filemap = json.load(f)

        for _, fp in filemap.items():
            try:
                jet_k, jec_k, ch_k, yoera, cat_k, raw_sample = parse_job_filename_from_path(fp)
            except Exception:
                continue
            if jet_k != jet or jec_k != jec or ch_k != ch:
                continue
            if not yoera.startswith(year):
                continue
            if cat_k != cat:
                continue
            agg = aggregate_sample(ch, cat, raw_sample)
            if agg != sample:
                continue
            selected.append(fp)

    if verbose:
        print(f"[MergedJobs] selected {len(selected)} files")
    return sum_events(selected, hist, verbose=verbose)


def total_mergederas(jet, jec, ch, cat, sample, hist, verbose=False):
    files = []
    for year in Years:
        path = os.path.join(INPUT_JSON_DIR, JSON_MERGEDEras_FMT.format(jet=jet, jec=jec, ch=ch, year=year))
        if not os.path.exists(path):
            continue
        with open(path) as f:
            filemap = json.load(f)
        for key, fp in filemap.items():
            parts = key.split("_")
            if len(parts) < 6:
                continue
            jet_k, jec_k, ch_k = parts[0], parts[1], parts[2]
            cat_k, year_k = parts[3], parts[4]
            sam_k = "_".join(parts[5:])
            if jet_k != jet or jec_k != jec or ch_k != ch:
                continue
            if cat_k != cat or year_k != year or sam_k != sample:
                continue
            files.append(fp)

    if verbose:
        print(f"[MergedEras] selected {len(files)} files")
    return sum_events(files, hist, verbose=verbose)


def total_mergedyears(jet, jec, ch, cat, sample, hist, verbose=False):
    path = os.path.join(INPUT_JSON_DIR, JSON_MERGEDYEARS_FMT.format(jet=jet, jec=jec, ch=ch))
    if not os.path.exists(path):
        return 0.0

    with open(path) as f:
        filemap = json.load(f)

    files = []
    for key, fp in filemap.items():
        parts = key.split("_")
        # <jet>_<jec>_<ch>_<cat>_Run2_<sample...>
        if len(parts) < 6:
            continue
        jet_k, jec_k, ch_k = parts[0], parts[1], parts[2]
        cat_k, run2_k = parts[3], parts[4]
        sam_k = "_".join(parts[5:])
        if jet_k != jet or jec_k != jec or ch_k != ch:
            continue
        if cat_k != cat or run2_k != "Run2" or sam_k != sample:
            continue
        files.append(fp)

    if verbose:
        print(f"[MergedYears] selected {len(files)} files")
    return sum_events(files, hist, verbose=verbose)


# ---------------- validation ---------------- #

def validate_combo(jet, jec, ch, cat, sample, hist, tol=0.0, verbose=False):
    print("\n" + "=" * 50)
    print(f"{jet} {jec} {ch} {cat} {sample}")
    print("=" * 100)

    a = total_separate(jet, jec, ch, cat, sample, hist, verbose=verbose)
    b = total_mergedjobs(jet, jec, ch, cat, sample, hist, verbose=verbose)
    c = total_mergederas(jet, jec, ch, cat, sample, hist, verbose=verbose)
    d = total_mergedyears(jet, jec, ch, cat, sample, hist, verbose=verbose)

    res = {
        "Separate"   : a,
        "MergedJobs" : b,
        "MergedEras" : c,
        "MergedYears": d,
    }

    for k, v in res.items():
        if abs(v - round(v)) < 1e-6:
            print(f"{k:12s}: {int(round(v))}")
        else:
            print(f"{k:12s}: {v:.6f}")

    vals = list(res.values())
    if all(v == 0 for v in vals):
        print("[INFO] all zero, skipping")
        return True

    ref = vals[0]
    ok = all(abs(v - ref) <= tol for v in vals[1:])
    print("[PASS]" if ok else "[FAIL]")
    return ok


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--hist", default=VALIDATION_HISTS.get("default"))
    parser.add_argument("--jet", default=None)
    parser.add_argument("--jec", default=None)
    parser.add_argument("--channel", default=None)
    parser.add_argument("--tol", type=float, default=0.0)
    parser.add_argument("--verbose", action="store_true")
    args = parser.parse_args()

    combos = discover_combos_from_mergederas(jet=args.jet, jec=args.jec, ch=args.channel)
    if not combos:
        print("[INFO] No combos found from MergedEras JSONs.")
        return

    nfail = 0
    for jet, jec, ch, cat, sample in sorted(combos):
        ok = validate_combo(jet, jec, ch, cat, sample, args.hist, tol=args.tol, verbose=args.verbose)
        if not ok:
            nfail += 1

    print("\n" + "#" * 80)
    if nfail == 0:
        print("[FINAL] All validations PASSED.")
        sys.exit(0)
    print(f"[FINAL] {nfail} validations FAILED.")
    sys.exit(2)


if __name__ == "__main__":
    main()

