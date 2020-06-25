#!/usr/bin/env python
import os,sys,tempfile,subprocess
from functions import *
import argparse

## Argument Parser
parser = argparse.ArgumentParser(description="Make histograms with a script")
parser.add_argument("Generator",type=str,help="Generator name (MG or Sherpa)")
parser.add_argument("ProcessName",type=str,help="Process name used in MakeEvent")
parser.add_argument("RootScript",type=str,nargs="+",help="ROOT script to process")
parser.add_argument("-n","--njobs",type=int,default=0,help="The total number of jobs. default=Nfiles")
args=parser.parse_args()
print(args)

## Check
if not CheckCMSSW(): exit(1)
for i in range(len(args.RootScript)):
    if not CheckScript(args.RootScript[i]): 
        exit(1)
    else: 
        args.RootScript[i]=os.path.abspath(args.RootScript[i])

## Get list of files
files=os.popen("find $GENERATORTOOLS_BASE/"+args.Generator+"/Event/"+args.ProcessName+" -maxdepth 2 -name events.root").read().split()
if len(files)==0:
    files=os.popen("find $GENERATORTOOLS_BASE/"+args.Generator+"/Event/"+args.ProcessName+" -maxdepth 2 -name \"*GEN*.root\"").read().split()
if len(files)==0:
    print("No root file... Exit...")
    exit(1)
print("Total "+str(len(files))+" root files are detected")

if args.njobs==0 or args.njobs>len(files) : args.njobs=len(files)
step=len(files)/args.njobs
if len(files)%args.njobs: step+=1

sys.stdout.write("Submitting "+str(args.njobs)+" jobs")
sys.stdout.flush()

workingdir=tempfile.mkdtemp(prefix="_".join(["MakeHist",args.Generator,args.ProcessName,""]),dir=os.getenv("GENERATORTOOLS_BASE")+"/Hist/")

## Make run script
for i in range(args.njobs):
    script=workingdir+"/run"+str(i)+".sh"
    with open(script,"w") as f:
        filenames=[",".join(['\\"'+s+'\\"' for s in files[i*step:(i+1)*step]])]
        filenames+=["\\\""+workingdir+"/step"+str(j)+"_hists"+str(i)+".root\\\"" for j in range(len(args.RootScript))]
        filenames[-1]="\\\""+workingdir+"/hists"+str(i)+".root\\\""
        f.write(
'''#!/bin/bash
export ROOT_HIST=0
''')
        for j in range(len(args.RootScript)):
            f.write(
'''echo -e ".L {0}+\n loop({{{1}}},{2});\n.q"|root -l -b
'''.format(args.RootScript[j],filenames[j],filenames[j+1]))
        os.chmod(script,0o774)

## Submit jobs and wait
if os.getenv("GENERATORTOOLS_USECONDOR"):
    with open(workingdir+"/condor.jds","w") as f:
        f.write(
'''executable = run$(Process).sh
output = run$(Process).out
error =  run$(Process).err
log = condor.log
getenv = true
queue {0}
'''.format(args.njobs))
    os.system("cd "+workingdir+";condor_submit -batch-name "+"_".join(["MakeHist",args.Generator,args.ProcessName])+" condor.jds")
    os.system("condor_wait "+workingdir+"/condor.log")
else:
    waitlist=[]
    for i in range(args.njobs):
        waitlist+=[subprocess.Popen("cd "+workingdir+";./run"+str(i)+".sh 1>run"+str(i)+".err 2>run"+str(i)+".err",shell=True)]
    for p in waitlist: p.wait()

## hadd
if os.system("hadd -f $GENERATORTOOLS_BASE/Hist/"+"_".join([os.path.splitext(os.path.basename(args.RootScript[-1]))[0],args.Generator,args.ProcessName])+".root "+" ".join([workingdir+"/hists"+str(i)+".root" for i in range(args.njobs)])) == 0:
    os.system("rm -r "+workingdir)
else:
    print("Error... Check logs in "+workingdir)
    exit(1)
