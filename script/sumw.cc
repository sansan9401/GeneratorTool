#include "TH1D.h"
#include "TFile.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"
#include "DataFormats/JetReco/interface/GenJet.h"
#include "DataFormats/FWLite/interface/Handle.h"
#include "DataFormats/FWLite/interface/Event.h"
#include "SimDataFormats/GeneratorProducts/interface/GenEventInfoProduct.h"
map<TString,TH1D*> hists;
void FillHist(TString histname, double value, double weight, int n_bin, double x_min, double x_max){
  auto it = hists.find(histname);
  TH1D* hist=NULL;
  if( it==hists.end() ){
    hist = new TH1D(histname, "", n_bin, x_min, x_max);
    hists[histname] = hist;
  }else hist=it->second;
  hist->Fill(value, weight);  
}
void loop(TString infile,TString outfile){
  TFile f(infile);
  fwlite::Event ev(&f);
  int ievent=0;
  for(ev.toBegin();!ev.atEnd();++ev){
    if(ievent%1000==0) cout<<ievent<<endl;

    fwlite::Handle<GenEventInfoProduct> geninfo;
    geninfo.getByLabel(ev,"generator");
    const vector<double>& weights=geninfo.ptr()->weights();

    for(int i=0;i<weights.size();i++){
      FillHist("sumw",i,weights[i],200,0,200);
    }
    ievent++;
  }
  
  TFile fout(outfile,"recreate");
  for(const auto& [histname,hist]:hists){
    hist->Write();
  }
  fout.Close();
}
