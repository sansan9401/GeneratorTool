#!/bin/bash

if [ -z "$1" ]
then 
    echo "usage: $0 CARDPATH [NCORE]"
    echo "example: $0 Sherpa/Card/Run.dat_ttH 4"
    echo "         $0 MG/Card/ttH 4"
    exit 1
fi
CARDPATH=$1
NCORE=${2:-1}
echo "CARDPATH=$CARDPATH"
echo "NCORE=$NCORE"

if [ ! -e $CARDPATH ]
then
    echo "No $CARDPATH... Exiting..."
    exit 1
fi
CARDPATH=$(realpath $CARDPATH)

if echo $(basename $CARDPATH)|grep -q Run.dat
then
    #### Sherpa Gridpack Generation #####
    echo "This is Sherpa card"
    PROCESSNAME=$(basename $CARDPATH|sed 's/Run.dat_//')
    echo "PROCESSNAME=$PROCESSNAME"

    echo cmssw_base: $CMSSW_BASE
    echo cmssw_version: $CMSSW_VERSION|grep 10_6_0 || { echo "it is not 10_6_0... Exiting...";exit 1; }

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
	condor_submit -batch-name Sherap_MakeGridpack_$PROCESSNAME <<EOF
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
    #### Madgraph Gridpack Generation #####
    echo "This is MG card"
    PROCESSNAME=$(basename $CARDPATH)
    echo "PROCESSNAME=$PROCESSNAME"
    NJETMAX=0
    grep " j " $CARDPATH/${PROCESSNAME}_proc_card.dat && NJETMAX=1
    grep " j j " $CARDPATH/${PROCESSNAME}_proc_card.dat && NJETMAX=2
    grep " j j j " $CARDPATH/${PROCESSNAME}_proc_card.dat && NJETMAX=3
    grep " j j j j " $CARDPATH/${PROCESSNAME}_proc_card.dat && NJETMAX=4
    echo "NJETMAX=$NJETMAX"

    [[ $CMSSW_BASE ]] && { echo "Use new shell with 'setup.sh nocmsenv' for MG gridpack generation... Exiting...";exit 1; }

    MG_PYTHON_DIR=$GENERATORTOOLS_BASE/external/CMSSW_10_6_0/src/MY/mg/python
    mkdir -p $MG_PYTHON_DIR
    GRIDPATH=$GENERATORTOOLS_BASE/MG/Gridpack/$PROCESSNAME
    mkdir -p $GRIDPATH
    cd $GRIDPATH

    MG_RUN_DIR=$GENERATORTOOLS_BASE/external/genproductions/bin/MadGraph5_aMCatNLO
    ln -sf $GENERATORTOOLS_BASE/MG/Card $MG_RUN_DIR/Card

    SCRIPT=MG_MakeGridpack_${PROCESSNAME}.sh
    echo "#!/bin/bash" > $SCRIPT
    echo "cd $MG_RUN_DIR" >>$SCRIPT
    echo "time NB_CORE=$NCORE ./gridpack_generation.sh $PROCESSNAME Card/$PROCESSNAME" >>$SCRIPT
    echo "mv ${PROCESSNAME} ${PROCESSNAME}.log ${PROCESSNAME}_slc?_amd??_gcc???_CMSSW_*_tarball.tar.xz $GRIDPATH/" >>$SCRIPT
    echo "cd $GRIDPATH" >>$SCRIPT
    if grep "\[QCD\]" $CARDPATH/${PROCESSNAME}_proc_card.dat 
    then
	if [ $NJETMAX -gt 0 ]
	then
	    echo "cp $GENERATORTOOLS_BASE/template_MG_FXFX_cff.py $MG_PYTHON_DIR/${PROCESSNAME}.py" >>$SCRIPT
	else
	    echo "cp $GENERATORTOOLS_BASE/template_MG_NLO_cff.py $MG_PYTHON_DIR/${PROCESSNAME}.py" >>$SCRIPT
	fi	    
    else 
	if [ $NJETMAX -gt 0 ]
	then 
	    echo "cp $GENERATORTOOLS_BASE/template_MG_MLM_cff.py $MG_PYTHON_DIR/${PROCESSNAME}.py" >>$SCRIPT
	else
	    echo "cp $GENERATORTOOLS_BASE/template_MG_LO_cff.py $MG_PYTHON_DIR/${PROCESSNAME}.py" >>$SCRIPT
	fi
    fi	
    echo "sed -i 's@GRIDPACKLOCATION@'\$(find $GRIDPATH -type f -name \"${PROCESSNAME}_*_tarball.tar.xz\")'@' $MG_PYTHON_DIR/${PROCESSNAME}.py" >>$SCRIPT
    echo "sed -i 's@NJETMAX@$NJETMAX@' $MG_PYTHON_DIR/${PROCESSNAME}.py" >>$SCRIPT
    echo "ln -sf $MG_PYTHON_DIR/${PROCESSNAME}.py $GRIDPATH/" >>$SCRIPT
    echo "cd $MG_PYTHON_DIR; cmsenv; scram b" >>$SCRIPT
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

