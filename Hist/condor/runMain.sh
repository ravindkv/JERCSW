#!/bin/bash
#To be run on remote machine
#Take input arguments as an array
myArray=( "$@" )
#Array: Size=$#, an element=$1, all element = $@

printf "Start histograming at ";/bin/date
printf "Worker node hostname ";/bin/hostname

if [ -z ${_CONDOR_SCRATCH_DIR} ] ; then 
    echo "Running Interactively" ; 
else
    echo "Running In Batch"
    echo ${_CONDOR_SCRATCH_DIR}
	tar -zxf Hist.tar.gz
    cp libcorrectionlib.so Hist/
    cd Hist
    #make clean
    #make -j 4
fi

#Run for Base, Signal region
echo "All arguements: "$@
echo "Number of arguements: "$#
oName=$1
outDir=$2

#for correctionlib
export LD_LIBRARY_PATH=$(pwd):$LD_LIBRARY_PATH

echo "./runMain oName"
./runMain ${oName}

printf "Done histograming at ";/bin/date
#---------------------------------------------
#Copy the ouput root files
#---------------------------------------------
if [ -z ${_CONDOR_SCRATCH_DIR} ] ; then
    echo "Running Interactively" ;
else
    xrdcp -f output/${oName} ${outDir}
    echo "Cleanup"
    cd ..
    rm -rf Hist
fi
printf "Done ";/bin/date
