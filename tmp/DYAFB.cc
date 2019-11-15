TH1* GetAFB(TH1* forward,TH1* backward){
  TH1* hist=(TH1*)forward->Clone("AFB");
  hist->Reset();
  for(int i=0;i<hist->GetNbinsX()+2;i++){
    double valf=forward->GetBinContent(i);
    double ef=forward->GetBinError(i);
    double valb=backward->GetBinContent(i);
    double eb=backward->GetBinError(i);
    double val=(valf-valb)/(valf+valb);
    double err=2*sqrt(ef*ef*valb*valb+eb*eb*valf*valf)/pow(valf+valb,2);
    if(val==val&&err==err){
      hist->SetBinContent(i,val);
      hist->SetBinError(i,err);
    }
  }
  hist->SetDirectory(0);
  return hist;
}

TCanvas* GetCanvas(){
  TFile f0("MG/Event/DY_NLO0/hists.root");
  TFile f1("MG/Event/DY_NLO0_aew127/hists.root");
  TFile f2("MG/Event/DY_NLO0_aew137/hists.root");
  TH1* hist0=GetAFB((TH1*)f0.Get("forward"),(TH1*)f0.Get("backward"));
  hist0->SetLineColor(2);
  hist0->SetLineWidth(2);
  hist0->GetXaxis()->SetRangeUser(60,120);
  hist0->GetXaxis()->SetTitle("m(ll) (GeV)");
  hist0->GetXaxis()->SetTitleSize(0.05);
  hist0->GetXaxis()->SetTitleOffset(0.8);
  hist0->GetYaxis()->SetTitle("A_{FB}");
  hist0->GetYaxis()->SetTitleSize(0.05);
  hist0->GetYaxis()->SetTitleOffset(0.8);
  hist0->SetTitle("");
  hist0->SetStats(0);
  TH1* hist1=GetAFB((TH1*)f1.Get("forward"),(TH1*)f1.Get("backward"));
  hist1->SetLineColor(3);
  hist1->SetLineWidth(2);
  TH1* hist2=GetAFB((TH1*)f2.Get("forward"),(TH1*)f2.Get("backward"));
  hist2->SetLineColor(4);
  hist2->SetLineWidth(2);
  TCanvas* c=new TCanvas;
  hist0->Draw("e hist");
  hist1->Draw("same e hist");
  hist2->Draw("same e hist");
  TLegend* leg=new TLegend(0.12,0.88,0.50,0.65);
  leg->AddEntry(hist0,"sin^{2}#theta_{W} = 0.22225 (DEFAULT) ");
  leg->AddEntry(hist1,"sin^{2}#theta_{W} = 0.23608  ");
  leg->AddEntry(hist2,"sin^{2}#theta_{W} = 0.21222  ");
  leg->Draw();
  return c;
}

TCanvas* GetCanvas2(){
  TFile f0("MG/Event/DY_NLO0/hists.root");
  TFile f1("MG/Event/DY_NLO0_aew127/hists.root");
  TFile f2("MG/Event/DY_NLO0_aew137/hists.root");
  TH1* hist0=(TH1*)f0.Get("forward");
  hist0->Add((TH1*)f0.Get("backward"));
  hist0->SetDirectory(0);
  hist0->SetLineColor(2);
  hist0->SetLineWidth(2);
  hist0->GetXaxis()->SetRangeUser(60,120);
  hist0->GetXaxis()->SetTitle("m(ll) (GeV)");
  hist0->GetXaxis()->SetTitleSize(0.05);
  hist0->GetXaxis()->SetTitleOffset(0.8);
  hist0->GetYaxis()->SetTitle("A_{FB}");
  hist0->GetYaxis()->SetTitleSize(0.05);
  hist0->GetYaxis()->SetTitleOffset(0.8);
  hist0->SetTitle("");
  hist0->SetStats(0);
  TH1* hist1=(TH1*)f1.Get("forward");
  hist1->Add((TH1*)f1.Get("backward"));
  hist1->SetDirectory(0);
  hist1->SetLineColor(3);
  hist1->SetLineWidth(2);
  TH1* hist2=(TH1*)f2.Get("forward");
  hist2->Add((TH1*)f2.Get("backward"));
  hist2->SetDirectory(0);
  hist2->SetLineColor(4);
  hist2->SetLineWidth(2);
  TCanvas* c=new TCanvas;
  hist0->Draw("e hist");
  hist1->Draw("same e hist");
  hist2->Draw("same e hist");
  TLegend* leg=new TLegend(0.12,0.88,0.50,0.65);
  leg->AddEntry(hist0,"sin^{2}#theta_{W} = 0.22225 (DEFAULT) ");
  leg->AddEntry(hist1,"sin^{2}#theta_{W} = 0.23608  ");
  leg->AddEntry(hist2,"sin^{2}#theta_{W} = 0.21222  ");
  leg->Draw();
  return c;
}

double GetSin2theta(double aew){
  double mz=91.188;
  double gf=1.16639e-5;
  double mw=sqrt(mz*mz/2+sqrt(pow(mz,4)/4-3.141592*aew*mz*mz/sqrt(2)/gf));
  return 1-mw*mw/mz/mz;
}
