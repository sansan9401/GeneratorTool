#!/bin/bash

CONCURRENCY_LIMITS=""
MEMORY=5000
POST_PROCESS=""

PARAMS=""
while (( "$#" )); do
    case "$1" in
	--nmax) ## condor concurrency_limits
	    echo "$2"|grep -q "[^0-9]" && { echo "NMAX=$NMAX is not integer... Exiting...";exit 1; }
	    CONCURRENCY_LIMITS="concurrency_limits = n${2}.$USER"
	    shift 2;;
	--memory) ## condor request_memory
	    MEMORY=$2
	    shift 2;;
	--post-process) ## post-process script
	    POST_PROCESS=$(readlink -m $2)
	    shift 2;;
	--delete) ## delete all except event root file
	    DELETE=true
	    shift 1;;
	--) ## end argument parsing
	    shift
	    break;;
	-*|--*=) # unsupported flags
	    echo "Error: Unsupported flag $1" >&2
	    exit 1;;
	*) # preserve positional arguments
	    PARAMS="$PARAMS $1"
	    shift;;
    esac
done
# set positional arguments in their proper place
eval set -- "$PARAMS"

if [ -z "$1" -o -z "$2" -o -z "$3" ]
then
    echo "usage: $0 Sherpa|MG PROCESSNAME NEVENT [NCORE] [--nmax NMAX] [--memory MEMORY] [--post-process POST_PROCESS] [--delete]"
    echo "example: $0 Sherpa ttH 2000 24"
    exit 1
fi

test -z "$GENERATORTOOLS_BASE" && { echo "Please source setup.sh";exit 1; }
source $GENERATORTOOLS_BASE/bin/functions.sh
CheckCMSSW

GENERATOR=$1
PROCESSNAME=$2
NEVENT=$3
NCORE=${4:-1}

echo "GENERATOR=$GENERATOR"
echo "PROCESSNAME=$PROCESSNAME"
echo "NEVENT=$NEVENT"
echo "NCORE=$NCORE"

echo $NEVENT|grep -q "[^0-9]" && { echo "NEVENT=$NEVENT is not integer... Exiting...";exit 1; }
echo $NCORE|grep -q "[^0-9]" && { echo "NCORE=$NCORE is not integer... Exiting...";exit 1; }

if [ -n "$POST_PROCESS" ];then
    echo -n "Check script... "
    CHECK=$(root -l -b < <(echo .L $POST_PROCESS+) 2>&1)
    if echo $CHECK|grep -q error
    then echo $CHECK; exit 1;
    else echo "success"
    fi
fi

if [ "$GENERATOR" = "Sherpa" ];then
    PYTHON_CONFIG_PATH="$CMSSW_BASE/src/MY/sherpa/python/sherpa_${PROCESSNAME}_MASTER_cff.py"
    CMSRUN_CMD='time cmsDriver.py ${PYTHON_CONFIG_PATH/"$CMSSW_BASE/src/"/} -s GEN -n $NEVENT --conditions auto:mc --eventcontent RAWSIM  --customise_commands \"process.RandomNumberGeneratorService.generator.initialSeed=$SEED\\nprocess.MessageLogger.cerr.FwkReport.reportEvery=1000\"'
    EDM_OUTPUT="sherpa_${PROCESSNAME}_MASTER_cff_py_GEN.root"
elif [ "$GENERATOR" = "MG" ];then
    PYTHON_CONFIG_PATH="$CMSSW_BASE/src/MY/mg/python/${PROCESSNAME}.py"
    CMSRUN_CMD='time cmsDriver.py ${PYTHON_CONFIG_PATH/"$CMSSW_BASE/src/"/} -s LHE,GEN -n $NEVENT --conditions auto:mc --eventcontent RAWSIM  --customise_commands \"process.RandomNumberGeneratorService.externalLHEProducer.initialSeed=$SEED\\nprocess.RandomNumberGeneratorService.generator.initialSeed=$SEED\\nprocess.MessageLogger.cerr.FwkReport.reportEvery=1000\"'
    EDM_OUTPUT="${PROCESSNAME}_py_LHE_GEN.root"
else
    echo "Improper GENERATOR=$GENERATOR"
    exit 1
fi

[ -f "${PYTHON_CONFIG_PATH}c" ] || { echo "No $GENERATOR python config for $PROCESSNAME... Exiting...";exit 1; }

echo -n Submitting jobs
for i in $(seq 1 $NCORE);do
    ## Make run directory with proper seed
    SEED=$RANDOM$(date +%S)
    WORKING_DIR=$GENERATORTOOLS_BASE/$GENERATOR/Event/$PROCESSNAME/run$SEED
    while [ -d $WORKING_DIR ];do
	SEED=$RANDOM$(date +%S)
	WORKING_DIR=$GENERATORTOOLS_BASE/$GENERATOR/Event/$PROCESSNAME/run$SEED
    done
    mkdir -p $WORKING_DIR
    cd $WORKING_DIR

    ## run bash script
    echo "#!/bin/bash" > run.sh
    echo "export ROOT_HIST=0" >> run.sh
    echo "cd $WORKING_DIR" >> run.sh
    eval echo $CMSRUN_CMD >> run.sh
    OUTPUT="$EDM_OUTPUT"
    if [ ! -z "$POST_PROCESS" ];then
	echo 'echo -e ".L '$POST_PROCESS'+\n loop({\"'$EDM_OUTPUT'\"},\"events.root\");\n .q"|root -l -b' >> run.sh
	OUTPUT="events.root"
    fi
    [ "$DELETE" = "true" ] && echo "ls -1|egrep -v '^${OUTPUT}$|run.err|run.out|run.sh|run.log'|xargs -i rm -r {}" >> run.sh
    chmod +x run.sh

    ## submit job
    if [[ $GENERATORTOOLS_USECONDOR ]];then
	condor_submit -batch-name ${GENERATOR}_MakeEvent_$PROCESSNAME <<EOF > /dev/null
executable = run.sh
output = run.out
error = run.err
log = run.log
getenv = true
request_memory = $MEMORY
$CONCURRENCY_LIMITS
queue
EOF
    else
	./run.sh 1>run.out 2>run.err &
    fi
    echo -n '.'
done
echo ""
