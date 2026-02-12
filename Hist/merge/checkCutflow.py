#!/usr/bin/env python3
import json
import sys
from pathlib import Path
import importlib.util
import ROOT


def load_inputs(inputs_path: Path):
    """Load ../Inputs.py as a module without needing PYTHONPATH hacks."""
    spec = importlib.util.spec_from_file_location("Inputs", str(inputs_path))
    if spec is None or spec.loader is None:
        raise RuntimeError(f"Could not load Inputs.py from: {inputs_path}")
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod


def check_cutflow(json_path: Path):
    print("\n" + "#" * 100)
    print(f"# Processing JSON: {json_path}")
    print("#" * 100)

    with open(json_path, "r") as f:
        file_map = json.load(f)

    for sample_name, root_path in file_map.items():
        print("\n" + "=" * 80)
        print(f"Sample: {sample_name}")
        print(f"File  : {root_path}")

        rf = ROOT.TFile.Open(root_path, "READ")
        if not rf or rf.IsZombie():
            print("  [ERROR] Could not open file")
            continue

        h_raw = rf.Get("Base/HistCutflow/h1EventInCutflow")
        h_wgt = rf.Get("Base/HistCutflow/h1EventInCutflowWithWeight")

        if not h_raw:
            print("  [ERROR] Missing Base/HistCutflow/h1EventInCutflow")
            rf.Close()
            continue
        if not h_wgt:
            print("  [ERROR] Missing Base/HistCutflow/h1EventInCutflowWithWeight")
            rf.Close()
            continue

        nbins = h_raw.GetNbinsX()
        if h_wgt.GetNbinsX() != nbins:
            print("  [WARNING] Raw vs weighted histograms have different bin counts")

        print("  Bin  Label                       N_raw         N_weighted      ratio")
        print("  ---- ------------------------ ------------  --------------  ----------")

        for ibin in range(1, nbins + 1):
            n_raw = h_raw.GetBinContent(ibin)
            n_wgt = h_wgt.GetBinContent(ibin)
            label = h_raw.GetXaxis().GetBinLabel(ibin) or f"bin{ibin}"
            ratio = n_wgt / n_raw if n_raw > 0 else 0.0
            print(
                f"  {ibin:4d} {label:24s} "
                f"{n_raw:12.3f}  {n_wgt:14.3f}  {ratio:10.3f}"
            )

        rf.Close()


def iter_jsons_from_inputs(inputs_mod, merged_json_dir: Path):
    jet_algos = getattr(inputs_mod, "JetAlgos", [])
    jec_levels = getattr(inputs_mod, "JecDerLevel", {})
    years = getattr(inputs_mod, "Years", [])

    for jet_algo in jet_algos:
        for jec_level, channels in jec_levels.items():
            for channel in channels:
                for year in years:
                    #name = f"FilesHistMergedJobs_{jet_algo}_{jec_level}_{channel}_{year}.json"
                    name = f"FilesHistMergedEras_{jet_algo}_{jec_level}_{channel}_{year}.json"
                    yield jet_algo, jec_level, channel, year, merged_json_dir / name


if __name__ == "__main__":
    # Defaults designed for your merge/ directory layout:
    # merge/check_cutflow.py and merge/merged_json/ and merge/../Inputs.py
    here = Path(__file__).resolve()
    default_inputs = (here.parent / "../Inputs.py").resolve()
    default_merged_dir = (here.parent / "merged_json").resolve()

    # Optional CLI:
    #   ./check_cutflow.py                (uses defaults)
    #   ./check_cutflow.py ../Inputs.py merged_json
    inputs_path = Path(sys.argv[1]).resolve() if len(sys.argv) >= 2 else default_inputs
    merged_dir = Path(sys.argv[2]).resolve() if len(sys.argv) >= 3 else default_merged_dir

    if not inputs_path.exists():
        print(f"[ERROR] Inputs.py not found: {inputs_path}")
        sys.exit(1)
    if not merged_dir.exists():
        print(f"[ERROR] merged_json dir not found: {merged_dir}")
        sys.exit(1)

    Inputs = load_inputs(inputs_path)

    n_total = 0
    n_found = 0

    for jet_algo, jec_level, channel, year, json_path in iter_jsons_from_inputs(Inputs, merged_dir):
        n_total += 1
        if not json_path.exists():
            print(f"[SKIP] missing JSON: {json_path.name}")
            continue

        n_found += 1
        print("\n" + "=" * 100)
        print(f"==> {jet_algo} | {jec_level} | {channel} | {year}")
        print("=" * 100)
        check_cutflow(json_path)

    print("\n" + "-" * 100)
    print(f"Done. Existing JSONs processed: {n_found}/{n_total}")
    print("-" * 100)

