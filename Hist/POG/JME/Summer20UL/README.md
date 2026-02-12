# Summer20UL JER/JES to JSON

This repository provides a streamlined workflow to download, extract, and convert Jet Energy Resolution (JER) and Jet Energy Scale (JES) text files for the Summer20 Ultra-Legacy (UL) data-taking period into JSON format, using the `jerc2json` tool.

## Table of Contents

* [Setup](#setup)
* [Download and Extract JER Scale Factors](#download-and-extract-jer-scale-factors)
* [Retrieve JER/JES Text Files](#retrieve-jerr-jes-text-files)

  * [JER Text Files](#jer-text-files)
  * [JES Text Files](#jes-text-files)
* [Generate JSON Files](#generate-json-files)
* [Inspect JSON Keys](#inspect-json-keys)
* [Configuration](#configuration)


## Setup

1. **Clone the `jerc2json` repository** (private GitLab):

   ```bash
   git clone ssh://git@gitlab.cern.ch:7999/cms-jetmet/jerc2json.git
   ```
2. **Copy configuration file**

   ```bash
   cd jerc2json
   cp THISDIR/config.yml .
   ```
3. **Create a working directory** and enter it:

   ```bash
   mkdir work && cd work
   ```
4. **Copy the source files**

   ```bash
   cp THISDIR/getTxtFile_*.sh .
   ```

## Download and Extract JER Scale Factors

1. Download the JER SF tarball from the Indico page:

   ```bash
   wget https://indico.cern.ch/event/1290028/contributions/5439963/attachments/2661121/4610042/jersf-20230607.tar.gz
   ```
    In case wget does not work, download mannualy
2. Extract the archive:

   ```bash
   tar -xvf jersf-20230607.tar.gz
   ```
3. You should now see a `plots/` directory containing JER text files.

## Retrieve JER/JES Text Files

### JER Text Files

This step copies additional resolution and algorithm variants (e.g. AK8Puppi) from the Summer19 legacy files.

```bash
source getTxtFile_JER.sh
```

### JES Text Files

The Summer20 JES files are in draft mode. We fetch specific versions from the commit (https://github.com/cms-jet/JECDatabase/tree/0efe8b54cd4a21619e123d30bae8f9022e0accad):

```bash
source getTxtFile_JES.sh
```

Both scripts will place all necessary `.txt` files in your current `work/` directory.

## NOTE: Mannual changes
The eta range was discontinuos so we need to mannually copy-paset from positve (0, 0.621) to negative side (-0.621, 0) in the PtResolution text files: https://mattermost.web.cern.ch/cms-jme-pog/pl/r6f1og9qdtbh8eiuuzjtuqmfte


## Generate JSON Files

1. Return to the root of the `jerc2json` project:

   ```bash
   cd ..
   ```
2. Run the conversion tool for the Summer20 UL 2018 era:

   ```bash
   python3 -m jerc2json create --eras Run2Summer20UL18
   ```
3. The JSON output(s) will be written to the project directory (e.g. `json_files/Run2Summer20UL18/jet_jerc.json`).

## Inspect JSON Keys

Quickly browse the JSON keys to verify contents:

```bash
python printKeys_jercJson.py  jet_jerc.json           # list all keys  with inputs
python printKeys_jercJson.py  jet_jerc.json  edge     # list all keys with inputs and bin edges
```


## NOTE: Mannual changes
1. In the output json files, change fabs to abs 

