import os,sys

def CheckCMSSW():
    generatortools_cmssw_version=os.getenv("GENERATORTOOLS_CMSSW_VERSION")
    if not generatortools_cmssw_version: 
        print("Please source setup.sh")
        return False
    cmssw_base=os.getenv("CMSSW_BASE")
    cmssw_version=os.getenv("CMSSW_VERSION")
    print("cmssw_base: "+str(cmssw_base))
    print("cmssw_version: "+str(cmssw_version))
    if cmssw_version != generatortools_cmssw_version:
        print("it is not "+generatortools_cmssw_version+"...")
        return False
    return True

def GetNJetMax():
    pass

def CheckScript(path):
    if not os.path.exists(path):
        print(path+" doesn't exist")
        return False
    out=os.popen('echo -e ".L '+path+'+"|root -l -b 2>&1').read()
    if "error" in out:
        print(out)
        return False
    return True
