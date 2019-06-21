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
    WORKING_DIR=$(mktemp -d -t ${PROCESSNAME}_XXXXXXXX --tmpdir=$SHERPA_DATA_DIR)
    cd $WORKING_DIR
    cp $CARDPATH $WORKING_DIR

    echo "#!/bin/bash" > Sherpa_MakeGridpack_${PROCESSNAME}.sh
    echo "time $SHERPA_DATA_DIR/MakeSherpaLibs.sh -v -o LBCR -p $PROCESSNAME -m mpirun -M '-n $NCORE'" >> Sherpa_MakeGridpack_${PROCESSNAME}.sh
    chmod +x Sherpa_MakeGridpack_${PROCESSNAME}.sh
  
    if [[ $SHERPADAY_USECONDOR ]]
    then
	condor_qsub -cwd -V -l nodes=1:ppn=$NCORE Sherpa_MakeGridpack_${PROCESSNAME}.sh
    else 
	./Sherpa_MakeGridpack_${PROCESSNAME}.sh
    fi
    
    ln -sf $WORKING_DIR $SHERPADAY_BASE/Sherpa/Gridpack/$PROCESSNAME    
else
    #### Madgraph Gridpack Generation #####
    echo "This is MG card"
    PROCESSNAME=$(basename $CARDPATH)
    echo "PROCESSNAME=$PROCESSNAME"

    [[ $CMSSW_BASE ]] && { echo "Use new shell with 'setup.sh nocmsenv' for MG gridpack generation... Exiting...";exit 1; }

    MG_DIR=$SHERPADAY_BASE/Tool/genproductions/bin/MadGraph5_aMCatNLO
    ln -sf $SHERPADAY_BASE/MG/Card $MG_DIR/Card
    cd $MG_DIR
    echo "#!/bin/bash" > MG_MakeGridpack_${PROCESSNAME}.sh
    echo "time NB_CORE=$NCORE ./gridpack_generation.sh $PROCESSNAME Card/$PROCESSNAME" >> MG_MakeGridpack_${PROCESSNAME}.sh
    chmod +x MG_MakeGridpack_${PROCESSNAME}.sh

    if [[ $SHERPADAY_USECONDOR ]]
    then
	condor_qsub -cwd -V -l nodes=1:ppn=$NCORE MG_MakeGridpack_${PROCESSNAME}.sh
    else
	./MG_MakeGridpack_${PROCESSNAME}.sh
    fi

fi

