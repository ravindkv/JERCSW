
* step-1: brilCalcHlt.py : which coomputes the lumi for every run in csv format

* step-2: csvToJson.py : covert the csv to json format

* step-3: mergeJson.py : for a given year, merge jsons (e.g. from photon and jet HLTs)

* step-4: sumHltLumi.py: So far the json will have per Run lumi. Sum lumi for whole year and convert into 1/fb unit

* step-5: updateHltConfig.py: finally, insert the lumi into Hist/config/Hlt.json 

The output from step-5 is used in the Hist package (ScaleEvent.cpp code)
