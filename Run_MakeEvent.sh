ncore=100
nevent=1000

samples=(ttMuMu_Dielec_NLO_bUnstable ttMuMu_Dielec_NLO_bMassive)
#samples=(ttMuMu_Dielec_NLO_ModifiedJetAlgo)

generater=Sherpa

for sample in ${samples[@]}
do
  echo "" &>>run.log&
  echo "" &>>run.log&
  echo "=======================================" &>>run.log&
  echo "[Make Event] " ${sample} " sample NCORE = " ${ncore} &>>run.log&
  now=$(date +"%T")
  echo "Start time : $now" &>>run.log&
  ./bin/MakeEvent.sh $generater ${sample} ${nevent} ${ncore} &>>run.log&
  now=$(date +"%T")
  echo "End time : $now" &>>run.log&
  echo "=======================================" &>>run.log&
done

echo "+ END RUN +"
