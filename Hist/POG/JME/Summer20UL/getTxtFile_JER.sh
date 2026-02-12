#!/usr/bin/env bash
set -euo pipefail


# Summer20UL JER SF from https://indico.cern.ch/event/1290028/#4-jer-sf-with-zerobias-for-ul
# ——— preflight check ———
if [[ ! -d "./plots" ]]; then
  echo "First have the plots dir by copying and un‑tarring (tar -xvf) this file https://indico.cern.ch/event/1290028/contributions/5439963/attachments/2661121/4610042/jersf-20230607.tar.gz" >&2
  exit 1
fi

# Then JRDatabase
API_BASE="https://api.github.com/repos/cms-jet/JRDatabase/contents/textFiles"
RAW_BASE="https://raw.githubusercontent.com/cms-jet/JRDatabase/refs/heads/master/textFiles"

# list all the directories you want to pull
dirs=(
    Summer20UL16APV_JRV4_MC
    Summer20UL16_JRV4_MC
    Summer20UL17_JRV1_MC
    Summer20UL18_JRV1_MC

    # for copying some content from these
    Summer20UL16APV_JRV3_MC
    Summer20UL16_JRV3_MC
    Summer19UL17_JRV3_MC
    Summer19UL18_JRV2_MC 

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
  api_url="$API_BASE/$d"
  files=$(curl -s "$api_url" \
           | jq -r '.[] | select(.type=="file") | .name')

  # download each one
  for f in $files; do
    raw_url="$RAW_BASE/$d/$f"
    echo " $raw_url"
    echo "   - $f"
    curl -sL "$raw_url" -o "$d/$f"
  done
done


#---------------------------------------------
# Manually Copy files 
#---------------------------------------------
echo "Manually copying and overriding ..."
# from https://indico.cern.ch/event/1290028/#4-jer-sf-with-zerobias-for-ul 
cp plots/Summer20UL2016APV_ZB_v26c_JRV3_MC_SF_AK4PFchs.txt Summer20UL16APV_JRV4_MC/Summer20UL16APV_JRV4_MC_SF_AK4PFchs.txt

# from JECDatabase
cp Summer20UL16APV_JRV3_MC/Summer20UL16APV_JRV3_MC_SF_AK4PFPuppi.txt Summer20UL16APV_JRV4_MC/Summer20UL16APV_JRV4_MC_SF_AK4PFPuppi.txt
cp Summer20UL16APV_JRV3_MC/Summer20UL16APV_JRV3_MC_SF_AK8PFchs.txt Summer20UL16APV_JRV4_MC/Summer20UL16APV_JRV4_MC_SF_AK8PFchs.txt
cp Summer20UL16APV_JRV3_MC/Summer20UL16APV_JRV3_MC_SF_AK8PFPuppi.txt Summer20UL16APV_JRV4_MC/Summer20UL16APV_JRV4_MC_SF_AK8PFPuppi.txt

cp plots/Summer20UL2016GH_ZB_v26c_JRV3_MC_SF_AK4PFchs.txt Summer20UL16_JRV4_MC/Summer20UL16_JRV4_MC_SF_AK4PFchs.txt
cp Summer20UL16_JRV3_MC/Summer20UL16_JRV3_MC_SF_AK4PFPuppi.txt Summer20UL16_JRV4_MC/Summer20UL16_JRV4_MC_SF_AK4PFPuppi.txt
cp Summer20UL16_JRV3_MC/Summer20UL16_JRV3_MC_SF_AK8PFchs.txt Summer20UL16_JRV4_MC/Summer20UL16_JRV4_MC_SF_AK8PFchs.txt
cp Summer20UL16_JRV3_MC/Summer20UL16_JRV3_MC_SF_AK8PFPuppi.txt Summer20UL16_JRV4_MC/Summer20UL16_JRV4_MC_SF_AK8PFPuppi.txt

cp plots/Summer20UL2017_ZB_v26c_JRV3_MC_SF_AK4PFchs.txt Summer20UL17_JRV1_MC/Summer20UL17_JRV1_MC_SF_AK4PFchs.txt
cp Summer19UL17_JRV3_MC/Summer19UL17_JRV3_MC_SF_AK4PFPuppi.txt Summer20UL17_JRV1_MC/Summer20UL17_JRV1_MC_SF_AK4PFPuppi.txt
cp Summer19UL17_JRV3_MC/Summer19UL17_JRV3_MC_SF_AK8PFchs.txt Summer20UL17_JRV1_MC/Summer20UL17_JRV1_MC_SF_AK8PFchs.txt
cp Summer19UL17_JRV3_MC/Summer19UL17_JRV3_MC_SF_AK8PFPuppi.txt Summer20UL17_JRV1_MC/Summer20UL17_JRV1_MC_SF_AK8PFPuppi.txt

cp plots/Summer20UL2018_ZB_v26c_JRV3_MC_SF_AK4PFchs.txt Summer20UL18_JRV1_MC/Summer20UL18_JRV1_MC_SF_AK4PFchs.txt
cp Summer19UL18_JRV2_MC/Summer19UL18_JRV2_MC_SF_AK4PFPuppi.txt Summer20UL18_JRV1_MC/Summer20UL18_JRV1_MC_SF_AK4PFPuppi.txt
cp Summer19UL18_JRV2_MC/Summer19UL18_JRV2_MC_SF_AK8PFchs.txt Summer20UL18_JRV1_MC/Summer20UL18_JRV1_MC_SF_AK8PFchs.txt
cp Summer19UL18_JRV2_MC/Summer19UL18_JRV2_MC_SF_AK8PFPuppi.txt Summer20UL18_JRV1_MC/Summer20UL18_JRV1_MC_SF_AK8PFPuppi.txt


echo "All done!"



