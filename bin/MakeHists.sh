#!/bin/bash

if [ -z "$1" -o -z "$2" ]
then
    echo "usage: $0 <ROOTSCRIPT> <EVENT_DIR>"
    echo "example: $0 script/ttV.cc Sherpa/Event/ttH"
    exit 1
fi
ROOTSCRIPT=$(realpath $1)
EVENT_DIR=$(realpath $2)
PROCESSNAME=$(basename $EVENT_DIR)

echo cmssw_base: $CMSSW_BASE
echo cmssw_version: $CMSSW_VERSION|grep 10_6_0 || { echo "it is not 10_6_0... Exiting...";exit 1; }

FILES=($(find $EVENT_DIR -maxdepth 2 -name "*GEN*.root"))
WAITLIST=()
echo ${FILES[@]}
for FILE in "${FILES[@]}"
do
    cd $GENERATORTOOLS_BASE
    REALPATH=$(realpath $FILE)
    DIRNAME=$(dirname $REALPATH)
    BASENAME=$(basename $REALPATH)

    cd $DIRNAME
    SCRIPT=MakeHists_${PROCESSNAME}
    echo "#!/bin/bash" >$SCRIPT
    echo 'echo -e ".L '$ROOTSCRIPT'\n loop(\"'$REALPATH'\",\"'$DIRNAME'/hists.root\");\n .q"|root -l -b' >>$SCRIPT
    chmod +x $SCRIPT
    
    if [[ $GENERATORTOOLS_USECONDOR ]]
    then
	condor_submit -batch-name $SCRIPT <<EOF
executable = $SCRIPT
output = $SCRIPT.out
error = $SCRIPT.err
log = $SCRIPT.log
getenv = true
queue
EOF
	WAITLIST+=($DIRNAME/$SCRIPT.log)
    fi
done

for WAITTARGET in "${WAITLIST[@]}"
do 
    if [[ $GENERATORTOOLS_USECONDOR ]]
    then
	condor_wait $WAITTARGET
    fi
done

hadd -f $EVENT_DIR/hists.root $EVENT_DIR/run*/hists.root
