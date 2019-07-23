#include "DataFormats/FWLite/interface/Handle.h"
TLorentzVector MakeTLorentzVector(reco::LeafCandidate* particle){
  TLorentzVector vec;
  if(particle) vec.SetPxPyPzE(particle->px(),particle->py(),particle->pz(),particle->energy());
  return vec;
}
TLorentzVector MakeTLorentzVector(reco::GenParticle* particle){
  return MakeTLorentzVector((reco::LeafCandidate*)particle);
}
TLorentzVector MakeTLorentzVector(reco::GenJet* particle){
  return MakeTLorentzVector((reco::LeafCandidate*)particle);
}
TLorentzVector MakeTLorentzVector(tuple<reco::GenJet*,reco::GenJet*> particles){
  return MakeTLorentzVector(get<0>(particles))+MakeTLorentzVector(get<1>(particles));
}

reco::GenParticle* FindHard(const vector<reco::GenParticle>& gens,int pdgid,bool absid=false){
  for(const auto& gen:gens){
    if(gen.isHardProcess())
      if((absid?abs(gen.pdgId()):gen.pdgId())==pdgid) return (reco::GenParticle*)&gen;
  }
  cout<<"[FindHard] no particle in hard gen wiht "<<pdgid<<endl;
  return NULL;
}    
reco::GenParticle* FindLastCopy(const vector<reco::GenParticle>& gens,reco::GenParticle *ancestor){
  reco::GenParticle* last=ancestor;
  for(const auto& gen:gens){
    if(gen.mother()==last&&gen.pdgId()==ancestor->pdgId()){
      last=(reco::GenParticle*)&gen;
    }
  }
  return last;
}
      
vector<reco::GenParticle*> FindDaughters(const vector<reco::GenParticle>& gens,reco::GenParticle *mother,int pdgid){
  vector<reco::GenParticle*> daughters;
  for(const auto& gen:gens){
    if((reco::GenParticle*)gen.mother()==mother)
      if(!pdgid||gen.pdgId()==pdgid)
	daughters.push_back((reco::GenParticle*)&gen);
  }
  if(daughters.size()==0)
    cout<<"[FindDaughters] no particle in daughers wiht "<<pdgid<<endl;
  return daughters;
}
reco::GenParticle* FindDaughter(const vector<reco::GenParticle>& gens,reco::GenParticle *mother,int pdgid){
  vector<reco::GenParticle*> daughters=FindDaughters(gens,mother,pdgid);
  if(daughters.size()==1) return daughters[0];
  else cout<<"[FindDaughter] "<<daughters.size()<<" particles in daughers with "<<pdgid<<endl;
  return NULL;
}
void RemoveJet(vector<reco::GenJet*>& jets,reco::GenJet* target){
  for(auto it=jets.begin();it!=jets.end();it++){
    if(*it==target){
      jets.erase(it);
      break;
    }
  }
}
reco::GenJet* FindJet(const vector<reco::GenJet*>& jets,reco::GenParticle* gen){
  vector<reco::GenJet*> candidates;
  for(const auto& jet:jets){
    double deta=jet->eta()-gen->eta();
    double dphi=TVector2::Phi_mpi_pi(jet->phi()-gen->phi());
    double dR=sqrt(deta*deta+dphi*dphi);
    double pratio=jet->p()/gen->p();
    if(dR<0.4){
      candidates.push_back(jet);
    }
  }
  reco::GenJet* final=NULL;
  if(candidates.size()>0){
    double min=10000;
    for(const auto& candidate:candidates){
      double diff=fabs(candidate->p()-gen->p());
      if(diff<min){
	min=diff;
	final=candidate;
      }      
    }
  }else{
    cout<<"[FindJet] no matching jet"<<endl;
  }    
  return final;
}
tuple<reco::GenJet*,reco::GenJet*> FindBosonJets(const vector<reco::GenJet*>& jets,reco::GenParticle* mother){
  vector<tuple<reco::GenJet*,reco::GenJet*>> candidates;
  for(int i=0;i<jets.size();i++){
    for(int j=i+1;j<jets.size();j++){
      auto sum=jets[i]->p4()+jets[j]->p4();
      double deta=sum.Rapidity()-mother->rapidity();
      double dphi=TVector2::Phi_mpi_pi(sum.phi()-mother->phi());
      double dR=sqrt(deta*deta+dphi*dphi);
      double pratio=sum.P()/mother->p();
      if(dR<0.4){
	candidates.push_back(make_tuple((reco::GenJet*)jets[i],(reco::GenJet*)jets[j]));
      }
    }
  }
  tuple<reco::GenJet*,reco::GenJet*> final=make_tuple((reco::GenJet*)NULL,(reco::GenJet*)NULL);
  if(candidates.size()>0){
    //cout<<mother->energy()<<" "<<mother->mass()<<" "<<mother->eta()<<" "<<mother->phi()<<endl;
    double min=1000;
    for(const auto& candidate:candidates){
      auto sum=get<0>(candidate)->p4()+get<1>(candidate)->p4();
      if(fabs(sum.M()-mother->mass())<min){
	min=fabs(sum.M()-mother->mass());
	final=candidate;
	//cout<<sum.E()<<" "<<sum.M()<<" "<<sum.Eta()<<" "<<sum.Phi()<<endl;
      }
    }
    //cout<<"[FindBosonJets] more than one matching jet pairs"<<endl;    
  }else{
    cout<<"[FindBosonJets] no matching jets"<<endl;
  }    
  return final;
}
tuple<reco::GenJet*,reco::GenJet*> FindWJets(const vector<reco::GenJet*>& jets,reco::GenParticle* mother){
  vector<tuple<reco::GenJet*,reco::GenJet*>> candidates;
  for(int i=0;i<jets.size();i++){
    for(int j=i+1;j<jets.size();j++){
      auto sum=jets[i]->p4()+jets[j]->p4();
      double deta=sum.Rapidity()-mother->rapidity();
      double dphi=TVector2::Phi_mpi_pi(sum.phi()-mother->phi());
      double dR=sqrt(deta*deta+dphi*dphi);
      double pratio=sum.P()/mother->p();
      if(dR<0.4){
	candidates.push_back(make_tuple((reco::GenJet*)jets[i],(reco::GenJet*)jets[j]));
      }
    }
  }
  tuple<reco::GenJet*,reco::GenJet*> final=make_tuple((reco::GenJet*)NULL,(reco::GenJet*)NULL);
  if(candidates.size()>0){
    //cout<<mother->energy()<<" "<<mother->mass()<<" "<<mother->eta()<<" "<<mother->phi()<<endl;
    double min=1000;
    for(const auto& candidate:candidates){
      auto sum=get<0>(candidate)->p4()+get<1>(candidate)->p4();
      if(fabs(sum.M()-mother->mass())<min){
	min=fabs(sum.M()-mother->mass());
	final=candidate;
	//cout<<sum.E()<<" "<<sum.M()<<" "<<sum.Eta()<<" "<<sum.Phi()<<endl;
      }
    }
    //cout<<"[FindBosonJets] more than one matching jet pairs"<<endl;    
  }else{
    cout<<"[FindBosonJets] no matching jets"<<endl;
  }    
  return final;
}


void loop(TString infile,TString outfile){
  cout << "Loading FW Lite setup." << endl;
  gSystem->Load("libFWCoreFWLite.so");
  FWLiteEnabler::enable();
  gSystem->Load("libDataFormatsFWLite.so");
  gSystem->Load("libDataFormatsPatCandidates.so");

  TFile f(infile);
  fwlite::Event ev(&f);
  int ievent=0;
  map<TString,TH1D*> hists_base;
  hists_base["njet"]=new TH1D("njet","njet",20,0,20);
  hists_base["jet_pt"]=new TH1D("jet_pt","jet_pt",60,0,300);
  hists_base["jet_eta"]=new TH1D("jet_eta","jet_eta",20,-5,5);
  hists_base["nbjet"]=new TH1D("nbjet","nbjet",5,0,5);
  hists_base["bjet_pt"]=new TH1D("bjet_pt","bjet_pt",60,0,300);
  hists_base["bjet_eta"]=new TH1D("bjet_eta","bjet_eta",20,-5,5);
  hists_base["W_m"]=new TH1D("W_m","W_m",60,50,110);
  hists_base["W_pt"]=new TH1D("W_pt","W_pt",60,0,300);
  hists_base["W_eta"]=new TH1D("W_eta","W_eta",20,-5,5);
  hists_base["V_m"]=new TH1D("V_m","V_m",60,50,110);
  hists_base["V_pt"]=new TH1D("V_pt","V_pt",60,0,300);
  hists_base["V_eta"]=new TH1D("V_eta","V_eta",20,-5,5);
  hists_base["t_m"]=new TH1D("t_m","t_m",75,100,250); 
  hists_base["t_pt"]=new TH1D("t_pt","t_pt",500,0,2000);
  hists_base["t_eta"]=new TH1D("t_eta","t_eta",20,-5,5);

  hists_base["gen_W_m"]=new TH1D("gen_W_m","gen_W_m",60,50,110);
  hists_base["gen_W_pt"]=new TH1D("gen_W_pt","gen_W_pt",60,0,300);
  hists_base["gen_W_eta"]=new TH1D("gen_W_eta","gen_W_eta",20,-5,5);
  hists_base["gen_V_m"]=new TH1D("gen_V_m","gen_V_m",60,50,110);
  hists_base["gen_V_pt"]=new TH1D("gen_V_pt","gen_V_pt",60,0,300);
  hists_base["gen_V_eta"]=new TH1D("gen_V_eta","gen_V_eta",20,-5,5);
  hists_base["gen_t_m"]=new TH1D("gen_t_m","gen_t_m",75,100,250); 
  hists_base["gen_t_pt"]=new TH1D("gen_t_pt","gen_t_pt",500,0,2000);
  hists_base["gen_t_eta"]=new TH1D("gen_t_eta","gen_t_eta",20,-5,5);

  hists_base["hard_W_m"]=new TH1D("hard_W_m","hard_W_m",60,50,110);
  hists_base["hard_W_pt"]=new TH1D("hard_W_pt","hard_W_pt",60,0,300);
  hists_base["hard_W_eta"]=new TH1D("hard_W_eta","hard_W_eta",20,-5,5);
  hists_base["hard_V_m"]=new TH1D("hard_V_m","hard_V_m",60,50,110);
  hists_base["hard_V_pt"]=new TH1D("hard_V_pt","hard_V_pt",60,0,300);
  hists_base["hard_V_eta"]=new TH1D("hard_V_eta","hard_V_eta",20,-5,5);
  hists_base["hard_t_m"]=new TH1D("hard_t_m","hard_t_m",75,100,250); 
  hists_base["hard_t_pt"]=new TH1D("hard_t_pt","hard_t_pt",60,0,300);
  hists_base["hard_t_eta"]=new TH1D("hard_t_eta","hard_t_eta",20,-5,5);
  hists_base["hard_b_m"]=new TH1D("hard_b_m","hard_b_m",80,0,20); 
  hists_base["hard_b_pt"]=new TH1D("hard_b_pt","hard_b_pt",60,0,300);
  hists_base["hard_b_eta"]=new TH1D("hard_b_eta","hard_b_eta",20,-5,5);

  for(int i=0;i<8;i++){
    hists_base[Form("jet%d_pt",i)]=new TH1D(Form("jet%d_pt",i),Form("jet%d_pt",i),60,0,300);
    hists_base[Form("jet%d_eta",i)]=new TH1D(Form("jet%d_eta",i),Form("jet%d_eta",i),20,-5,5);
  }
  hists_base["jet_ht"]=new TH1D("jet_ht","jet_ht",500,0,2000);

  map<TString,TH1D*> hists;
  hists["sumw"]=new TH1D("sumw","sumw",200,0,200);
  int fire=0;
  for(ev.toBegin();!ev.atEnd();++ev){
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
    vector<reco::GenJet*> jets_ptcut=jets;

    fwlite::Handle<GenEventInfoProduct> geninfo;
    geninfo.getByLabel(ev,"generator");
    const vector<double>& weights=geninfo.ptr()->weights();
    //scale:4-10 pdf:11-110 as:111-112

    for(int i=0;i<weights.size();i++){
      hists["sumw"]->Fill(i,weights[i]);
    }

    reco::GenParticle *t=FindHard(gens,6), *tbar=FindHard(gens,-6), *V=FindHard(gens,24,true);
    if(V==NULL) V=FindHard(gens,23,true);
    if(V==NULL) V=FindHard(gens,25,true);
    if(V==NULL) continue;
    t=FindLastCopy(gens,t); tbar=FindLastCopy(gens,tbar); V=FindLastCopy(gens,V);
    reco::GenParticle *W=FindDaughter(gens,t,24), *b=FindDaughter(gens,t,5), *Wbar=FindDaughter(gens,tbar,-24),*bbar=FindDaughter(gens,tbar,-5);
    W=FindLastCopy(gens,W); b=FindLastCopy(gens,b); Wbar=FindLastCopy(gens,Wbar);bbar=FindLastCopy(gens,bbar);

    reco::GenJet *bjet=FindJet(jets,b); RemoveJet(jets,bjet);
    reco::GenJet *bbarjet=FindJet(jets,bbar); RemoveJet(jets,bbarjet);
    tuple<reco::GenJet*,reco::GenJet*> Wjets=FindWJets(jets,W); RemoveJet(jets,get<0>(Wjets)); RemoveJet(jets,get<1>(Wjets));
    tuple<reco::GenJet*,reco::GenJet*> Wbarjets=FindWJets(jets,Wbar); RemoveJet(jets,get<0>(Wbarjets)); RemoveJet(jets,get<1>(Wbarjets));
    tuple<reco::GenJet*,reco::GenJet*> Vjets=FindBosonJets(jets,V); RemoveJet(jets,get<0>(Vjets)); RemoveJet(jets,get<1>(Vjets));

    TLorentzVector vec_bjet=MakeTLorentzVector(bjet),vec_bbarjet=MakeTLorentzVector(bbarjet);
    TLorentzVector vec_Wjets=MakeTLorentzVector(Wjets), vec_Wbarjets=MakeTLorentzVector(Wbarjets), vec_Vjets=MakeTLorentzVector(Vjets);

    vector<reco::GenParticle> bjets;
    if(bjet) bjets.push_back(*bjet);
    if(bbarjet) bjets.push_back(*bbarjet);

    for(int i=0;i<weights.size();i++){
      TString suf=(i==0?"":Form("_weight%d",i));
      if(ievent==0){
	for(const auto& hist_base:hists_base)
	  hists[hist_base.first+suf]=(TH1D*)hist_base.second->Clone(hist_base.first+suf);
      }

      hists["njet"+suf]->Fill(jets_ptcut.size(),weights[i]);
      double HT=0;
      for(int i=0;i<jets_ptcut.size()&&i<8;i++){
	reco::GenJet* jet=jets_ptcut[i];
	HT+=jet->pt();
	hists["jet_pt"+suf]->Fill(jet->pt(),weights[i]);
	hists["jet_eta"+suf]->Fill(jet->eta(),weights[i]);
	hists[Form("jet%d_pt%s",i,suf.Data())]->Fill(jet->pt(),weights[i]);
	hists[Form("jet%d_eta%s",i,suf.Data())]->Fill(jet->eta(),weights[i]);
      }
      hists["jet_ht"+suf]->Fill(HT,weights[i]);

      hists["hard_t_m"+suf]->Fill(t->mass(),weights[i]);
      hists["hard_t_m"+suf]->Fill(tbar->mass(),weights[i]);
      hists["hard_t_pt"+suf]->Fill(t->pt(),weights[i]);
      hists["hard_t_pt"+suf]->Fill(tbar->pt(),weights[i]);
      hists["hard_t_eta"+suf]->Fill(t->eta(),weights[i]);
      hists["hard_t_eta"+suf]->Fill(tbar->eta(),weights[i]);
      hists["hard_b_m"+suf]->Fill(b->mass(),weights[i]);
      hists["hard_b_m"+suf]->Fill(bbar->mass(),weights[i]);
      hists["hard_b_pt"+suf]->Fill(b->pt(),weights[i]);
      hists["hard_b_pt"+suf]->Fill(bbar->pt(),weights[i]);
      hists["hard_b_eta"+suf]->Fill(b->eta(),weights[i]);
      hists["hard_b_eta"+suf]->Fill(bbar->eta(),weights[i]);
      hists["hard_W_m"+suf]->Fill(W->mass(),weights[i]);
      hists["hard_W_m"+suf]->Fill(Wbar->mass(),weights[i]);
      hists["hard_W_pt"+suf]->Fill(W->pt(),weights[i]);
      hists["hard_W_pt"+suf]->Fill(Wbar->pt(),weights[i]);
      hists["hard_W_eta"+suf]->Fill(W->eta(),weights[i]);
      hists["hard_W_eta"+suf]->Fill(Wbar->eta(),weights[i]);
      hists["hard_V_m"+suf]->Fill(V->mass(),weights[i]);
      hists["hard_V_pt"+suf]->Fill(V->pt(),weights[i]);
      hists["hard_V_eta"+suf]->Fill(V->eta(),weights[i]);
      
      hists["nbjet"+suf]->Fill(bjets.size(),weights[i]);
      for(const auto& bjet:bjets){
	hists["bjet_pt"+suf]->Fill(bjet.pt(),weights[i]);
	hists["bjet_eta"+suf]->Fill(bjet.eta(),weights[i]);
      }
      
      if(get<0>(Wjets)){
	hists["W_m"+suf]->Fill(vec_Wjets.M(),weights[i]);
	hists["W_pt"+suf]->Fill(vec_Wjets.Pt(),weights[i]);
	hists["W_eta"+suf]->Fill(vec_Wjets.Eta(),weights[i]);
      }
      if(get<0>(Wbarjets)){
	hists["W_m"+suf]->Fill(vec_Wbarjets.M(),weights[i]);
	hists["W_pt"+suf]->Fill(vec_Wbarjets.Pt(),weights[i]);
	hists["W_eta"+suf]->Fill(vec_Wbarjets.Eta(),weights[i]);
      }
      if(get<0>(Vjets)){
	hists["V_m"+suf]->Fill(vec_Vjets.M(),weights[i]);
	hists["V_pt"+suf]->Fill(vec_Vjets.Pt(),weights[i]);
	hists["V_eta"+suf]->Fill(vec_Vjets.Eta(),weights[i]);
      }      
      if(bjet&&get<0>(Wjets)){
	hists["t_m"+suf]->Fill((vec_bjet+vec_Wjets).M(),weights[i]);
	hists["t_pt"+suf]->Fill((vec_bjet+vec_Wjets).Pt(),weights[i]);
	hists["t_eta"+suf]->Fill((vec_bjet+vec_Wjets).Eta(),weights[i]);
      }	
      if(bbarjet&&get<0>(Wbarjets)){
	hists["t_m"+suf]->Fill((vec_bbarjet+vec_Wbarjets).M(),weights[i]);
	hists["t_pt"+suf]->Fill((vec_bbarjet+vec_Wbarjets).Pt(),weights[i]); 
	hists["t_eta"+suf]->Fill((vec_bbarjet+vec_Wbarjets).Eta(),weights[i]);
      }

    }

    
    /*
    for(unsigned int i=0;i<gens.size();i++){
      if(gens[i].isHardProcess())  cout<<i<<" "<<&gens[i]<<" "<<gens[i].pdgId()<<" "<<gens[i].status()<<" "<<gens[i].mother()<<"\t"<<gens[i].energy()<<" "<<gens[i].p()<<" "<<gens[i].eta()<<" "<<gens[i].phi()<<endl;
      //      cout<<i<<" "<<&gens[i]<<" "<<gens[i].pdgId()<<" "<<gens[i].status()<<" "<<gens[i].mother()<<"\t"<<gens[i].isHardProcess()<<gens[i].isLastCopy()<<gens[i].isLastCopyBeforeFSR()<<gens[i].isPromptDecayed()<<gens[i].isPromptFinalState()<<endl;
    }
    */
    ievent++;
  }
  TFile fout(outfile,"recreate");
  for(const auto& im:hists){
    im.second->Write();
  }
  cout<<fire<<endl;
}
