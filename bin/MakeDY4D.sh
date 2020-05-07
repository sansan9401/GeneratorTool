#!/bin/bash

if [ -z "$1" -o -z "$2" -o -z "$3" ]
then
    echo "usage: $0 Sherpa|MG PROCESSNAME NEVENT [NJOB] [NMAX]"
    echo "example: $0 MG DY_NLO0 2000 100 10"
    exit 1
fi

test -z "$GENERATORTOOLS_BASE" && { echo "Please source setup.sh";exit 1; }
source $GENERATORTOOLS_BASE/bin/functions.sh
CheckCMSSW

GENERATOR=$1
PROCESSNAME=$2
NEVENT=$3
NJOB=${4:-1}
NMAX=${5:-0}

echo "GENERATOR=$GENERATOR"
echo "PROCESSNAME=$PROCESSNAME"
echo "NEVENT=$NEVENT"
echo "NJOB=$NJOB"
echo "NMAX=$NMAX"

echo $NEVENT|grep -q "[^0-9]" && { echo "NEVENT=$NEVENT is not integer... Exiting...";exit 1; }
echo $NJOB|grep -q "[^0-9]" && { echo "NJOB=$NJOB is not integer... Exiting...";exit 1; }
echo $NMAX|grep -q "[^0-9]" && { echo "NMAX=$NMAX is not integer... Exiting...";exit 1; }
    
if [ "$GENERATOR" = "Sherpa" ]
then
    [ -f $CMSSW_BASE/src/MY/sherpa/python/sherpa_${PROCESSNAME}_MASTER_cff.pyc ] || { echo "No Sherpa python config for $PROCESSNAME... Exiting...";exit 1; }
    echo -n Submitting jobs
    for i in $(seq 1 $NJOB)
    do
	SEED=$RANDOM$(date +%S)
	WORKING_DIR=$GENERATORTOOLS_BASE/Sherpa/Event/$PROCESSNAME/run$SEED
	while [ -d $WORKING_DIR ]
	do
	    SEED=$RANDOM$(date +%S)
	    WORKING_DIR=$GENERATORTOOLS_BASE/Sherpa/Event/$PROCESSNAME/run$SEED
	done
	mkdir -p $WORKING_DIR
	cd $WORKING_DIR
	echo "#!/bin/bash" > run${SEED}.sh
	echo "cd $WORKING_DIR" >>run${SEED}.sh
	echo "time cmsDriver.py MY/sherpa/python/sherpa_${PROCESSNAME}_MASTER_cff.py -s GEN -n $NEVENT --conditions auto:mc --eventcontent RAWSIM  --customise_commands process.RandomNumberGeneratorService.generator.initialSeed=$SEED" >> run${SEED}.sh
	chmod +x run${SEED}.sh
	if [[ $GENERATORTOOLS_USECONDOR ]]
	then
	    [ $NMAX -gt 0 ] && CONDOR_NMAX="concurrency_limits = n${NMAX}.$USER" || CONDOR_NAME=""
	    condor_submit -batch-name Sherpa_MakeDY4D_$PROCESSNAME <<EOF > /dev/null
executable = run${SEED}.sh
output = run${SEED}.out
error = run${SEED}.err
log = run${SEED}.log
getenv = true
request_memory = 5G
$CONDOR_NMAX
queue
EOF
	else
	    ./run${SEED}.sh 1>run${SEED}.out 2>run${SEED}.err &
	fi
	echo -n '.'
    done
    echo ""
elif [ "$GENERATOR" = "MG" ]
then
    [ -f $CMSSW_BASE/src/MY/mg/python/${PROCESSNAME}.pyc ] || { echo "No MG python config for $PROCESSNAME... Exiting...";exit 1; }

    echo -n Submitting jobs
    for i in $(seq 1 $NJOB)
    do
	SEED=$RANDOM$(date +%S)
	WORKING_DIR=$GENERATORTOOLS_BASE/MG/Event/$PROCESSNAME/run$SEED
	while [ -d $WORKING_DIR ]
	do
	    SEED=$RANDOM$(date +%S)
	    WORKING_DIR=$GENERATORTOOLS_BASE/MG/Event/$PROCESSNAME/run$SEED
	done
	mkdir -p $WORKING_DIR
	cd $WORKING_DIR
	echo "#!/bin/bash" > run${SEED}.sh	
	echo "cd $WORKING_DIR" >>run${SEED}.sh
	echo "time cmsDriver.py MY/mg/python/${PROCESSNAME}.py -s LHE,GEN -n $NEVENT --conditions auto:mc --eventcontent RAWSIM  --customise_commands \"process.RandomNumberGeneratorService.externalLHEProducer.initialSeed=$SEED\nprocess.RandomNumberGeneratorService.generator.initialSeed=$SEED\"" >> run${SEED}.sh
	echo "echo -e \".L $GENERATORTOOLS_WD/script/DY4D.
	chmod +x run${SEED}.sh
	if [[ $GENERATORTOOLS_USECONDOR ]]
	then
	    condor_submit -batch-name MG_MakeDY4D_$PROCESSNAME <<EOF >/dev/null
executable = run${SEED}.sh
output = run${SEED}.out
error = run${SEED}.err
log = run${SEED}.log
#should_transfer_files = YES
#when_to_transfer_output = ON_EXIT
#transfer_output_files = ${PROCESSNAME}_py_LHE_GEN.py,${PROCESSNAME}_py_LHE_GEN.root
getenv = true
request_memory = 5G
queue
EOF
	else
	    ./run${SEED}.sh 1>run${SEED}.out 2>run${SEED}.err &
	fi
	echo -n '.'
    done
    echo ""
else
    echo "Improper GENERATOR=$GENERATOR"
    exit 1
fi
