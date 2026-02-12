
# -----------------------------------------------------------------
# Merge/validation conventions (Single Source of Truth)
# -----------------------------------------------------------------

INPUT_JSON_DIR = "merged_json"
INPUT_FILES_HIST_DIR = "../input/json"  # where FilesHist_*.json live

CATEGORIES = ("MC", "Data")
RUN2_TAG = "Run2"

# ---- JSON filenames ----
JSON_FILES_HIST_FMT = "FilesHist{stage}_{jet}_{jec}_{ch}_{year}.json"

# Outputs from merge steps (DO include jecDerLevel)
JSON_MERGEDJOBS_FMT  = "FilesHist{stage}MergedJobs_{jet}_{jec}_{ch}_{year}.json"
JSON_MERGEDEras_FMT  = "FilesHist{stage}MergedEras_{jet}_{jec}_{ch}_{year}.json"
JSON_MERGEDYEARS_FMT = "FilesHist{stage}MergedYears_{jet}_{jec}_{ch}_Run2.json"

# ---- JSON keys (inside JSON dicts) ----
KEY_MERGEDEras_FMT   = "{jet}_{jec}_{ch}_{cat}_{year}_{sample}"
KEY_MERGEDYEARS_FMT  = "{jet}_{jec}_{ch}_{cat}_Run2_{sample}"

# ---- ROOT produced file naming (per-job file pattern) ----
# <JetAlgo>_<JecDerLevel>_<Channel>_<YearOrEra>_<Category>_<Sample>_HistBase_<job>.root
FILENAME_JOB_TOKEN = "Hist{stage}Base"

# ---- EOS directory layout (recommended and used by mergeEras/Years/validate we wrote) ----
# Separate job outputs are expected under:
#   {eosHistDir}{stage}/{jet}/{jec}/{ch}/{year}/
EOS_DIR_SEPARATE_FMT    = "{base}{stage}/{jet}/{jec}/{ch}/{year}"

# MergedJobs outputs:
EOS_DIR_MERGEDJOBS_FMT  = "{base}{stage}/{jet}/{jec}/{ch}/MergedJobs/{year}"

# MergedEras outputs:
EOS_DIR_MERGEDEras_FMT  = "{base}{stage}/{jet}/{jec}/{ch}/MergedEras/{year}"

# MergedYears outputs:
EOS_DIR_MERGEDYEARS_FMT = "{base}{stage}/{jet}/{jec}/{ch}/MergedYears/Run2"

# ---- Grouping rules (keep merge & validate consistent) ----
MC_GROUPING_RULES = {
    "GamJet":     [("GJetsHT", "GJets"), ("QCDHT", "QCD")],
    "GamJetFake": [("QCDHT", "QCD")],
    "DiJet":   [("QCDHT", "QCD")],
    "MultiJet":   [("QCDHT", "QCD")],
}

def group_mc_sample(channel, raw_sample):
    for prefix, grouped in MC_GROUPING_RULES.get(channel, []):
        if raw_sample.startswith(prefix):
            return grouped
    return raw_sample

# ---- Validation histogram defaults ----
VALIDATION_HISTS = {
    "default": "Base/HistCutflow/h1EventInCutflow",
    # Optional overrides per channel (if ever needed):
    # "MultiJet": "Base/.../h1EventInRefPt",
}

# ---- Parallelism defaults ----
# cap this on lxplus — enough to speed up, not enough to melt
DEFAULT_WORKERS = 4


