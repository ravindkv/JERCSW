## Run2 Histogram Merging (Jobs → Eras → Years) + Validation

This directory contains scripts to merge per-job ROOT histogram outputs into:
1) per-year merged files (`MergedEras`)
2) full Run2 merged files (`MergedYears/Run2`)
and to validate event-count consistency across stages.

All naming, directory layout, grouping rules, and validation histogram paths are centralized in `Hist/Inputs.py` and `Hist/merge/MergeInputs.py`

## Typical Workflow

1. Run merging:

```bash
python mergeJobs.py  
python mergeEras.py  
python mergeYears.py 
```

2. Quick merge-sanity check (recommended):

```bash
python validateMerge.py 
```

3. Quick cutflow-weight check (recommended):

```bash
python checkCutflow.py
```

