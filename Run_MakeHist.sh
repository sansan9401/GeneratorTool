samples=(ttMuMu_Dielec_mll30_NLO ttMuMu_Dielec_NLO ttMuMu_Dielec_NLO_2 ttMuMu_Dielec_NLO_3)
samples=(ttMuMu_Dielec_NLO_bUnstable)
samples=(ttMuMu_Dielec_NLO_ModifiedJetAlgo ttMuMu_Dielec_NLO)
#samples=(ttMuMu_Dielec_NLO_bMassive)

HistScript=ttMuMu_Dielec #WW_leptonic #ttV_leptonic
generators=(MG Sherpa)
generators=(Sherpa)
generators=(MG)

#rm script/${HistScript}_cc_*

for generator in ${generators[@]}
do
  for sample in ${samples[@]}
  do

    rm Hist/${HistScript}_${generators}_${sample}.root

    echo "" 
    echo "" 
    echo "=======================================" 
    echo "[Make Hist] " ${sample} " : " ${generator}
    now=$(date +"%T")
    echo "Start time : $now" 
    ./bin/MakeHist.sh ${generator} ${sample} script/${HistScript}.cc &>> run.log &  #MakeHist_${generator}_${sample}.log&
    now=$(date +"%T")
    echo "End time : $now" 
    echo "=======================================" 
  done
done

disown -a

echo "+ END RUN +"
