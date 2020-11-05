ncore=24
samples=(ttMuMu_Dielec_NLO_bMassive ttMuMu_Dielec_NLO_bUnstable)


generater=Sherpa # MG Sherpa

for sample in ${samples[@]}
do
  echo "" &>> run.log &
  echo "" &>> run.log &
  echo "=======================================" &>> run.log &
  echo "[Make Gridpack] generater: " $generater ", sample: " ${sample} ", NCORE = " ${ncore} &>> run.log &

  #now=$(date +"%T")
  #echo "Start time : $now"
  ./bin/MakeGridpack.sh $generater ${sample} ${ncore} &>> run.log &
  #now=$(date +"%T")
  #echo "End time : $now"
  echo "=======================================" &>> run.log &
done

sleep 5
disown -a

echo "+ END RUN +"
