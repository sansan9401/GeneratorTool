# GeneratorTool

## Updates
  - The names of shell scripts and usage have been changed a little
  - Trying to use consistent syntax: `bin/MakeSomething.sh GENERATOR PROCESSNAME OPTIONS`
  - Adding new shell scripts: `GetInfo.sh`, `SetTune.sh`, ...
  - No need to use `nocmsenv` option anymore
  - histogram root files will go to `$GENERATORTOOLS_BASE/Hist`

## 0. `source setup.sh`
  - This will download CMSSW_10_2_18 and genproduction if they don't exist. And it also change some lines in those packages.
  - If Condor is available, it will set `$GENERATORTOOLS_USECONDOR` to use condor.
  
## 1. put card at Sherpa/Card or MG/Card
  - Sherpa card is a file.
  - MG card is a directory.
  - See existing cards for example.
  - Currently, Many libraries are missing in OpenLoops 2.1.0 in CMSSW_10_2_18. To use local OpenLoops 2.1.0 in SNU cluster, `./bin/SetOpenLoopsPath.sh /data4/Users/hsseo/GeneratorTools/OpenLoops-2.1.0`
  
## 2. `bin/MakeGridpack.sh Sherpa|MG PROCESSNAME [NCORE] [fragment_template]`
  - Make Gridpack using card, place geneneration python fragment at CMSSW directory, and compile
  - For MG gridpack generation, default tune is CP5. You can change by specifying the option or using `SetTune.sh` (see below)

## 3. `bin/MakeEvent.sh Sherpa|MG PROCESSNAME NEVENT [NCORE]`
  - Generating events using gridpack.

## 4. `bin/MakeHist.sh Sherpa|MG PROCESSNAME ROOTSCRIPT`
  - Make histogram root file using a script
  - Histogram root file go to `Hist` directory

# More

## `bin/GetInfo.sh Sherpa|MG PROCESSNAME`
  - Print useful information

## `bin/SetTune.sh MG PROCESSNAME fragment_template`
  - You can change tune using template python files in `python` directory

## `bin/CopyGridpack.sh MG SOURCE_PROCESSNAME TARGET_PROCESSNAME [TEMPLATE]`
  - If you want same MG gridpack with different tune, you can copy existing gridpack and just change the python template
