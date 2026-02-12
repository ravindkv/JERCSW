#!/usr/bin/env python3
import json
import glob
import os
import sys

# --- CONFIGURE THESE PATHS AS NEEDED ---
HLT_JSON   = "../../config/HltDiJetAK8Puppi.json"
LUMI_GLOB  = "./HLT/LumiForTrig_{year}*_HLT_merged_lumi.json"
# ---------------------------------------

def load_hlt():
    try:
        with open(HLT_JSON, "r") as f:
            return json.load(f)
    except Exception as e:
        print(f"ERROR: cannot read {HLT_JSON}: {e}", file=sys.stderr)
        sys.exit(1)

def save_hlt(hlt):
    years = list(hlt.keys())
    with open(HLT_JSON, "w") as f:
        f.write("{\n")
        for yi, year in enumerate(years):
            triggers = hlt[year]
            f.write(f'  "{year}": {{\n')
            names = list(triggers.keys())
            for ti, name in enumerate(names):
                cfg = triggers[name]
                # build the inner { key: val, ... } with insertion order preserved
                inner_parts = []
                for k, v in cfg.items():
                    # use repr() so floats look like "0.0" etc.
                    inner_parts.append(f'"{k}": {repr(v)}')
                inner = ", ".join(inner_parts)
                comma = "," if ti < len(names)-1 else ""
                f.write(f'    "{name}": {{ {inner} }}{comma}\n')
            closing = "  }," if yi < len(years)-1 else "  }"
            f.write(f"{closing}\n")
        f.write("}\n")
    print(f"✔ Updated {HLT_JSON} with one-line trigger blocks")

def main():
    hlt = load_hlt()

    for year, triggers in hlt.items():
        # pick up matching lumi file (2016Pre vs 2016Post, etc.)
        pattern = LUMI_GLOB.format(year=year)
        files = glob.glob(pattern)
        if not files:
            print(f"[WARN] no lumi file for year '{year}' (tried '{pattern}')", file=sys.stderr)
            for cfg in triggers.values():
                cfg["lumi"] = 0.0
            continue

        lumi_file = files[0]
        try:
            with open(lumi_file, "r") as f:
                lumi_map = json.load(f)
        except Exception as e:
            print(f"[WARN] cannot read {lumi_file}: {e}", file=sys.stderr)
            for cfg in triggers.values():
                cfg["lumi"] = 0.0
            continue

        for name, cfg in triggers.items():
            lumi_val = lumi_map.get(name, 0.0)
            if name not in lumi_map:
                print(f"[WARN] '{name}' not in {os.path.basename(lumi_file)}; setting lumi=0.0",
                      file=sys.stderr)
            cfg["lumi"] = round(lumi_val,4)

    save_hlt(hlt)

if __name__ == "__main__":
    main()

