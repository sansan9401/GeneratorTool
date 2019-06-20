#!/bin/bash

if [ -z "$1" -o -z "$2" ]
then
    echo "usage: $0 PROCESSNAME NEVENT [NCORE]"
    echo "example: $0 ttH 2000 24"
fi
PROCESSNAME=$1
NEVENT=$2
NCORE=${3:-1}

echo cmssw_base: $CMSSW_BASE
echo cmssw_version: $CMSSW_VERSION|grep 10_6_0 || { echo "It is not 10_6_0... Exiting...";exit 1; }

[ -f $CMSSW_BASE/src/MY/sherpa/python/sherpa_${PROCESSNAME}_MASTER_cff.pyc ] || { echo "No python config for $PROCESSNAME... Exiting...";exit 1; }
echo $NEVENT|grep -q ^[0-9] && { echo "NEVENT=$NEVENT is not integer... Exiting...";exit 1; }
echo $NCORE|grep -q ^[0-9] && { echo "NCORE=$NCORE is not integer... Exiting...";exit 1; }

for i in $(seq 1 $NCORE)
do
    SEED=$RANDOM$(date +%S)
    WORKING_DIR=$SHERPADAY_BASE/Sherpa/Event/$PROCESSNAME/run$SEED
    mkdir -p $WORKING_DIR
    cd $WORKING_DIR
    echo "#!/bin/bash" > run${SEED}.sh
    echo "time cmsDriver.py MY/sherpa/python/sherpa_${PROCESSNAME}_MASTER_cff.py -s GEN -n $NEVENT --conditions auto:mc --eventcontent RAWSIM  --customise_commands process.RandomNumberGeneratorService.generator.initialSeed=$SEED" >> run${SEED}.sh
    chmod +x run${SEED}.sh
    if [[ $SHERPADAY_USECONDOR ]]
    then
	condor_submit -batch-name Sherpa_GenerateEvent_$PROCESSNAME <<EOF
executable = run${SEED}.sh
output = run${SEED}.out
error = run${SEED}.err
log = run${SEED}.log
should_transfer_files = YES
when_to_transfer_output = ON_EXIT
transfer_output_files = sherpa_ttH_MASTER_cff_py_GEN.root,sherpa_ttH_MASTER_cff_py_GEN.py
getenv = true
queue
EOF
    fi
done
