#include "TLorentzVector.h"
#include "TH1D.h"
#include "TFile.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"
#include "DataFormats/JetReco/interface/GenJet.h"
#include "DataFormats/FWLite/interface/Handle.h"
#include "DataFormats/FWLite/interface/Event.h"
#include "SimDataFormats/GeneratorProducts/interface/GenEventInfoProduct.h"

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
TLorentzVector MakeTLorentzVector(vector<reco::GenParticle*> particles){
  TLorentzVector out;
  for(const auto& particle:particles) out+=MakeTLorentzVector(particle);
  return out;
}

int GetIndex(const vector<reco::GenParticle>& gens,reco::GenParticle* particle){
  for(int i=0;i<gens.size();i++){
    if(particle==(reco::GenParticle*)&gens[i]) return i;
  }
  return -1;
}
void PrintGen(const reco::GenParticle& gen){
  cout<<&gen<<", id: "<<gen.pdgId()<<", ("<<gen.pt()<<", "<<gen.eta()<<", "<<gen.phi()<<") status: "<<gen.status()<<", mom: "<<gen.mother()<<"\tisHard"<<gen.isHardProcess()<<" last"<<gen.isLastCopy()<<" beforeFSR"<<gen.isLastCopyBeforeFSR()<<" prompt"<<gen.isPromptDecayed()<<" promptFS"<<gen.isPromptFinalState()<<endl;
}  
void PrintGens(const vector<reco::GenParticle>& gens){
  for(unsigned int i=0;i<gens.size();i++){
    //cout<<(reco::GenParticle*)&gens[i]<<endl;
    cout<<GetIndex(gens,(reco::GenParticle*)&gens[i])<<", "<<&gens[i]<<", id: "<<gens[i].pdgId()<<", ("<<gens[i].pt()<<", "<<gens[i].eta()<<", "<<gens[i].phi()<<"), status: "<<gens[i].status()<<", mom: "<<GetIndex(gens,(reco::GenParticle*)gens[i].mother())<<"\thard"<<gens[i].isHardProcess()<<" last"<<gens[i].isLastCopy()<<" beforeFSR"<<gens[i].isLastCopyBeforeFSR()<<" prompt"<<gens[i].isPromptDecayed()<<" promptFS"<<gens[i].isPromptFinalState()<<endl;
    //cout<<i<<" ";
    //PrintGen(gens[i]);
  }
}  
void PrintGens(const vector<reco::GenParticle*>& gens){
  for(unsigned int i=0;i<gens.size();i++){
    cout<<i<<" ";
    PrintGen(*gens[i]);
  }
}  

reco::GenParticle* FindHard(const vector<reco::GenParticle>& gens,int pdgid,bool absid=false){
  for(const auto& gen:gens){
    if(gen.isHardProcess())
      if((absid?abs(gen.pdgId()):gen.pdgId())==pdgid) return (reco::GenParticle*)&gen;
  }
  cout<<"[FindHard] no particle in hard gen with "<<pdgid<<endl;
  return NULL;
}    
reco::GenParticle* FindLastCopy(const vector<reco::GenParticle>& gens,reco::GenParticle *ancestor){
  if(abs(ancestor->pdgId())==5) {
    cout<<"[FindLastCopy] b quark (pdgId: "<<ancestor->pdgId()<<")"<<endl;
    cout<<"acestor: "; PrintGen(*ancestor); 
  }
  reco::GenParticle* last=ancestor;
  for(const auto& gen:gens){
    if(gen.mother()==last&&gen.pdgId()==ancestor->pdgId()){
      last=(reco::GenParticle*)&gen;
      if(abs(ancestor->pdgId())==5) { cout<<"New Cand: "; PrintGen(*last);  }
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
    cout<<"[FindDaughters] no particle in daughers with "<<pdgid<<endl;
  return daughters;
}
reco::GenParticle* FindDaughter(const vector<reco::GenParticle>& gens,reco::GenParticle *mother,int pdgid){
  vector<reco::GenParticle*> daughters=FindDaughters(gens,mother,pdgid);
  if(daughters.size()==1) return daughters[0];
  else cout<<"[FindDaughter] "<<daughters.size()<<" particles in daughers with "<<pdgid<<endl;
  return NULL;
}
bool isDescendant(const reco::GenParticle *ancestor,const reco::GenParticle *descendant){
  while(descendant->mother()){
    if(ancestor==(reco::GenParticle*)descendant->mother()) return true;
    else descendant=(reco::GenParticle*)descendant->mother();
  }
  return false;
}
vector<reco::GenParticle*> FindDescendants(const vector<reco::GenParticle>& gens,reco::GenParticle *ancestor,int status,int pdgid=0) {
  vector<reco::GenParticle*> descendants;
  for(const auto& gen:gens){
    //if(pdgid>0&&gen.pdgId()!=pdgid) continue;
    if(pdgid&&gen.pdgId()!=pdgid) continue;
    //if(status>0&&gen.status()%10!=status) continue;
    if(gen.status()%10!=status) continue;
    if(!gen.isLastCopy()) continue;
    if(isDescendant(ancestor,&gen)) {
      descendants.push_back((reco::GenParticle*)&gen);
      break;
    }
  }
  return descendants;
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
    double min=0.6;
    for(const auto& candidate:candidates){
      double diff=fabs(candidate->p()-gen->p())/gen->p();
      if(diff<min){
        min=diff;
        final=candidate;
      }
    }
  }
  else{
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

  TFile f(infile);
  fwlite::Event ev(&f);
  int ievent=0;
  map<TString,TH1D*> hists_base;
  hists_base["njet"]=new TH1D("njet","njet",20,0,20);
  hists_base["jet_pt"]=new TH1D("jet_pt","jet_pt",150,0,300);
  hists_base["jet_eta"]=new TH1D("jet_eta","jet_eta",100,-5,5);
  hists_base["nbjet"]=new TH1D("nbjet","nbjet",5,0,5);
  hists_base["bjet_pt"]=new TH1D("bjet_pt","bjet_pt",100,0,500);
  hists_base["bjet_eta"]=new TH1D("bjet_eta","bjet_eta",100,-5,5);
  hists_base["W_m"]=new TH1D("W_m","W_m",80,60,100);
  hists_base["W_pt"]=new TH1D("W_pt","W_pt",150,0,300);
  hists_base["W_eta"]=new TH1D("W_eta","W_eta",100,-5,5);
  hists_base["V_m"]=new TH1D("V_m","V_m",120,60,120);
  hists_base["V_pt"]=new TH1D("V_pt","V_pt",150,0,300);
  hists_base["V_eta"]=new TH1D("V_eta","V_eta",100,-5,5);
  hists_base["t_m"]=new TH1D("t_m","t_m",150,100,250); 
  hists_base["t_pt"]=new TH1D("t_pt","t_pt",100,0,500);
  hists_base["t_eta"]=new TH1D("t_eta","t_eta",100,-5,5);
  hists_base["t_rap"]=new TH1D("t_rap","t_rap",100,-5,5);
  hists_base["tt_m"]=new TH1D("tt_m","tt_m",150,250,1000); 
  hists_base["tt_pt"]=new TH1D("tt_pt","tt_pt",100,0,500);
  hists_base["tt_eta"]=new TH1D("tt_eta","tt_eta",100,-5,5);
  hists_base["tt_rap"]=new TH1D("tt_rap","tt_rap",80,-4,4);
  hists_base["ttV_m"]=new TH1D("ttV_m","ttV_m",120,300,1500); 
  hists_base["ttV_pt"]=new TH1D("ttV_pt","ttV_pt",100,0,200);
  hists_base["ttV_eta"]=new TH1D("ttV_eta","ttV_eta",100,-5,5);
  hists_base["ttV_rap"]=new TH1D("ttV_rap","ttV_rap",80,-4,4);
  
  hists_base["em_pt"]=new TH1D("em_pt","em_pt",100,0,200);
  hists_base["ep_pt"]=new TH1D("ep_pt","ep_pt",100,0,200);
  hists_base["lldelphi"]=new TH1D("lldelphi","lldelphi",100,-5,5);

  hists_base["hard_W_m"]=new TH1D("hard_W_m","hard_W_m",80,60,100);
  hists_base["hard_W_pt"]=new TH1D("hard_W_pt","hard_W_pt",100,0,200);
  hists_base["hard_W_eta"]=new TH1D("hard_W_eta","hard_W_eta",100,-5,5);
  hists_base["hard_V_m"]=new TH1D("hard_V_m","hard_V_m",120,0,120);
  hists_base["hard_V_pt"]=new TH1D("hard_V_pt","hard_V_pt",150,0,300);
  hists_base["hard_V_eta"]=new TH1D("hard_V_eta","hard_V_eta",100,-5,5);
  hists_base["hard_t_m"]=new TH1D("hard_t_m","hard_t_m",100,150,200); 
  hists_base["hard_t_pt"]=new TH1D("hard_t_pt","hard_t_pt",100,0,500);
  hists_base["hard_t_eta"]=new TH1D("hard_t_eta","hard_t_eta",100,-5,5);
  hists_base["hard_b_m"]=new TH1D("hard_b_m","hard_b_m",80,0,20); 
  hists_base["hard_b_pt"]=new TH1D("hard_b_pt","hard_b_pt",100,0,200);
  hists_base["hard_b_eta"]=new TH1D("hard_b_eta","hard_b_eta",100,-5,5);

  for(int i=0;i<4;i++){
    hists_base[Form("jet%d_pt",i)]=new TH1D(Form("jet%d_pt",i),Form("jet%d_pt",i),60,0,300);
    hists_base[Form("jet%d_eta",i)]=new TH1D(Form("jet%d_eta",i),Form("jet%d_eta",i),20,-5,5);
  }
  hists_base["jet_ht"]=new TH1D("jet_ht","jet_ht",100,0,1000);

  map<TString,TH1D*> hists;
  hists["sumw"]=new TH1D("sumw","sumw",200,0,200);
/////////////////////////////////////////////////////////////////////////
  //hists["dimuon_m"]=new TH1D("dimuon_m","dimuon_m",150,0,150);
/////////////////////////////////////////////////////////////////////////

  int fire=0;
  for(ev.toBegin();!ev.atEnd();++ev){
    //if(ievent>100) break;
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
    vector<reco::GenJet*> jets_ptcut=jets;

cout<<endl;
cout<<"=========================================================="<<endl;
cout<<"----- Jet information -----"<<endl;
cout<<"njets(pt > 30) = "<<jets.size()<<endl;
for(int i=0; i<jets.size(); i++) {
  cout<<i<<" ("<<jets[i]->pt()<<", "<<jets[i]->eta()<<", "<<jets[i]->phi()<<")"<<endl;
}
cout<<endl;

    fwlite::Handle<GenEventInfoProduct> geninfo;
    geninfo.getByLabel(ev,"generator");
    const vector<double>& weights=geninfo.ptr()->weights();
    //scale:4-10 pdf:11-110 as:111-112

    for(int i=0;i<weights.size();i++){
      hists["sumw"]->Fill(i,weights[i]);
    }

/////////////////////////////////////////////////////////////////////////

    // Find top quarks
    //cout<<endl;
    //cout<<"Find t & V"<<endl;
    reco::GenParticle *t=FindHard(gens,6), *tbar=FindHard(gens,-6);
    t=FindLastCopy(gens,t); tbar=FindLastCopy(gens,tbar);
    // Find genparticles from top decays (W bosons & b quarks)
    //cout<<"Find t decays (b, W)"<<endl;
    reco::GenParticle *W=FindDaughter(gens,t,24), *b=FindDaughter(gens,t,5), *Wbar=FindDaughter(gens,tbar,-24),*bbar=FindDaughter(gens,tbar,-5);
    W=FindLastCopy(gens,W); b=FindLastCopy(gens,b); Wbar=FindLastCopy(gens,Wbar);bbar=FindLastCopy(gens,bbar);

    // Find b-jets
    //cout<<"Find bjets"<<endl;
    reco::GenJet *bjet=FindJet(jets,b); RemoveJet(jets,bjet);
    reco::GenJet *bbarjet=FindJet(jets,bbar); RemoveJet(jets,bbarjet);
    TLorentzVector vec_bjet=MakeTLorentzVector(bjet),vec_bbarjet=MakeTLorentzVector(bbarjet);

    vector<reco::GenParticle> bjets;
    if(bjet) bjets.push_back(*bjet);
    if(bbarjet) bjets.push_back(*bbarjet);

    // Find W(->enu) decays
    //cout<<"Find W decay products"<<endl;
    reco::GenParticle *ebar=FindDaughter(gens,W,-11), *nu=FindDaughter(gens,W,12), *e=FindDaughter(gens,Wbar,11), *nubar=FindDaughter(gens,Wbar,-12);
    ebar=FindLastCopy(gens,ebar); nu=FindLastCopy(gens,nu); e=FindLastCopy(gens,e); nubar=FindLastCopy(gens,nubar);
//    vector<reco::GenParticle*> W_descendants=FindDescendants(gens,W,1), Wbar_descendants=FindDescendants(gens,Wbar,1);
    vector<reco::GenParticle*> W_descendants, Wbar_descendants;
////    W_descendants.push_back((reco::GenParticle*)&ebar); W_descendants.push_back((reco::GenParticle*)&nu);
////    Wbar_descendants.push_back((reco::GenParticle*)&e); Wbar_descendants.push_back((reco::GenParticle*)&nubar);
    W_descendants.push_back(ebar); W_descendants.push_back(nu);
    Wbar_descendants.push_back(e); Wbar_descendants.push_back(nubar);

    TLorentzVector vec_Wjets=MakeTLorentzVector(W_descendants), vec_Wbarjets=MakeTLorentzVector(Wbar_descendants);
    TLorentzVector vec_ep, vec_em;
    for(const auto& gen:W_descendants)
      if(gen->pdgId()==-11) vec_ep=MakeTLorentzVector(gen);
    for(const auto& gen:Wbar_descendants)
      if(gen->pdgId()==11) vec_em=MakeTLorentzVector(gen);

    // Find V(Z->mumu) decays
    reco::GenParticle *V=FindHard(gens,23,true);
    //if(V==NULL) V=FindHard(gens,24,true);
    //if(V==NULL) V=FindHard(gens,21,true);
    //if(V==NULL) V=FindHard(gens,25,true);
    vector<reco::GenParticle*> V_descendants; 
    cout<<endl<<"-------------------------------------------------------"<<endl;
    if(V!=NULL) { // MG
      cout<<"For MG"<<endl;
      V=FindLastCopy(gens,V);

      vector<reco::GenParticle*> temp=FindDescendants(gens,V,1,13);
      V_descendants.insert(V_descendants.end(), temp.begin(), temp.end());
      temp.clear();
      temp=FindDescendants(gens,V,1,-13);
      V_descendants.insert(V_descendants.end(), temp.begin(), temp.end());
    }
    else { // Sherpa
      cout<<"For Sherpa"<<endl;
      for(const auto& gen:gens){
        if(gen.isHardProcess() && gen.isLastCopy() && gen.pdgId()==13)
           V_descendants.push_back((reco::GenParticle*)&gen);
        else if(gen.isHardProcess() && gen.isLastCopy() && gen.pdgId()==-13)
           V_descendants.push_back((reco::GenParticle*)&gen);
      }
    }
    if(V_descendants.size()==0) {
      cout<<"[CHANGE GENERATOR] NOT Sherpa -> MG"<<endl;
      V_descendants.clear();

      reco::GenParticle *mu, *mubar;
      for(const auto& gen:gens) {
        if(gen.isHardProcess() && gen.pdgId()==13)
          mu=(reco::GenParticle*)&gen;
        else if(gen.isHardProcess() && gen.pdgId()==-13)
          mubar=(reco::GenParticle*)&gen;
      }

      V_descendants.push_back(FindLastCopy(gens,mu));
      V_descendants.push_back(FindLastCopy(gens,mubar));
    }

    if(V_descendants.size()!=2) { cout<<"[ERROR] No V(Z->mumu) descendants"<<endl; PrintGens(gens); continue; }
    TLorentzVector vec_Vjets=MakeTLorentzVector(V_descendants);


    for(int i=0;i<1/*weights.size()*/;i++){
      TString suf=(i==0?"":Form("_weight%d",i));
      if(ievent==0){
        for(const auto& hist_base:hists_base)
          hists[hist_base.first+suf]=(TH1D*)hist_base.second->Clone(hist_base.first+suf);
      }

      cout<<endl;
      cout<<"-- [JET INFORMATION] --"<<endl;
      cout<<"njets(pt > 30) = "<<jets_ptcut.size()<<endl;
      cout<<"* Only i < 4 jets"<<endl;

      hists["njet"+suf]->Fill(jets_ptcut.size(),weights[i]);
      double HT=0;
      for(int j=0;j<jets_ptcut.size()&&j<4;j++){
        reco::GenJet* jet=jets_ptcut[j];
        HT+=jet->pt();
        hists["jet_pt"+suf]->Fill(jet->pt(),weights[i]);
        hists["jet_eta"+suf]->Fill(jet->eta(),weights[i]);
        //hists[Form("jet%d_pt%s",i,suf.Data())]->Fill(jet->pt(),weights[i]);
        //hists[Form("jet%d_eta%s",i,suf.Data())]->Fill(jet->eta(),weights[i]);
        hists[Form("jet"+to_string(j)+"_pt"+suf)]->Fill(jet->pt(),weights[i]);
        hists[Form("jet"+to_string(j)+"_eta"+suf)]->Fill(jet->eta(),weights[i]);

        cout<<"jet ("<<jet->pt()<<", "<<jet->eta()<<", "<<jet->phi()<<")"<<endl;
      }
      hists["jet_ht"+suf]->Fill(HT,weights[i]);

      cout<<endl;
      hists["nbjet"+suf]->Fill(bjets.size(),weights[i]);
      for(const auto& bjet:bjets){
        hists["bjet_pt"+suf]->Fill(bjet.pt(),weights[i]);
        hists["bjet_eta"+suf]->Fill(bjet.eta(),weights[i]);

        cout<<"bjet ("<<bjet.pt()<<", "<<bjet.eta()<<", "<<bjet.phi()<<")"<<endl;
      }
      cout<<endl;

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
      if(V!=NULL) {
        hists["hard_V_m"+suf]->Fill(V->mass(),weights[i]);
        hists["hard_V_pt"+suf]->Fill(V->pt(),weights[i]);
        hists["hard_V_eta"+suf]->Fill(V->eta(),weights[i]);
      }

      hists["W_m"+suf]->Fill(vec_Wjets.M(),weights[i]);
      hists["W_pt"+suf]->Fill(vec_Wjets.Pt(),weights[i]);
      hists["W_eta"+suf]->Fill(vec_Wjets.Eta(),weights[i]);
      hists["W_m"+suf]->Fill(vec_Wbarjets.M(),weights[i]);
      hists["W_pt"+suf]->Fill(vec_Wbarjets.Pt(),weights[i]);
      hists["W_eta"+suf]->Fill(vec_Wbarjets.Eta(),weights[i]);
      hists["V_m"+suf]->Fill(vec_Vjets.M(),weights[i]);
      hists["V_pt"+suf]->Fill(vec_Vjets.Pt(),weights[i]);
      hists["V_eta"+suf]->Fill(vec_Vjets.Eta(),weights[i]);
      hists["ep_pt"+suf]->Fill(vec_ep.Pt(),weights[i]);
      hists["em_pt"+suf]->Fill(vec_em.Pt(),weights[i]);
      hists["lldelphi"+suf]->Fill(vec_ep.DeltaPhi(vec_em),weights[i]);
      /*
      if(vec_ep.DeltaPhi(vec_em)==0){
        cout<<"W"<<endl;
        PrintGens(W_descendants);
        cout<<"Wbar"<<endl;
        PrintGens(Wbar_descendants);
        cout<<"V"<<endl;
        PrintGens(V_descendants);
        cout<<"All"<<endl;
        PrintGens(gens);
      }
      */
      if(bjet){
        hists["t_m"+suf]->Fill((vec_bjet+vec_Wjets).M(),weights[i]);
        hists["t_pt"+suf]->Fill((vec_bjet+vec_Wjets).Pt(),weights[i]);
        hists["t_eta"+suf]->Fill((vec_bjet+vec_Wjets).Eta(),weights[i]);
        hists["t_rap"+suf]->Fill((vec_bjet+vec_Wjets).Rapidity(),weights[i]);
      }        
      if(bbarjet){
        hists["t_m"+suf]->Fill((vec_bbarjet+vec_Wbarjets).M(),weights[i]);
        hists["t_pt"+suf]->Fill((vec_bbarjet+vec_Wbarjets).Pt(),weights[i]); 
        hists["t_eta"+suf]->Fill((vec_bbarjet+vec_Wbarjets).Eta(),weights[i]);
        hists["t_rap"+suf]->Fill((vec_bbarjet+vec_Wbarjets).Rapidity(),weights[i]);
      }
      if(bjet&&bbarjet){
        hists["tt_m"+suf]->Fill((vec_bjet+vec_Wjets+vec_bbarjet+vec_Wbarjets).M(),weights[i]);
        hists["tt_pt"+suf]->Fill((vec_bjet+vec_Wjets+vec_bbarjet+vec_Wbarjets).Pt(),weights[i]);
        hists["tt_eta"+suf]->Fill((vec_bjet+vec_Wjets+vec_bbarjet+vec_Wbarjets).Eta(),weights[i]);
        hists["tt_rap"+suf]->Fill((vec_bjet+vec_Wjets+vec_bbarjet+vec_Wbarjets).Rapidity(),weights[i]);
        hists["ttV_m"+suf]->Fill((vec_bjet+vec_Wjets+vec_bbarjet+vec_Wbarjets+vec_Vjets).M(),weights[i]);
        hists["ttV_pt"+suf]->Fill((vec_bjet+vec_Wjets+vec_bbarjet+vec_Wbarjets+vec_Vjets).Pt(),weights[i]);
        hists["ttV_eta"+suf]->Fill((vec_bjet+vec_Wjets+vec_bbarjet+vec_Wbarjets+vec_Vjets).Eta(),weights[i]);
        hists["ttV_rap"+suf]->Fill((vec_bjet+vec_Wjets+vec_bbarjet+vec_Wbarjets+vec_Vjets).Rapidity(),weights[i]);
      }
    }


cout<<endl<<"-- Full Event -- "<<endl;
PrintGens(gens);


    
    ievent++;
  }
  TFile fout(outfile,"recreate");
  for(const auto& im:hists){
    im.second->Write();
  }
  //cout<<fire<<endl;
}
