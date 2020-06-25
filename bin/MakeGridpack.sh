#!/bin/bash

if [ -z "$1" -o -z "$2" ]
then 
    echo "usage: $0 Sherpa|MG PROCESSNAME [NCORE] [fragment_template]"
    echo "example: $0 Sherpa ttH 4"
    echo "         $0 MG ttH 4 python/MG_NLO_CP5_cff.py"
    exit 1
fi

test -z "$GENERATORTOOLS_BASE" && { echo "Please source setup.sh";exit 1; }
source $GENERATORTOOLS_BASE/bin/functions.sh
CheckCMSSW

GENERATOR="$1"
if [ "$GENERATOR" = Sherpa ]
then
    CARDPATH=$GENERATORTOOLS_BASE/$GENERATOR/Card/Run.dat_${2}
elif [ "$GENERATOR" = MG ]
then 
    CARDPATH=$GENERATORTOOLS_BASE/$GENERATOR/Card/${2}
else
    echo "first argument should be Sherpa or MG"
    exit 1
fi
PROCESSNAME=$2
NCORE=${3:-1}
if [ ! -z "$4" ]
then TEMPLATE=$(realpath $4)
else TEMPLATE=AUTO
fi
echo "GENERATOR=$GENERATOR"
echo "PROCESSNAME=$PROCESSNAME"
echo "CARDPATH=$CARDPATH"
echo "NCORE=$NCORE"

if [ ! -e $CARDPATH ]
then
    echo "No $CARDPATH... Exiting..."
    exit 1
fi

if [ -e $GENERATORTOOLS_BASE/$GENERATOR/Gridpack/$PROCESSNAME ]
then
    echo "Gridpack already exist... Remove existing gridpack or change processname"
    echo "To remove: rm -r $GENERATORTOOLS_BASE/$GENERATOR/Gridpack/$PROCESSNAME"
    exit 1
fi

CARDPATH=$(realpath $CARDPATH)

if [ "$GENERATOR" = Sherpa ]
then
    SHERPA_DATA_DIR=$CMSSW_BASE/src/GeneratorInterface/SherpaInterface/data
    SHERPA_PYTHON_DIR=$CMSSW_BASE/src/MY/sherpa/python
    mkdir -p $SHERPA_PYTHON_DIR
    WORKING_DIR=$(mktemp -d -t ${PROCESSNAME}_XXXXXXXX --tmpdir=$SHERPA_DATA_DIR)
    cp $CARDPATH $WORKING_DIR
    GRIDPATH=$GENERATORTOOLS_BASE/Sherpa/Gridpack/$PROCESSNAME
    cd $WORKING_DIR

    SCRIPT=Sherpa_MakeGridpack_${PROCESSNAME}.sh
    echo "#!/bin/bash" > $SCRIPT
    echo "cd $WORKING_DIR" >> $SCRIPT
    echo "time $SHERPA_DATA_DIR/MakeSherpaLibs.sh -v -o LBCR -p $PROCESSNAME -m mpirun -M '-n $NCORE'" >> $SCRIPT
    echo 'test $? -ne 0 && { echo [GeneratorTool] Fail;exit $?; }' >>$SCRIPT
    echo "$SHERPA_DATA_DIR/PrepareSherpaLibs.sh" >>$SCRIPT
    echo "mv $WORKING_DIR $GRIDPATH" >>$SCRIPT
    echo "cd $GRIDPATH" >>$SCRIPT
    echo "sed -i '/FetchSherpack/ s/False/True/' sherpa_${PROCESSNAME}_MASTER_cff.py" >>$SCRIPT
    echo "sed -i '/SherpackLocation/ s@\./@$GRIDPATH@' sherpa_${PROCESSNAME}_MASTER_cff.py" >>$SCRIPT
    echo "mv sherpa_${PROCESSNAME}_MASTER_cff.py $SHERPA_PYTHON_DIR" >>$SCRIPT
    echo "ln -sf $SHERPA_PYTHON_DIR/sherpa_${PROCESSNAME}_MASTER_cff.py ." >>$SCRIPT
    echo "cd $SHERPA_PYTHON_DIR; scram b" >>$SCRIPT

    chmod +x $SCRIPT
  
    if [[ $GENERATORTOOLS_USECONDOR ]]
    then
	condor_submit -batch-name Sherpa_MakeGridpack_$PROCESSNAME <<EOF
executable = Sherpa_MakeGridpack_${PROCESSNAME}.sh
output = Sherpa_MakeGridpack_${PROCESSNAME}.out
error = Sherpa_MakeGridpack_${PROCESSNAME}.err
log = Sherpa_MakeGridpack_${PROCESSNAME}.log
request_cpus = $NCORE
getenv = true
queue
EOF
	condor_wait Sherpa_MakeGridpack_${PROCESSNAME}.log
    else 
	./Sherpa_MakeGridpack_${PROCESSNAME}.sh
    fi
else
    NJETMAX=$(GetNJetMax $GENERATOR $PROCESSNAME)
    echo "NJETMAX=$NJETMAX"

    GRIDPATH=$GENERATORTOOLS_BASE/MG/Gridpack/$PROCESSNAME
    mkdir -p $GRIDPATH
    cd $GRIDPATH

    MG_RUN_DIR=$GENERATORTOOLS_BASE/external/genproductions/bin/MadGraph5_aMCatNLO
    ln -sf $GENERATORTOOLS_BASE/MG/Card $MG_RUN_DIR/

    SCRIPT=MG_MakeGridpack_${PROCESSNAME}.sh
    echo "#!/bin/bash" > $SCRIPT
    echo "cd $MG_RUN_DIR" >>$SCRIPT
    echo time env -i 'HOME=$HOME' NB_CORE=$NCORE 'PYTHONPATH=$PYTHONPATH' bash -l -c \"source /cvmfs/cms.cern.ch/cmsset_default.sh\; ./gridpack_generation.sh $PROCESSNAME Card/$PROCESSNAME local ALL $SCRAM_ARCH $CMSSW_VERSION\" >>$SCRIPT
    echo "mv ${PROCESSNAME}.log ${PROCESSNAME}_slc?_amd??_gcc???_CMSSW_*_tarball.tar.xz $GRIDPATH/" >>$SCRIPT
#    echo "rm -rf ${PROCESSNAME}" >>$SCRIPT
    echo "cd $GRIDPATH" >>$SCRIPT
    if [ "$TEMPLATE" = AUTO ]
    then
	if grep -q "\[QCD\]" $CARDPATH/${PROCESSNAME}_proc_card.dat 
	then
	    if [ "$NJETMAX" -gt 0 ]
	    then TEMPLATE=$GENERATORTOOLS_BASE/python/MG_FXFX_cff.py
	    else TEMPLATE=$GENERATORTOOLS_BASE/python/MG_NLO_cff.py
	    fi	    
	else 
	    if [ "$NJETMAX" -gt 0 ]
	    then TEMPLATE=$GENERATORTOOLS_BASE/python/MG_MLM_cff.py
	    else TEMPLATE=$GENERATORTOOLS_BASE/python/MG_LO_cff.py
	    fi
	fi	
    fi
    echo "Using template $TEMPLATE"
    echo "$GENERATORTOOLS_BASE/bin/SetTune.sh MG $PROCESSNAME $TEMPLATE" >> $SCRIPT
    chmod +x $SCRIPT

    if [[ $GENERATORTOOLS_USECONDOR ]]
    then
	condor_submit -batch-name MG_MakeGridpack_$PROCESSNAME <<EOF
executable = MG_MakeGridpack_${PROCESSNAME}.sh
output = MG_MakeGridpack_${PROCESSNAME}.out
error = MG_MakeGridpack_${PROCESSNAME}.err
log = MG_MakeGridpack_${PROCESSNAME}.log
request_cpus = $NCORE
getenv = true
queue
EOF
	condor_wait MG_MakeGridpack_${PROCESSNAME}.log
    else
	./MG_MakeGridpack_${PROCESSNAME}.sh
    fi    
fi

