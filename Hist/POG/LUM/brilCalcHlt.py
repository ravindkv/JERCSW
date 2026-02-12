

# https://twiki.cern.ch/twiki/bin/viewauth/CMS/BrilcalcQuickStart
# source /cvmfs/cms-bril.cern.ch/cms-lumi-pog/brilws-docker/brilws-env
# brilcalc lumi --normtag  -i lumiGolden.json --hltpath "HLT_Photon*" -o output.csv

lumiJsons = {}
lumiJsons["2016Pre"] = "Golden/Cert_271036-284044_13TeV_Legacy2016_Collisions16_JSON.txt"
lumiJsons["2016Post"]= "Golden/Cert_271036-284044_13TeV_Legacy2016_Collisions16_JSON.txt"
lumiJsons["2017"]    = "Golden/Cert_294927-306462_13TeV_UL2017_Collisions17_GoldenJSON.txt"
lumiJsons["2018"]    = "Golden/Cert_314472-325175_13TeV_Legacy2018_Collisions18_JSON.txt"


normTagJson = "/cvmfs/cms-bril.cern.ch/cms-lumi-pog/Normtags/normtag_PHYSICS.json"
hlts = ["HLT_AK8PFJet"]#, "HLT_Photon", "HLT_PFJet"]

cmd = ""
for year, lumiJson in lumiJsons.items():
    for hlt in hlts:
        cmd += f"brilcalc lumi --precision 3f --normtag {normTagJson} -i {lumiJson} --hltpath \"{hlt}*\" -o HLT/LumiForTrig_{year}_{hlt}.csv &&"

print(cmd)

