#include "TLorentzVector.h"
#include "TH1D.h"
#include "TFile.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"
#include "DataFormats/JetReco/interface/GenJet.h"
#include "DataFormats/FWLite/interface/Handle.h"
#include "DataFormats/FWLite/interface/Event.h"
#include "SimDataFormats/GeneratorProducts/interface/GenEventInfoProduct.h"
map<TString,TH1D*> hists;
TLorentzVector MakeTLorentzVector(reco::LeafCandidate* particle){
  TLorentzVector vec;
  if(particle) vec.SetPxPyPzE(particle->px(),particle->py(),particle->pz(),particle->energy());
  return vec;
}
TLorentzVector MakeTLorentzVector(const reco::GenParticle* particle){
  return MakeTLorentzVector((reco::LeafCandidate*)particle);
}
TLorentzVector MakeTLorentzVector(reco::GenJet* particle){
  return MakeTLorentzVector((reco::LeafCandidate*)particle);
}
TLorentzVector MakeTLorentzVector(tuple<reco::GenJet*,reco::GenJet*> particles){
  return MakeTLorentzVector(get<0>(particles))+MakeTLorentzVector(get<1>(particles));
}
TLorentzVector MakeTLorentzVector(vector<reco::GenParticle*> particles){
  TLorentzVector out;
  for(const auto& particle:particles) out+=MakeTLorentzVector(particle);
  return out;
}
void FillHist(TString histname, double value, double weight, int n_bin, double x_min, double x_max){
  auto it = hists.find(histname);
  TH1D* hist=NULL;
  if( it==hists.end() ){
    hist = new TH1D(histname, "", n_bin, x_min, x_max);
    hists[histname] = hist;
  }else hist=it->second;
  hist->Fill(value, weight);  
}
void PrintGen(const reco::GenParticle& gen){
  cout<<&gen<<" "<<gen.pdgId()<<" "<<gen.status()<<" "<<gen.mother()<<"\t"<<gen.isHardProcess()<<gen.isLastCopy()<<gen.isLastCopyBeforeFSR()<<gen.isPromptDecayed()<<gen.isPromptFinalState()<<endl;
}
void PrintGen(const reco::GenParticle* gen){
  if(gen) PrintGen(*gen);
  else cout<<"[PrintGen] NULL pointer"<<endl;
}
void PrintGens(const vector<reco::GenParticle>& gens){
  for(unsigned int i=0;i<gens.size();i++){
    //if(gens[i].isHardProcess())  cout<<i<<" "<<&gens[i]<<" "<<gens[i].pdgId()<<" "<<gens[i].status()<<" "<<gens[i].mother()<<"\t"<<gens[i].energy()<<" "<<gens[i].p()<<" "<<gens[i].eta()<<" "<<gens[i].phi()<<endl;                                                                                 
    cout<<i<<" ";
    PrintGen(gens[i]);
  }
}
void PrintGens(const vector<reco::GenParticle*>& gens){
  for(unsigned int i=0;i<gens.size();i++){
    cout<<i<<" ";
    PrintGen(gens[i]);
  }
}

void loop(TString infile,TString outfile){
  //cout << "Loading FW Lite setup." << endl;
  //gSystem->Load("libFWCoreFWLite.so");
  //FWLiteEnabler::enable();
  //gSystem->Load("libDataFormatsFWLite.so");
  //gSystem->Load("libDataFormatsPatCandidates.so");

  TFile f(infile);
  fwlite::Event ev(&f);
  int ievent=0;
  for(ev.toBegin();!ev.atEnd();++ev){
    //if(ievent>3) break;
    if(ievent%1000==0) cout<<ievent<<endl;
    fwlite::Handle<std::vector<reco::GenParticle>> gens_;
    gens_.getByLabel(ev,"genParticles");
    const vector<reco::GenParticle>& gens=*gens_.ptr();

    fwlite::Handle<std::vector<reco::GenJet>> jets_;
    jets_.getByLabel(ev,"ak4GenJets");
    const vector<reco::GenJet>& jets_all=*jets_.ptr();
    vector<reco::GenJet*> jets;
    for(const auto& jet:jets_all){
      if(jet.pt()>30) jets.push_back((reco::GenJet*)&jet);
    }

    fwlite::Handle<GenEventInfoProduct> geninfo;
    geninfo.getByLabel(ev,"generator");
    const vector<double>& weights=geninfo.ptr()->weights();

    for(int i=0;i<weights.size();i++){
      FillHist("sumw",i,weights[i],200,0,200);
    }

    const reco::GenParticle *hard_Wp=NULL,*hard_Wm=NULL;
    const reco::GenParticle *last_Wp=NULL,*last_Wm=NULL;
    const reco::GenParticle *HD_l=NULL,*HD_lbar=NULL,*HD_nu=NULL,*HD_nubar=NULL;
    const reco::GenParticle *l=NULL,*lbar=NULL,*nu=NULL,*nubar=NULL;
    vector<const reco::GenParticle*> photons;
    for(int i=0;i<gens.size();i++){
      if(gens[i].isHardProcess()){
	if(gens[i].pdgId()==24) hard_Wp=&gens[i];
	else if(gens[i].pdgId()==-24) hard_Wm=&gens[i];
      }
      if(gens[i].pdgId()==24) last_Wp=&gens[i];
      else if(gens[i].pdgId()==-24) last_Wm=&gens[i];

      if((gens[i].pdgId()==11||gens[i].pdgId()==13)&&gens[i].mother()==last_Wm) HD_l=&gens[i];
      else if((gens[i].pdgId()==12||gens[i].pdgId()==14)&&gens[i].mother()==last_Wp) HD_nu=&gens[i];
      else if((gens[i].pdgId()==-11||gens[i].pdgId()==-13)&&gens[i].mother()==last_Wp) HD_lbar=&gens[i];
      else if((gens[i].pdgId()==-12||gens[i].pdgId()==-14)&&gens[i].mother()==last_Wm) HD_nubar=&gens[i];

      if(gens[i].pdgId()==22&&gens[i].status()==1) photons.push_back(&gens[i]);
      if(gens[i].isPromptFinalState()){
	if(gens[i].pdgId()==11||gens[i].pdgId()==13) l=&gens[i];
	else if(gens[i].pdgId()==12||gens[i].pdgId()==14) nu=&gens[i];
	else if(gens[i].pdgId()==-11||gens[i].pdgId()==-13) lbar=&gens[i];
	else if(gens[i].pdgId()==-12||gens[i].pdgId()==-14) nubar=&gens[i];
      }
    }
    if(0){
      PrintGen(hard_Wp);
      PrintGen(hard_Wm);
      PrintGen(HD_l);
      PrintGen(HD_lbar);
      PrintGen(HD_nu);
      PrintGen(HD_nubar);
      PrintGen(l);
      PrintGen(lbar);
      PrintGen(nu);
      PrintGen(nubar);
      if(!HD_l){
	PrintGens(gens);
	exit(1);
      }
    }

    if(l&&lbar&&nu&&nubar){
      TLorentzVector vec_hard_Wp=MakeTLorentzVector(hard_Wp);
      TLorentzVector vec_hard_Wm=MakeTLorentzVector(hard_Wm);

      TLorentzVector vec_last_Wp=MakeTLorentzVector(last_Wp);
      TLorentzVector vec_last_Wm=MakeTLorentzVector(last_Wm);

      TLorentzVector vec_HD_l=MakeTLorentzVector(HD_l);
      TLorentzVector vec_HD_lbar=MakeTLorentzVector(HD_lbar);
      TLorentzVector vec_HD_nu=MakeTLorentzVector(HD_nu);
      TLorentzVector vec_HD_nubar=MakeTLorentzVector(HD_nubar);
      TLorentzVector vec_HD_Wp=vec_HD_lbar+vec_HD_nu;
      TLorentzVector vec_HD_Wm=vec_HD_l+vec_HD_nubar;

      TLorentzVector vec_l=MakeTLorentzVector(l);
      TLorentzVector vec_lbar=MakeTLorentzVector(lbar);
      TLorentzVector vec_nu=MakeTLorentzVector(nu);
      TLorentzVector vec_nubar=MakeTLorentzVector(nubar);
      TLorentzVector vec_dressed_l=vec_l;
      TLorentzVector vec_dressed_lbar=vec_lbar;

      for(const auto& photon:photons){
	TLorentzVector vec_photon=MakeTLorentzVector(photon);
	double delr_l=vec_photon.DeltaR(vec_l);
	double delr_lbar=vec_photon.DeltaR(vec_lbar);
	if(delr_l<delr_lbar&&delr_l<0.4) vec_dressed_l+=vec_photon;
	else if(delr_l>delr_lbar&&delr_lbar<0.4) vec_dressed_lbar+=vec_photon;
      }	  

      TLorentzVector vec_Wp=vec_lbar+vec_nu;
      TLorentzVector vec_Wm=vec_l+vec_nubar;
      TLorentzVector vec_dressed_Wp=vec_dressed_lbar+vec_nu;
      TLorentzVector vec_dressed_Wm=vec_dressed_l+vec_nubar;
      TLorentzVector vec_WW=vec_Wp+vec_Wm;

      FillHist("WW_m",vec_WW.M(),weights[0],50,30,130);
      FillHist("WW_pt",vec_WW.Pt(),weights[0],50,0,100);
      FillHist("WW_rap",vec_WW.Rapidity(),weights[0],50,-5,5);

      FillHist("hard_W_m",vec_hard_Wp.M(),weights[0],120,50,110);
      FillHist("hard_W_m",vec_hard_Wm.M(),weights[0],120,50,110);

      FillHist("last_W_m",vec_last_Wp.M(),weights[0],120,50,110);
      FillHist("last_W_m",vec_last_Wm.M(),weights[0],120,50,110);

      FillHist("HD_W_m",vec_HD_Wp.M(),weights[0],120,50,110);
      FillHist("HD_W_m",vec_HD_Wm.M(),weights[0],120,50,110);

      FillHist("W_m",vec_Wp.M(),weights[0],120,50,110);
      FillHist("W_m",vec_Wm.M(),weights[0],120,50,110);

      FillHist("dressed_W_m",vec_dressed_Wp.M(),weights[0],120,50,110);
      FillHist("dressed_W_m",vec_dressed_Wm.M(),weights[0],120,50,110);

      FillHist("W_pt",vec_Wp.Pt(),weights[0],50,0,100);
      FillHist("W_pt",vec_Wm.Pt(),weights[0],50,0,100);
      FillHist("W_rap",vec_Wp.Rapidity(),weights[0],50,-5,5);
      FillHist("W_rap",vec_Wm.Rapidity(),weights[0],50,-5,5);

      FillHist("l_pt",vec_l.Pt(),weights[0],50,0,100);
      FillHist("l_pt",vec_lbar.Pt(),weights[0],50,0,100);
      FillHist("nu_pt",vec_nu.Pt(),weights[0],50,0,100);
      FillHist("nu_pt",vec_nubar.Pt(),weights[0],50,0,100);
      FillHist("l_eta",vec_l.Eta(),weights[0],50,-5,5);
      FillHist("l_eta",vec_lbar.Eta(),weights[0],50,-5,5);
      FillHist("nu_eta",vec_nu.Eta(),weights[0],50,-5,5);
      FillHist("nu_eta",vec_nubar.Eta(),weights[0],50,-5,5);
    }else{
      cout<<"Something wrong"<<endl;
      PrintGens(gens);
      exit(1);
    }
    ievent++;
  }
  
  TFile fout(outfile,"recreate");
  for(const auto& [histname,hist]:hists){
    hist->Write();
  }
  fout.Close();
}
