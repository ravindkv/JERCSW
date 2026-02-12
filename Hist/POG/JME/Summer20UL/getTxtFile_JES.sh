#!/usr/bin/env bash
set -euo pipefail

# Visit:
# https://github.com/cms-jet/JECDatabase/tree/0efe8b54cd4a21619e123d30bae8f9022e0accad

# the commit you pointed at
COMMIT=0efe8b54cd4a21619e123d30bae8f9022e0accad

# base URLs
API_BASE="https://api.github.com/repos/cms-jet/JECDatabase/contents/textFiles"
RAW_BASE="https://raw.githubusercontent.com/cms-jet/JECDatabase/$COMMIT/textFiles"

# list all the directories you want to pull
dirs=(
  Summer20UL16_RunFGH_V2_DATA
  Summer20UL16_V2_MC

  Summer20UL16APV_RunBCDEF_V2_DATA
  Summer20UL16APV_V2_MC

  Summer20UL17_RunB_V2_DATA
  Summer20UL17_RunC_V2_DATA
  Summer20UL17_RunD_V2_DATA
  Summer20UL17_RunE_V2_DATA
  Summer20UL17_RunF_V2_DATA
  Summer20UL17_V2_MC

  Summer20UL18_RunA_V2_DATA
  Summer20UL18_RunB_V2_DATA
  Summer20UL18_RunC_V2_DATA
  Summer20UL18_RunD_V2_DATA
  Summer20UL18_V2_MC
)

# require jq for JSON parsing
command -v jq >/dev/null 2>&1 || {
  echo "This script requires 'jq' (https://stedolan.github.io/jq/). Aborting."
  exit 1
}

for d in "${dirs[@]}"; do
  echo "-> fetching directory: $d"
  mkdir -p "$d"

  # ask GitHub which files are in that sub‐folder
  api_url="$API_BASE/$d?ref=$COMMIT"
  files=$(curl -s "$api_url" \
           | jq -r '.[] | select(.type=="file") | .name')

  # download each one
  for f in $files; do
    raw_url="$RAW_BASE/$d/$f"
    echo "   - $f"
    curl -sL "$raw_url" -o "$d/$f"
  done
done

echo "All done!"

