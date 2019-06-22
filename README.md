# GeneratorTool

## 0. `source setup.sh`
  - This will download CMSSW_10_6_0 and genproduction if they don't exist. And it also change some lines in those packages.
  - Without any option, it will set cmsenv. If nocmsenv option is provided, it will not set cmsenv. This is for MG gridpack generation.
  - If Condor is available, it will set `$GENERATORTOOLS_USECONDOR` to use condor.
  
## 1. put card at Sherpa/Card or MG/Card
  - Sherpa card is a file.
  - MG card is a directory.
  - See existing cards for example.
  
## 2. `bin/MakeGridpack.sh CARDPATH [NCORE]`
  - Make Gridpack using card, place geneneration python fragment at CMSSW directory, and compile
  - For MG gridpack generation, you should start with clean shell and `source setup.sh nocmsenv`
  - WIP for MG

## 3. `bin/GenerateEvent.sh Sherpa|MG PROCESSNAME NEVENT [NCORE]`
  - Generating events using gridpack.

## 4. `bin/MakeHists.sh <ROOTSCRIPT> <EVENT_DIR>`
  - histograms using a script.
  - WIP
