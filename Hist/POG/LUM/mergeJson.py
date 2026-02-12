import json
import glob
import os

# Directory containing the JSON files
INPUT_DIR = "HLT"
OUTPUT_DIR = "HLT"

os.makedirs(OUTPUT_DIR, exist_ok=True)

# Merge PFJet and Photon JSONs for each year into a single JSON
def merge_for_year(year):
    dipfjet_pattern = os.path.join(INPUT_DIR, f"LumiForTrig_{year}*_HLT_DiPFJet.json")
    ak8jet_pattern = os.path.join(INPUT_DIR, f"LumiForTrig_{year}*_HLT_AK8PFJet.json")
    pfjet_pattern = os.path.join(INPUT_DIR, f"LumiForTrig_{year}*_HLT_PFJet.json")
    photon_pattern = os.path.join(INPUT_DIR, f"LumiForTrig_{year}*_HLT_Photon.json")
    files = glob.glob(ak8jet_pattern) + glob.glob(dipfjet_pattern) + glob.glob(pfjet_pattern) + glob.glob(photon_pattern)

    if not files:
        print(f"No files found for year {year}")
        return

    merged = {}
    for filepath in files:
        with open(filepath, 'r') as f:
            data = json.load(f)
        # Directly merge contents; assume unique keys
        merged.update(data)

    out_path = os.path.join(OUTPUT_DIR, f"LumiForTrig_{year}_HLT_merged.json")
    with open(out_path, 'w') as fout:
        json.dump(merged, fout)
    print(f"Merged JSON written to {out_path}")

if __name__ == "__main__":
    years = ["2016Pre", "2016Post", "2017", "2018"]
    for yr in years:
        merge_for_year(yr)

