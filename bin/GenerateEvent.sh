#!/bin/bash

if [ -z "$1" -o -z "$2" ]
then
    echo "usage: $0 Sherpa|MG PROCESSNAME NEVENT [NCORE]"
    echo "example: $0 Sherpa ttH 2000 24"
    exit 1
fi
GENERATOR=$1
PROCESSNAME=$2
NEVENT=$3
NCORE=${4:-1}

echo $NEVENT|grep -q "[^0-9]" && { echo "NEVENT=$NEVENT is not integer... Exiting...";exit 1; }
echo $NCORE|grep -q "[^0-9]" && { echo "NCORE=$NCORE is not integer... Exiting...";exit 1; }
    
echo cmssw_base: $CMSSW_BASE
echo cmssw_version: $CMSSW_VERSION|grep 10_6_0 || { echo "It is not 10_6_0... Exiting...";exit 1; }

if [[ "$GENERATOR" == "Sherpa" ]]
then
    [ -f $CMSSW_BASE/src/MY/sherpa/python/sherpa_${PROCESSNAME}_MASTER_cff.pyc ] || { echo "No Sherpa python config for $PROCESSNAME... Exiting...";exit 1; }
    for i in $(seq 1 $NCORE)
    do
	SEED=$RANDOM$(date +%S)
	WORKING_DIR=$GENERATORTOOLS_BASE/Sherpa/Event/$PROCESSNAME/run$SEED
	mkdir -p $WORKING_DIR
	cd $WORKING_DIR
	echo "#!/bin/bash" > run${SEED}.sh
	echo "cd $WORKING_DIR" >>run${SEED}.sh
	echo "time cmsDriver.py MY/sherpa/python/sherpa_${PROCESSNAME}_MASTER_cff.py -s GEN -n $NEVENT --conditions auto:mc --eventcontent RAWSIM  --customise_commands process.RandomNumberGeneratorService.generator.initialSeed=$SEED" >> run${SEED}.sh
	chmod +x run${SEED}.sh
	if [[ $GENERATORTOOLS_USECONDOR ]]
	then
	    condor_submit -batch-name Sherpa_GenerateEvent_$PROCESSNAME <<EOF
executable = run${SEED}.sh
output = run${SEED}.out
error = run${SEED}.err
log = run${SEED}.log
#should_transfer_files = YES
#when_to_transfer_output = ON_EXIT
#transfer_output_files = sherpa_${PROCESSNAME}_MASTER_cff_py_GEN.root,sherpa_${PROCESSNAME}_MASTER_cff_py_GEN.py
getenv = true
request_memory = 5G
accounting_group = group_cms
queue
EOF
	else
	    ./run${SEED}.sh 1>run${SEED}.out 2>run${SEED}.err &
	fi
    done
elif [[ "$GENERATOR" == "MG" ]]
then
    [ -f $CMSSW_BASE/src/MY/mg/python/${PROCESSNAME}.pyc ] || { echo "No MG python config for $PROCESSNAME... Exiting...";exit 1; }

    for i in $(seq 1 $NCORE)
    do
	SEED=$RANDOM$(date +%S)
	WORKING_DIR=$GENERATORTOOLS_BASE/MG/Event/$PROCESSNAME/run$SEED
	mkdir -p $WORKING_DIR
	cd $WORKING_DIR
	echo "#!/bin/bash" > run${SEED}.sh	
	echo "cd $WORKING_DIR" >>run${SEED}.sh
	echo "time cmsDriver.py MY/mg/python/${PROCESSNAME}.py -s LHE,GEN -n $NEVENT --conditions auto:mc --eventcontent RAWSIM  --customise_commands \"process.RandomNumberGeneratorService.externalLHEProducer.initialSeed=$SEED\nprocess.RandomNumberGeneratorService.generator.initialSeed=$SEED\"" >> run${SEED}.sh
	chmod +x run${SEED}.sh
	if [[ $GENERATORTOOLS_USECONDOR ]]
	then
	    condor_submit -batch-name MG_GenerateEvent_$PROCESSNAME <<EOF
executable = run${SEED}.sh
output = run${SEED}.out
error = run${SEED}.err
log = run${SEED}.log
#should_transfer_files = YES
#when_to_transfer_output = ON_EXIT
#transfer_output_files = ${PROCESSNAME}_py_LHE_GEN.py,${PROCESSNAME}_py_LHE_GEN.root
getenv = true
request_memory = 5G
accounting_group = group_cms
queue
EOF
	else
	    ./run${SEED}.sh 1>run${SEED}.out 2>run${SEED}.err &
	fi
    done
else
    echo "Improper GENERATOR=$GENERATOR"
    exit 1
fi
