#!/usr/bin/env python3
import json
import sys
import os
import re

"""
This script sums recorded lumi for every HLT path base in each merged JSON
and writes a separate output JSON for each input file in the specified output directory.

Usage:
    python sum_lumi_per_file.py <merged_dir> <output_dir>
"""

def sum_lumi_all(data):
    """
    Given merged JSON data (dict), sum recorded lumi for every HLT path base
    and return a mapping of base name to total recorded lumi.
    """
    totals = {}
    pattern = re.compile(r"^(?P<base>.+)_v\d+$")

    for full_path, runs in data.items():
        m = pattern.match(full_path)
        if not m:
            continue
        base = m.group('base')
        totals.setdefault(base, 0.0)
        for lumi_list in runs.values():
            for entry in lumi_list:
                totals[base] += 1e-9*entry.get('recorded', 0)#convert into 1/fb
    return totals


def main():
    if len(sys.argv) != 3:
        print(f"Usage: {sys.argv[0]} <merged_dir> <output_dir>")
        sys.exit(1)

    merged_dir = sys.argv[1]
    output_dir = sys.argv[2]

    if not os.path.isdir(merged_dir):
        print(f"Error: Merged directory not found: {merged_dir}")
        sys.exit(2)

    # Create output directory if needed
    os.makedirs(output_dir, exist_ok=True)

    for fname in os.listdir(merged_dir):
        if not fname.endswith('merged.json'):
            continue
        input_path = os.path.join(merged_dir, fname)
        try:
            with open(input_path, 'r') as f:
                data = json.load(f)
            totals = sum_lumi_all(data)

            # Build output filename: drop .json, append _lumi.json
            base_name = os.path.splitext(fname)[0]
            out_name = f"{base_name}_lumi.json"
            out_path = os.path.join(output_dir, out_name)

            with open(out_path, 'w') as out:
                json.dump(totals, out, indent=2)
            print(f"Wrote {out_name} with {len(totals)} paths")
        except Exception as e:
            print(f"Warning: failed to process {fname}: {e}")

if __name__ == '__main__':
    main()

