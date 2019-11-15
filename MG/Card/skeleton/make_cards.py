import os,sys

use_raw_input = True

try: script = sys.argv[1]
except: use_raw_input = True
else:
  use_raw_input = False
  input_lines = open(script).readlines()

get_input_line = 0

def get_input(commands, use_raw_input):
  if use_raw_input == True:
    return raw_input(">>> "+commands)
  else:
    global get_input_line
    this_line = input_lines[get_input_line].strip()
    get_input_line = get_input_line+1
    return this_line


###get ufo model lists from the url
cernweb = get_input("Use UFO models imported in MadGraph? [y,n]\n", use_raw_input)
if cernweb == "n":
  print "Getting available UFO models from https://cms-project-generators.web.cern.ch/cms-project-generators/"
  os.system("curl https://cms-project-generators.web.cern.ch/cms-project-generators/ &>cms-project-generators.dat")
  ufolines = open("cms-project-generators.dat").readlines()

  ###print available ufo lines
  ufoformat = [".zip", ".tar.gz", ".tgz", ".tar"]
  ufomodels = []
  for ufoline in ufolines:
    if (any(s in ufoline for s in ufoformat) and len(ufoline.split(">")) == 4) and not ("MG5_aMC_v" in ufoline):
      print ufoline.split(">")[2].split("</a")[0]
      ufomodels.append(ufoline.split(">")[2].split("</a")[0])
  os.system("rm cms-project-generators.dat")


###start making cards
print "................................................................................"
print "................................................................................"
print "................................................................................"
print "................................................................................"


###making proc_cards
proc_card_OUTPUT = get_input("What is the name of output?\n\te.g. TypeI_HeavyN_LL_M1200\n", use_raw_input)
if os.path.isdir("../"+proc_card_OUTPUT):
  DIREXISTS = get_input("Output already exists, remove the older directory? [y,n]\n", use_raw_input)
  if DIREXISTS == "y": os.system("rm -rf ../"+proc_card_OUTPUT)
  elif DIREXISTS == "n": sys.exit("exiting...")
os.system("mkdir ../"+proc_card_OUTPUT)

proc_card = open("../"+proc_card_OUTPUT+"/"+proc_card_OUTPUT+"_proc_card.dat", "a")
if cernweb == "n":
  proc_card_IMPORTMODEL = get_input("What is the UFO model card?\n\tInclude the format and also the name of untarred dir if it is different from the tarred dir name\n\te.g. SM_HeavyN_NLO_UFO.tgz && SM_HeavyN_NLO\n\t(there are some cases when tarred and untarred dir names are different)\n", use_raw_input)

  extramodels = open("../"+proc_card_OUTPUT+"/"+proc_card_OUTPUT+"_extramodels.dat", "w")
  if len(proc_card_IMPORTMODEL.split("&&")) == 2:
    proc_card.write("import model "+proc_card_IMPORTMODEL.split("&&")[1]+"\n")
  else:
    proc_card.write("import model "+proc_card_IMPORTMODEL.split(".")[0]+"\n")
  
proc_card_GENERATE1 = get_input("Need to define multiparticles?\n\te.g. ee = e+ e- && mm = mu+ mu- && ww = w+ w-\n\tELSE, just press ENTER\n", use_raw_input)
if proc_card_GENERATE1 != "":
  proc_card_GENERATE1 = proc_card_GENERATE1.split("&&")
  for i in range(len(proc_card_GENERATE1)):
    proc_card.write("define "+ proc_card_GENERATE1[i]+"\n")

order = "LO"
proc_card_GENERATE2 = get_input("What is the process?\n\te.g. p p > n1 ee, n1 > ee j j ### for LO && p p > n1 mm, n1 > mm j j\n\te.g. p p > ee n1 [QCD] && p p > mm n1 [QCD] ### for NLO\n", use_raw_input)
###WARNING https://answers.launchpad.net/mg5amcnlo/+question/661535
###QM breaking NLO processes might lead to problems for charge assignments
proc_card_GENERATE2 = proc_card_GENERATE2.split("&&")
for i in range(len(proc_card_GENERATE2)):
  if i == 0:
    proc_card.write("generate "+ proc_card_GENERATE2[i]+"\n")
    if "[QCD]" in proc_card_GENERATE2[i]: order = "NLO"
  else : proc_card.write("add process "+ proc_card_GENERATE2[i]+"\n")

proc_card.write("output "+proc_card_OUTPUT)
proc_card.close()


###making madspin_card ONLY FOR NLO
if order == "NLO":
  madspin_card_DECAY = get_input("What is the decay for MadSpin?\n\te.g. n1 > ee ww, ww > j j && n1 > mm ww, ww > j j\n\tELSE, if MadSpin is NOT NEEDED, just press ENTER\n", use_raw_input)
  if madspin_card_DECAY != "":
    madspin_card = open("../"+proc_card_OUTPUT+"/"+proc_card_OUTPUT+"_madspin_card.dat", "a")
    madspin_card.write("set ms_dir ./madspingrid\nset Nevents_for_max_weigth 250\nset max_weight_ps_point 400\nset max_running_process 1\n")
    madspin_card_DECAY = madspin_card_DECAY.split("&&")
    for i in range(len(madspin_card_DECAY)):
      madspin_card.write("decay "+madspin_card_DECAY[i]+"\n")
    madspin_card.write("launch")
    madspin_card.close()

###making run_cards
os.system("cp "+order+"/run_card.dat ../"+proc_card_OUTPUT+"/"+proc_card_OUTPUT+"_run_card.dat")


###making customizecards.dat
customizecards = open("../"+proc_card_OUTPUT+"/"+proc_card_OUTPUT+"_customizecards.dat", "a")
customizecards_CUSTOMIZE = get_input("Customise the parameters/cuts?\n\te.g. set param_card mass 9900012 1200 && compute_widths 9900012 && set param_card vmun1 0.1 && set param_card ven1 0.1 && set run_card ptl 10\n\tELSE, just press ENTER\n", use_raw_input)
if customizecards_CUSTOMIZE != "":
  customizecards_CUSTOMIZE = customizecards_CUSTOMIZE.split("&&")
  for i in range(len(customizecards_CUSTOMIZE)):
    customizecards.write(customizecards_CUSTOMIZE[i]+"\n")
customizecards.close()
