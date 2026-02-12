# Hist Package

## Table of Contents
- [Introduction](#introduction)
- [Installation](#installation)
  - [1. Install CorrectionLib](#1-install-correctionlib)
  - [2. Set Environment Variables](#2-set-environment-variables)
- [Preparing Input Files](#preparing-input-files)
  - [1. Get List of Skim Files](#1-get-list-of-skim-files)
- [Compiling the Code](#compiling-the-code)
- [Running the Code Locally](#running-the-code-locally)
  - [1. Display Help Message](#1-display-help-message)
- [Submitting Condor Jobs](#submitting-condor-jobs)
- [Merge Condor Jobs](#merge-jobs]
- [Contact](#contact)

## Introduction

The **Hist** package is designed to produce histogram files from skimmed NanoAOD trees. It utilizes a set of common classes and histogramming classes to process data and generate the desired histograms. The package supports both local execution and submission of jobs to a Condor batch system for efficient processing of large datasets.

## Installation

---
### 1. Install CorrectionLib

The Hist package depends on the **correctionlib** library. Follow these steps to install it:

```bash
cd Hist

# Clone the correctionlib repository into the 'corrlib' directory
git clone --recursive git@github.com:cms-nanoAOD/correctionlib.git corrlib

# Navigate to the corrlib directory
cd corrlib

# Build and install correctionlib
make
make install

# Return to the Hist package root directory
cd ..
```

### 2. Set Environment Variables

After installing CorrectionLib, update your `LD_LIBRARY_PATH` to include the CorrectionLib library:

```bash
export LD_LIBRARY_PATH=$(pwd)/corrlib/lib:$LD_LIBRARY_PATH
```

To make this change permanent, add the above line to your `~/.bashrc` file:

```bash
echo 'export LD_LIBRARY_PATH=$(pwd)/corrlib/lib:$LD_LIBRARY_PATH' >> ~/.bashrc

# Reload your bash configuration
source ~/.bashrc
```
---

## Preparing Input Files

In the Inputs.py, change the values as needed

### 1. Get List of Skim Files

The Hist package requires input files containing skimmed NanoAOD data. To generate the list of input files:

```bash
cd input
python3 getRootFiles.py 
```

This script will produce files containing paths to the skimmed NanoAOD trees. Review the generated files to ensure they are correct.
---


## Compiling the Code

To compile the code, run:

```bash
cd ..
make clean
make -j4
```

This will compile all the necessary C++ files and create object files in the `obj` directory. The main executable `runMain` will be created in the root directory.

---
## Running the Code Locally

To produce histogram files from the skimmed data locally, use the `runMain` executable.

### 1. Display Help Message

To see the available options:

```bash
./runMain -h
```
This will display all the commands and options available for running the code.

---
## Submitting Condor Jobs

Go to the Hist/condor directory and follow the README
---

## Merge Condor Jobs

Go to the Hist/merge directory and follow the README
---

## Contact

For any questions or issues, please contact [Ravindra Verma](mailto:rverma@cern.ch).

