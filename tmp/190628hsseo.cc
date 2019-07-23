#include"../script/util.cc"
TCanvas* GetCanvas(TString option){ //option: genspeed(S), crosssection(X), negativeweight(N)
  vector<TString> generator={"Sherpa","MG"};
  vector<TString> process={"ttH","ttW","ttZtoQQorNuNu","ttll"};
  vector<TString> order={"_LO0","_LO1","_LO2","_NLO0","_NLO1",""};
  vector<TString> order_title={"0j LO","01j LO","012j LO","0j NLO","01j NLO","01j NLO + 2j LO"};
  TH1D* hist[2]={new TH1D("Sherpa","Sherpa",process.size()*order.size(),0,process.size()*order.size()),new TH1D("MG","MG",process.size()*order.size(),0,process.size()*order.size())};
  TH1D* sys=new TH1D("#mu_{R}/#mu_{F}","#mu_{R}/#mu_{F}",process.size()*order.size(),0,process.size()*order.size());
  for(int ig=0;ig<generator.size();ig++){
    for(int ip=0;ip<process.size();ip++){
      for(int io=0;io<order.size();io++){
	if(generator[ig]=="MG"&&order[io]=="") continue;
	double x,err;
	TString eventdir=generator[ig]+"/Event/"+process[ip]+order[io];
	if(option=="genspeed"||option=="S"){
	  auto tu=GetEventGenerationSpeedAndError(eventdir);
	  x=get<0>(tu);
	  err=get<1>(tu);
	}else if(option=="crosssection"||option=="X"){
	  auto tu=GetCrossSectionAndStatError(eventdir);
	  x=get<0>(tu);
	  err=get<1>(tu);
	  if(generator[ig]=="Sherpa"){
	    sys->SetBinContent(ip*order.size()+io+1,x);
	    sys->SetBinError(ip*order.size()+io+1,GetCrossSectionSysError(eventdir));
	  }
	}else if(option=="negativeweight"||option=="N"){
	  auto tu=GetNegativeWeightAndError(eventdir);
	  x=get<0>(tu);
	  err=get<1>(tu);
	}
	cout<<generator[ig]<<" "<<process[ip]<<" "<<order[io]<<" "<<x<<" "<<err<<endl;
	hist[ig]->SetBinContent(ip*order.size()+io+1,x);
	hist[ig]->SetBinError(ip*order.size()+io+1,err);
	hist[ig]->GetXaxis()->SetBinLabel(ip*order.size()+io+1,order_title[io]);
      }
    }
  }
  TCanvas* c=new TCanvas;
  c->SetBottomMargin(0.25);
  double ymin=10, ymax=5000000;

  hist[0]->Draw("p");
  hist[0]->SetStats(0);
  hist[0]->SetMarkerStyle(20);
  hist[0]->SetMarkerColor(2);

  TLegend* leg=new TLegend(0.72,0.35,0.88,0.48);
  leg->AddEntry(hist[0]);
  leg->AddEntry(hist[1]);
  leg->Draw();

  if(option=="genspeed"||option=="S"){
    ymin=10; ymax=5000000;
    hist[0]->GetYaxis()->SetTitle("Events/day");
    hist[0]->SetTitle("Event generation per day");
    c->SetLogy();
  }else if(option=="crosssection"||option=="X"){
    ymin=0; ymax=1.1;
    hist[0]->GetYaxis()->SetTitle("Cross section");
    hist[0]->SetTitle("Cross section");
    sys->Draw("same e2");
    sys->SetFillColor(6);
    sys->SetFillStyle(3002);
    leg->AddEntry(sys);
  }else if(option=="negativeweight"||option=="N"){
    ymin=0; ymax=0.4;
    hist[0]->GetYaxis()->SetTitle("fraction");
    hist[0]->SetTitle("Negative weight fraction");
  }
  hist[0]->GetYaxis()->SetRangeUser(ymin,ymax);
  c->SetGridx();
  hist[0]->GetXaxis()->SetNdivisions(process.size());
  hist[0]->GetXaxis()->LabelsOption("v");
  hist[1]->Draw("same p");
  hist[1]->SetMarkerStyle(20);
  hist[1]->SetMarkerColor(4);

  TLine *line=new TLine;
  line->SetLineWidth(2);
  for(int ip=1;ip<process.size();ip++) line->DrawLine(ip*order.size(),ymin,ip*order.size(),ymax);

  TText *text=new TText;
  text->SetTextSize(0.03);
  text->DrawTextNDC(0.15,0.3,"ttH");
  text->DrawTextNDC(0.35,0.3,"ttW");
  text->DrawTextNDC(0.55,0.3,"ttZtoQQorNuNu");
  text->DrawTextNDC(0.78,0.3,"ttll");

  return c;
}	
