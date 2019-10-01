vector<TString> Split(TString s,TString del){
  TObjArray* array=s.Tokenize(del);
  vector<TString> out;
  for(const auto& obj:*array){
    out.push_back(((TObjString*)obj)->String());
  }
  array->Delete();
  return out;
}

////////////////////////Cpus/////////////////////////////////
int GetCpus(TString gridpackdir){
  double ncpu=gSystem->GetFromPipe("find "+gridpackdir+" -maxdepth 1 -name '*.log'|xargs -i tail {}|grep Cpus|awk '{print $4}'").Atoi();
  return ncpu;
}

////////////////////////gridpack generation speed/////////////////////////////////
double GetGridpackGenerationSpeed(TString gridpackdir){ 
  double real=gSystem->GetFromPipe("find "+gridpackdir+" -maxdepth 1 -name '*.err'|xargs -i tail -n 500 {}|grep ^real|sed 's/[^0-9.]/ /g'|awk '{print $1}'").Atof();
  //double user=gSystem->GetFromPipe("tail "+log+"|grep ^user|sed 's/[^0-9.]/ /g'|awk '{print $1}'").Atof();
  //double sys=gSystem->GetFromPipe("tail "+log+"|grep ^sys|sed 's/[^0-9.]/ /g'|awk '{print $1}'").Atof();
  return real;
}

////////////////////////event generation speed/////////////////////////////////
tuple<double,double> GetEventGenerationSpeedAndError(TString eventdir){
  vector<TString> lines=Split(gSystem->GetFromPipe("find "+eventdir+" -maxdepth 1 -name 'run*' -type d|xargs -i find {} -maxdepth 1 -type f -name 'run*.err'|while read filename;do TIME=$(tail -n 500 $filename|grep '^real'|sed 's/[^0-9.]/ /g'|awk '{print $1*60+$2}' || echo 0); NEVENT=$(grep 'Filter efficiency (event-level)' $filename|awk '{print $4}'|sed 's/[^0-9]//g' || echo 0);echo $TIME $NEVENT;done"),"\n");
  double sumwx=0,sumw=0,sumwx2=0;
  for(const auto& line:lines){
    vector<TString> words=Split(line," ");
    if(words.size()!=2){
      cout<<"Somethign wrong at "<<line<<endl;
      continue;
    }
    double time=words[0].Atof();
    double nevent=words[1].Atof();
    if(time==0&&nevent==0){
      cout<<"Unfinished job at "<<line<<endl;
      continue;
    }
    double x=nevent/time*3600*24;
    double w=nevent;
    sumwx+=x*w;
    sumwx2+=x*x*w;
    sumw+=w;
  }
  if(sumw>0) return make_tuple(sumwx/sumw,sqrt(sumwx2/sumw-sumwx*sumwx/sumw/sumw));
  else return make_tuple(-1,-1);
}  
double GetEventGenerationSpeed(TString eventdir){
  return get<0>(GetEventGenerationSpeedAndError(eventdir));
}
double GetEventGenerationSpeedError(TString eventdir){
  return get<1>(GetEventGenerationSpeedAndError(eventdir));
}


////////////////////////cross section/////////////////////////////////
tuple<double,double> GetCrossSectionAndStatError(TString eventdir){  
  vector<TString> lines=Split(gSystem->GetFromPipe("find "+eventdir+" -maxdepth 1 -name 'run*' -type d|xargs -i find {} -maxdepth 1 -type f -name 'run*.err'|xargs -i sh -c \"tail -n 500 {}|grep 'final cross section' || echo {} \""),"\n");
  double sumwx=0,sumw=0;
  for(const auto& line:lines){
    vector<TString> words=Split(line," ");
    if(words.size()!=10){
      cout<<"Unfinished job at "<<line<<endl;
      continue;
    }
    double x=words[6].Atof();
    double err=words[8].Atof();
    if(err>0){
      sumwx+=x/err/err;
      sumw+=1/err/err;
    }
  }
  if(sumw>0) return make_tuple(sumwx/sumw,sqrt(1/sumw));
  else if(sumw==0) return make_tuple(0,0);
  else return make_tuple(-1,-1);
}
double GetCrossSection(TString eventdir){
  return get<0>(GetCrossSectionAndStatError(eventdir));
}
double GetCrossSectionStatError(TString eventdir){
  return get<1>(GetCrossSectionAndStatError(eventdir));
}
double GetCrossSectionSysError(TString eventdir){
  if(gSystem->Exec("ls "+eventdir+"|grep hists.root > /dev/null")!=0) return 0;

  TFile f(eventdir+"/hists.root");
  TH1* hist=(TH1*)f.Get("sumw");
  if(!hist) return 0;

  double maxdiff=0;
  double norm=hist->GetBinContent(1);
  for(int i=5;i<12;i++){
    double diff=fabs(hist->GetBinContent(i)-norm);
    if(diff==norm) continue;
    if(diff>maxdiff){
      maxdiff=diff;
    }
  }
  return maxdiff/norm*GetCrossSection(eventdir);
}

////////////////////////negative weight fraction/////////////////////////////////
tuple<double,double> GetNegativeWeightAndError(TString eventdir){  
  vector<TString> lines=Split(gSystem->GetFromPipe("find "+eventdir+" -maxdepth 1 -name 'run*' -type d|xargs -i find {} -maxdepth 1 -type f -name 'run*.err'|xargs -i sh -c \"tail -n 500 {}|grep 'final fraction of events with negative weights' || echo {} \""),"\n");
  double sumwx=0,sumw=0;
  for(const auto& line:lines){
    vector<TString> words=Split(line," ");
    if(words.size()!=13){
      cout<<"Unfinished job at "<<line<<endl;
      continue;
    }
    double x=words[10].Atof();
    double err=words[12].Atof();
    if(err>0){
      sumwx+=x/err/err;
      sumw+=1/err/err;
    }
  }
  if(sumw>0) return make_tuple(sumwx/sumw,sqrt(1/sumw));
  else if(sumw==0) return make_tuple(0,0);
  else return make_tuple(-1,-1);
}
double GetNegativeWeight(TString eventdir){
  return get<0>(GetNegativeWeightAndError(eventdir));
}
double GetNegativeWeightError(TString eventdir){
  return get<1>(GetNegativeWeightAndError(eventdir));
}


////////////////////////
/////////OLD/////////////
///////////////////////
void summary_cross_section(){
  TH1::SetDefaultSumw2();
  TH1D* sherpa_cross_section=new TH1D("sherpa_cross_section","Sherpa",24,0,24);
  TH1D* sherpa_sys=new TH1D("sherpa_sys","#mu_{R}/#mu_{F} variation  ",24,0,24);
  TH1D* MG_cross_section=new TH1D("MG_cross_section","MG",24,0,24);

  TH1D* sherpa_negative_fraction=new TH1D("sherpa_negative_fraction","Sherpa",24,0,24);
  TH1D* MG_negative_fraction=new TH1D("MG_negative_fraction","MG",24,0,24);

  map<TString,tuple<TH1D*,int,TString,TH1D*>> binmap;
  binmap["sherpa/ttH_LO0/"]=make_tuple(sherpa_cross_section,2,"0j LO",sherpa_sys);
  binmap["sherpa/ttH_LO1/"]=make_tuple(sherpa_cross_section,3,"0,1j LO",sherpa_sys);
  binmap["sherpa/ttH_LO2/"]=make_tuple(sherpa_cross_section,4,"0,1,2j LO",sherpa_sys);
  binmap["sherpa/ttH_NLO0/"]=make_tuple(sherpa_cross_section,5,"0j NLO",sherpa_sys);
  binmap["sherpa/ttH_NLO1/"]=make_tuple(sherpa_cross_section,6,"0,1j NLO",sherpa_sys);

  binmap["sherpa/ttW_LO0/"]=make_tuple(sherpa_cross_section,10,"0j LO",sherpa_sys);
  binmap["sherpa/ttW_LO1/"]=make_tuple(sherpa_cross_section,11,"0,1j LO",sherpa_sys);
  binmap["sherpa/ttW_LO2/"]=make_tuple(sherpa_cross_section,12,"0,1,2j LO",sherpa_sys);
  binmap["sherpa/ttW_NLO0/"]=make_tuple(sherpa_cross_section,13,"0j NLO",sherpa_sys);
  binmap["sherpa/ttW_NLO1/"]=make_tuple(sherpa_cross_section,14,"0,1j NLO",sherpa_sys);
  binmap["sherpa/ttW/"]=make_tuple(sherpa_cross_section,15,"0,1j NLO + 2j LO",sherpa_sys);

  binmap["sherpa/ttZtoQQorNuNu_LO0/"]=make_tuple(sherpa_cross_section,18,"0j LO",sherpa_sys);
  binmap["sherpa/ttZtoQQorNuNu_LO1/"]=make_tuple(sherpa_cross_section,19,"0,1j LO",sherpa_sys);
  binmap["sherpa/ttZtoQQorNuNu_LO2/"]=make_tuple(sherpa_cross_section,20,"0,1,2j LO",sherpa_sys);
  binmap["sherpa/ttZtoQQorNuNu_NLO0/"]=make_tuple(sherpa_cross_section,21,"0j NLO",sherpa_sys);
  binmap["sherpa/ttZtoQQorNuNu_NLO1/"]=make_tuple(sherpa_cross_section,22,"0,1j NLO",sherpa_sys);
  //binmap["sherpa/ttZtoQQorNuNu/"]=make_tuple(sherpa_cross_section,23,"0,1j NLO + 2j LO",sherpa_sys);

  binmap["MG/TTH_LO0/"]=make_tuple(MG_cross_section,2,"0j LO",(TH1D*)NULL);
  binmap["MG/TTH_LO1/"]=make_tuple(MG_cross_section,3,"0,1j LO",(TH1D*)NULL);
  binmap["MG/TTH_LO2/"]=make_tuple(MG_cross_section,4,"0,1,2j LO",(TH1D*)NULL);
  binmap["MG/TTH_NLO0/"]=make_tuple(MG_cross_section,5,"0j NLO",(TH1D*)NULL);
  binmap["MG/TTH_NLO1/"]=make_tuple(MG_cross_section,6,"0,1j NLO",(TH1D*)NULL);

  binmap["MG/TTW_LO0/"]=make_tuple(MG_cross_section,10,"0j LO",(TH1D*)NULL);
  binmap["MG/TTW_LO1/"]=make_tuple(MG_cross_section,11,"0,1j LO",(TH1D*)NULL);
  binmap["MG/TTW_LO2/"]=make_tuple(MG_cross_section,12,"0,1,2j LO",(TH1D*)NULL);
  binmap["MG/TTW_NLO0/"]=make_tuple(MG_cross_section,13,"0j NLO",(TH1D*)NULL);
  binmap["MG/TTW/"]=make_tuple(MG_cross_section,14,"0,1j NLO",(TH1D*)NULL);

  binmap["MG/TTZtoQQorNuNu_LO0/"]=make_tuple(MG_cross_section,18,"0j LO",(TH1D*)NULL);
  binmap["MG/TTZtoQQorNuNu_LO1/"]=make_tuple(MG_cross_section,19,"0,1j LO",(TH1D*)NULL);
  binmap["MG/TTZtoQQorNuNu_LO2/"]=make_tuple(MG_cross_section,20,"0,1,2j LO",(TH1D*)NULL);
  binmap["MG/TTZtoQQorNuNu_NLO0/"]=make_tuple(MG_cross_section,21,"0j NLO",(TH1D*)NULL);
  binmap["MG/TTZtoQQorNuNu/"]=make_tuple(MG_cross_section,22,"0,1j NLO",(TH1D*)NULL);

  for(const auto& bin:binmap){
    TH1D* hist=get<0>(bin.second);
    hist->GetXaxis()->SetBinLabel(get<1>(bin.second),get<2>(bin.second));
    sherpa_negative_fraction->GetXaxis()->SetBinLabel(get<1>(bin.second),get<2>(bin.second));
  }

  vector<TString> generators={"sherpa","MG"};
  for(const auto& generator:generators){
    for(const auto& dir:Split(gSystem->GetFromPipe("ls -1d "+generator+"/*/"),"\n")){
      cout<<dir;
      double xsum=0;
      double ex2sum=0;
      double fsum=0;
      int n=0;
      for(const auto& run:Split(gSystem->GetFromPipe("ls -1d "+dir+"*/"),"\n")){
	vector<TString> linex=Split(gSystem->GetFromPipe("tail -n 50 "+run+"STDIN.e*|grep \"final cross section\"|xargs -i echo {}")," ");
	if(linex.size()!=10){
	  cout<<" still running? "<<run<<endl;
	  break;
	}
	xsum+=linex[6].Atof();
	ex2sum+=linex[8].Atof();
	n++;
	vector<TString> linef=Split(gSystem->GetFromPipe("tail -n 50 "+run+"STDIN.e*|grep \"final fraction of events\"|xargs -i echo {}")," ");
	fsum+=linef[10].Atof();
      }
      double result=xsum/n;
      double staterr=sqrt(ex2sum)/n;
      double syserr=0;
      double sumw=0;
      double nfrac=-1;
      if(gSystem->Exec("ls "+dir+"|grep hists.root > /dev/null")==0){
	TFile f(dir+"/hists.root");
	TH1* hist=(TH1*)f.Get("sumw");
	if(hist){
	  double maxdiff=0;
	  double norm=hist->GetBinContent(1);
	  sumw=norm;
	  for(int i=5;i<12;i++){
	    double diff=fabs(hist->GetBinContent(i)-norm);
	    if(diff==norm) continue;
	    if(diff>maxdiff){
	      maxdiff=diff;
	    }
	  }
	  syserr=result*maxdiff/norm;
	}
      }
      cout<<" Xsec: "<<result<<" +- "<<staterr<<" +- "<<syserr<<" negative fraction: "<<nfrac<<" sumw: "<<sumw<<endl;
      if(isfinite(result)&&result!=0){
	if(get<0>(binmap[dir])){
	  get<0>(binmap[dir])->SetBinContent(get<1>(binmap[dir]),result);
	  get<0>(binmap[dir])->SetBinError(get<1>(binmap[dir]),staterr);
	}	  
	if(get<3>(binmap[dir])){
	  get<3>(binmap[dir])->SetBinContent(get<1>(binmap[dir]),result);
	  get<3>(binmap[dir])->SetBinError(get<1>(binmap[dir]),syserr);
	}
	nfrac=fsum/n;
      }
      if(generator.Contains("sherpa")){
	sherpa_negative_fraction->SetBinContent(get<1>(binmap[dir]),nfrac);
      }else{
	MG_negative_fraction->SetBinContent(get<1>(binmap[dir]),nfrac);
      }	  
    }
  }
  TCanvas* c=new TCanvas;
  c->SetBottomMargin(0.3);
  c->SetGridx();
  sherpa_cross_section->GetXaxis()->LabelsOption("v");
  sherpa_cross_section->GetXaxis()->SetLabelSize(0.05);
  sherpa_cross_section->GetYaxis()->SetTitle("cross section (pb)");
  sherpa_cross_section->GetYaxis()->SetRangeUser(0,1);
  sherpa_cross_section->SetMarkerStyle(20);
  sherpa_cross_section->SetMarkerSize(0.7);
  sherpa_cross_section->SetMarkerColor(2);
  sherpa_cross_section->SetLineColor(2);
  sherpa_cross_section->SetLineWidth(2);
  sherpa_cross_section->SetStats(0);
  sherpa_cross_section->SetTitle("Cross section");
  sherpa_cross_section->Draw("p e");
  sherpa_sys->SetFillColor(2);
  sherpa_sys->SetFillStyle(3001);
  sherpa_sys->Draw("same e2");
  MG_cross_section->SetMarkerStyle(20);
  MG_cross_section->SetMarkerSize(0.7);
  MG_cross_section->SetMarkerColor(4);
  MG_cross_section->SetLineWidth(2);
  MG_cross_section->SetLineColor(4);
  MG_cross_section->Draw("same p e");
  TLine *line0=new TLine;
  line0->SetLineWidth(2);
  line0->DrawLineNDC(0.1+0.8/3,0.15,0.1+0.8/3,0.9);
  line0->DrawLineNDC(0.1+1.6/3,0.15,0.1+1.6/3,0.9);
  TText *text=new TText;
  text->DrawTextNDC(0.2,0.85,"ttH");
  text->DrawTextNDC(0.47,0.85,"ttW");
  text->DrawTextNDC(0.65,0.85,"ttZtoQQorNuNu");
  TLegend* legend=new TLegend(0.12,0.7,0.35,0.5);
  legend->AddEntry(sherpa_cross_section,"Sherpa","lp");
  legend->AddEntry(sherpa_sys,"","f");
  legend->AddEntry(MG_cross_section,"","lp");
  legend->Draw();

  //////////////////////
  TCanvas* c2=new TCanvas;
  c2->SetBottomMargin(0.3);
  c2->SetGridx();
  sherpa_negative_fraction->GetXaxis()->LabelsOption("v");
  sherpa_negative_fraction->GetXaxis()->SetLabelSize(0.05);
  sherpa_negative_fraction->GetYaxis()->SetTitle("Negative fraction");
  sherpa_negative_fraction->GetYaxis()->SetRangeUser(0,0.5);
  sherpa_negative_fraction->SetMarkerStyle(20);
  sherpa_negative_fraction->SetMarkerSize(1);
  sherpa_negative_fraction->SetMarkerColor(2);
  sherpa_negative_fraction->SetLineColor(2);
  sherpa_negative_fraction->SetLineWidth(2);
  sherpa_negative_fraction->SetStats(0);
  sherpa_negative_fraction->SetTitle("Negative fraction");
  sherpa_negative_fraction->Draw("p e");
  MG_negative_fraction->SetMarkerStyle(25);
  MG_negative_fraction->SetMarkerSize(1.);
  MG_negative_fraction->SetMarkerColor(4);
  MG_negative_fraction->SetLineWidth(2);
  MG_negative_fraction->SetLineColor(4);
  MG_negative_fraction->Draw("same p0");
  line0->SetLineWidth(2);
  line0->DrawLineNDC(0.1+0.8/3,0.15,0.1+0.8/3,0.9);
  line0->DrawLineNDC(0.1+1.6/3,0.15,0.1+1.6/3,0.9);
  text->DrawTextNDC(0.2,0.85,"ttH");
  text->DrawTextNDC(0.47,0.85,"ttW");
  text->DrawTextNDC(0.65,0.85,"ttZtoQQorNuNu");
  TLegend* legend2=new TLegend(0.14,0.65,0.32,0.79);
  legend2->AddEntry(sherpa_negative_fraction,"Sherpa","lp");
  legend2->AddEntry(MG_negative_fraction,"","lp");
  legend2->Draw();
  vector<int> list={1,2,3,4,5,6,7,8,9,10,15,16,17,18,23,24};
  for(const auto& num:list) MG_negative_fraction->SetBinContent(num,-1);
  
}      
void summary_pack_speed_core(){
  TH1::SetDefaultSumw2();
  TH1D* sherpa_pack_speed=new TH1D("sherpa_pack_speed","Sherpa",24,0,24);
  TH1D* MG_pack_speed=new TH1D("MG_pack_speed","MG",24,0,24);
  map<TString,tuple<TH1D*,int,TString>> binmap;
  binmap["ttH_core1"]=make_tuple(sherpa_pack_speed,2,"1");
  binmap["ttH_core2"]=make_tuple(sherpa_pack_speed,3,"2");
  binmap["ttH_core4"]=make_tuple(sherpa_pack_speed,4,"4");
  binmap["ttH_core8"]=make_tuple(sherpa_pack_speed,5,"8");
  binmap["ttH_core16"]=make_tuple(sherpa_pack_speed,6,"16");
  binmap["ttH_core32"]=make_tuple(sherpa_pack_speed,7,"32");

  binmap["ttW_NLO1_core1"]=make_tuple(sherpa_pack_speed,10,"1");
  binmap["ttW_NLO1_core2"]=make_tuple(sherpa_pack_speed,11,"2");
  binmap["ttW_NLO1_core4"]=make_tuple(sherpa_pack_speed,12,"4");
  binmap["ttW_NLO1_core8"]=make_tuple(sherpa_pack_speed,13,"8");
  binmap["ttW_NLO1_core16"]=make_tuple(sherpa_pack_speed,14,"16");
  binmap["ttW_NLO1_core32"]=make_tuple(sherpa_pack_speed,15,"32");

  binmap["ttZtoQQorNuNu_core1"]=make_tuple(sherpa_pack_speed,18,"1");
  binmap["ttZtoQQorNuNu_core2"]=make_tuple(sherpa_pack_speed,19,"2");
  binmap["ttZtoQQorNuNu_core4"]=make_tuple(sherpa_pack_speed,20,"4");
  binmap["ttZtoQQorNuNu_core8"]=make_tuple(sherpa_pack_speed,21,"8");
  binmap["ttZtoQQorNuNu_core16"]=make_tuple(sherpa_pack_speed,22,"16");
  binmap["ttZtoQQorNuNu_core32"]=make_tuple(sherpa_pack_speed,23,"32");

  binmap["TTW_core1"]=make_tuple(MG_pack_speed,10,"1");
  binmap["TTW_core2"]=make_tuple(MG_pack_speed,11,"2");
  binmap["TTW_core4"]=make_tuple(MG_pack_speed,12,"4");
  binmap["TTW_core8"]=make_tuple(MG_pack_speed,13,"8");
  binmap["TTW_core16"]=make_tuple(MG_pack_speed,14,"16");
  binmap["TTW_core32"]=make_tuple(MG_pack_speed,15,"32");

  for(const auto& bin:binmap){
    TH1D* hist=get<0>(bin.second);
    hist->GetXaxis()->SetBinLabel(get<1>(bin.second),get<2>(bin.second));
  }

  vector<TString> generators={"src/GeneratorInterface/SherpaInterface/data/","genproductions/bin/MadGraph5_aMCatNLO/"};
  for(const auto& generator:generators){
    TString dirs=generator.Contains("Sherpa")?
      gSystem->GetFromPipe("ls -1 "+generator+"*core*/*.e*"):
      gSystem->GetFromPipe("ls -1 "+generator+"*core*.e*");
    for(const auto& log:Split(dirs,"\n")){
      double real=gSystem->GetFromPipe("tail "+log+"|grep ^real|sed 's/[^0-9.]/ /g'|awk '{print $1}'").Atof();
      double user=gSystem->GetFromPipe("tail "+log+"|grep ^user|sed 's/[^0-9.]/ /g'|awk '{print $1}'").Atof();
      double sys=gSystem->GetFromPipe("tail "+log+"|grep ^sys|sed 's/[^0-9.]/ /g'|awk '{print $1}'").Atof();
      cout<<log<<" "<<real<<" "<<user+sys<<endl;
      vector<TString> names=Split(log,"/");
      TString key=generator.Contains("Sherpa")?
	names.at(4):
	Split(names.at(3),".").at(0);
      if(isfinite(real)&&real!=0){
	if(get<0>(binmap[key]))
	  get<0>(binmap[key])->SetBinContent(get<1>(binmap[key]),real);
      }
    }
  }
  TCanvas* c=new TCanvas;
  c->SetBottomMargin(0.3);
  c->SetLogy();
  c->SetGridx();
  sherpa_pack_speed->GetXaxis()->SetTitle("nThread");
  sherpa_pack_speed->GetXaxis()->LabelsOption("h");
  sherpa_pack_speed->GetXaxis()->SetLabelSize(0.04);
  sherpa_pack_speed->GetYaxis()->SetTitle("Time (min)");
  sherpa_pack_speed->SetMarkerStyle(20);
  sherpa_pack_speed->SetMarkerColor(2);
  sherpa_pack_speed->SetStats(0);
  sherpa_pack_speed->Draw("p e");
  MG_pack_speed->SetMarkerStyle(25);
  MG_pack_speed->SetMarkerColor(4);
  MG_pack_speed->Draw("same p e");
  TLine *line0=new TLine;
  line0->SetLineWidth(2);
  line0->DrawLineNDC(0.1+0.8/3,0.15,0.1+0.8/3,0.9);
  line0->DrawLineNDC(0.1+1.6/3,0.15,0.1+1.6/3,0.9);

  TLegend* legend=new TLegend(0.15,0.88,0.3,0.7);
  legend->AddEntry(sherpa_pack_speed,"Sherpa","lp");
  legend->AddEntry(MG_pack_speed,"","lp");
  legend->Draw();

  TText *text=new TText;
  text->SetTextSize(0.03);
  text->DrawTextNDC(0.17,0.15,"ttH 0j NLO");
  text->DrawTextNDC(0.44,0.15,"ttW 0,1j NLO");
  text->DrawTextNDC(0.65,0.15,"ttZtoQQorNuNu 0,1j NLO + 2j LO");
  sherpa_pack_speed->SetTitle("Time for gridpack generation");
}      
void summary_gen_speed_order(){
  vector<TString> generators={"src/GeneratorInterface/SherpaInterface/data/","genproductions/bin/MadGraph5_aMCatNLO/"};
  for(const auto& generator:generators){
    TString logs=generator.Contains("Sherpa")?
      gSystem->GetFromPipe("ls -1 "+generator+"/*/*.o*|grep -v core|grep -v _qsub"):
      gSystem->GetFromPipe("ls -1 "+generator+"/*.o*|grep -v core|grep -v _qsub");
    for(const auto& log:Split(logs,"\n")){
      double perday=gSystem->GetFromPipe("tail -n 100 "+log+" |grep -o ' [0-9e+.]* evts/day'").Atof();
      cout<<log<<" "<<perday<<endl;
    }
  }
}      


void summary_gen_speed(){
  TH1::SetDefaultSumw2();
  TH1D* sherpa_gen_speed=new TH1D("sherpa_gen_speed","Sherpa",24,0,24);
  TH1D* MG_gen_speed=new TH1D("MG_gen_speed","MG",24,0,24);
  map<TString,tuple<TH1D*,int,TString>> binmap;
  binmap["sherpa/ttH_LO0/"]=make_tuple(sherpa_gen_speed,2,"0j LO");
  binmap["sherpa/ttH_LO1/"]=make_tuple(sherpa_gen_speed,3,"0,1j LO");
  binmap["sherpa/ttH_LO2/"]=make_tuple(sherpa_gen_speed,4,"0,1,2j LO");
  binmap["sherpa/ttH_NLO0/"]=make_tuple(sherpa_gen_speed,5,"0j NLO");
  binmap["sherpa/ttH_NLO1/"]=make_tuple(sherpa_gen_speed,6,"0,1j NLO");

  binmap["sherpa/ttW_LO0/"]=make_tuple(sherpa_gen_speed,10,"0j LO");
  binmap["sherpa/ttW_LO1/"]=make_tuple(sherpa_gen_speed,11,"0,1j LO");
  binmap["sherpa/ttW_LO2/"]=make_tuple(sherpa_gen_speed,12,"0,1,2j LO");
  binmap["sherpa/ttW_NLO0/"]=make_tuple(sherpa_gen_speed,13,"0j NLO");
  binmap["sherpa/ttW_NLO1/"]=make_tuple(sherpa_gen_speed,14,"0,1j NLO");
  binmap["sherpa/ttW/"]=make_tuple(sherpa_gen_speed,15,"0,1j NLO + 2j LO");

  binmap["sherpa/ttZtoQQorNuNu_LO0/"]=make_tuple(sherpa_gen_speed,18,"0j LO");
  binmap["sherpa/ttZtoQQorNuNu_LO1/"]=make_tuple(sherpa_gen_speed,19,"0,1j LO");
  binmap["sherpa/ttZtoQQorNuNu_LO2/"]=make_tuple(sherpa_gen_speed,20,"0,1,2j LO");
  binmap["sherpa/ttZtoQQorNuNu_NLO0/"]=make_tuple(sherpa_gen_speed,21,"0j NLO");
  binmap["sherpa/ttZtoQQorNuNu_NLO1/"]=make_tuple(sherpa_gen_speed,22,"0,1j NLO");
  binmap["sherpa/ttZtoQQorNuNu/"]=make_tuple(sherpa_gen_speed,23,"0,1j NLO + 2j LO");

  binmap["MG/TTH_LO0/"]=make_tuple(MG_gen_speed,2,"0j LO");
  binmap["MG/TTH_LO1/"]=make_tuple(MG_gen_speed,3,"0,1j LO");
  binmap["MG/TTH_LO2/"]=make_tuple(MG_gen_speed,4,"0,1,2j LO");
  binmap["MG/TTH_NLO0/"]=make_tuple(MG_gen_speed,5,"0j NLO");
  binmap["MG/TTH_NLO1/"]=make_tuple(MG_gen_speed,6,"0,1j NLO");

  binmap["MG/TTW_LO0/"]=make_tuple(MG_gen_speed,10,"0j LO");
  binmap["MG/TTW_LO1/"]=make_tuple(MG_gen_speed,11,"0,1j LO");
  binmap["MG/TTW_LO2/"]=make_tuple(MG_gen_speed,12,"0,1,2j LO");
  binmap["MG/TTW_NLO0/"]=make_tuple(MG_gen_speed,13,"0j NLO");
  binmap["MG/TTW/"]=make_tuple(MG_gen_speed,14,"0,1j NLO");

  binmap["MG/TTZtoQQorNuNu_LO0/"]=make_tuple(MG_gen_speed,18,"0j LO");
  binmap["MG/TTZtoQQorNuNu_LO1/"]=make_tuple(MG_gen_speed,19,"0,1j LO");
  binmap["MG/TTZtoQQorNuNu_LO2/"]=make_tuple(MG_gen_speed,20,"0,1,2j LO");
  binmap["MG/TTZtoQQorNuNu_NLO0/"]=make_tuple(MG_gen_speed,21,"0j NLO");
  binmap["MG/TTZtoQQorNuNu/"]=make_tuple(MG_gen_speed,22,"0,1j NLO");

  for(const auto& bin:binmap){
    TH1D* hist=get<0>(bin.second);
    hist->GetXaxis()->SetBinLabel(get<1>(bin.second),get<2>(bin.second));
  }

  vector<TString> generators={"sherpa","MG"};
  for(const auto& generator:generators){
    for(const auto& dir:Split(gSystem->GetFromPipe("ls -1d "+generator+"/*/|grep -v core"),"\n")){
      cout<<dir<<" ";
      double sum=0;
      int n=0;
      for(const auto& log:Split(gSystem->GetFromPipe("ls -1d "+dir+"/run*/STDIN.o*"),"\n")){
	double perday=gSystem->GetFromPipe("tail -n 150 "+log+" |grep -o ' [0-9e+.]* evts/day' || echo 0").Atof();
	if(perday==0){
	  TString errorlog=log; errorlog.ReplaceAll("STDIN.o","STDIN.e");
	  double time=gSystem->GetFromPipe("tail -n 100 "+errorlog+"|grep ^user|sed 's/[^0-9.]/ /g'|awk '{print $1}' || echo 0").Atof()+gSystem->GetFromPipe("tail -n 100 "+errorlog+"|grep ^sys|sed 's/[^0-9.]/ /g'|awk '{print $1}' || echo 0").Atof();
	  int nevent=gSystem->GetFromPipe("cat "+errorlog+"|grep '^Begin processing '|wc -l").Atoi();
	  //cout<<time<<" "<<nevent<<" wow"<<endl;
	  perday=nevent/time*3600;
	}
	if(perday==0||perday!=perday){
	  cout<<" still running? end at "<<log<<" ";//<<gSystem->GetFromPipe("tail -n 100 "+log+" |grep -o ' [0-9e+.]* evts/day'")<<endl;
	  break;
	}
	sum+=perday;
	n++;
      }
      double result=sum/n;
      cout<<result<<" "<<n<<endl;
      if(isfinite(result)&&result!=0){
	if(get<0>(binmap[dir]))
	  get<0>(binmap[dir])->SetBinContent(get<1>(binmap[dir]),result);
      }
    }
  }
  TCanvas* c=new TCanvas;
  c->SetBottomMargin(0.3);
  c->SetLogy();
  c->SetGridx();
  sherpa_gen_speed->GetXaxis()->LabelsOption("v");
  sherpa_gen_speed->GetXaxis()->SetLabelSize(0.05);
  sherpa_gen_speed->SetMaximum(1.4e7);
  sherpa_gen_speed->GetYaxis()->SetTitle("events/day");
  sherpa_gen_speed->SetMarkerStyle(20);
  sherpa_gen_speed->SetMarkerColor(2);
  sherpa_gen_speed->SetStats(0);
  sherpa_gen_speed->Draw("p e");
  MG_gen_speed->SetMarkerStyle(25);
  MG_gen_speed->SetMarkerColor(4);
  MG_gen_speed->Draw("same p e");
  TLine *line0=new TLine;
  line0->SetLineWidth(2);
  line0->DrawLineNDC(0.1+0.8/3,0.15,0.1+0.8/3,0.9);
  line0->DrawLineNDC(0.1+1.6/3,0.15,0.1+1.6/3,0.9);

  TText *text=new TText;
  text->DrawTextNDC(0.2,0.85,"ttH");
  text->DrawTextNDC(0.47,0.85,"ttW");
  text->DrawTextNDC(0.65,0.85,"ttZtoQQorNuNu");
  TLegend* legend=new TLegend(0.38,0.46,0.53,0.57);
  legend->AddEntry(sherpa_gen_speed,"Sherpa","lp");
  legend->AddEntry(MG_gen_speed,"","lp");
  legend->Draw();
  sherpa_gen_speed->SetTitle("Event generation per day");
}      
