#include <iostream>
#include <vector>
#include "TLorentzVector.h"
#include "TFile.h"
#include "TTree.h"
#include "TChain.h"
#include "TROOT.h"
#include "TString.h"
#include "external/TH4D/TH4D.h"
using namespace std;
double GetCosThetaCS(TLorentzVector lm,TLorentzVector lp,int direction=0){
  TLorentzVector dilepton=lp+lm;
  double lmp=(lm.E()+lm.Pz())/sqrt(2);
  double lmm=(lm.E()-lm.Pz())/sqrt(2);
  double lpp=(lp.E()+lp.Pz())/sqrt(2);
  double lpm=(lp.E()-lp.Pz())/sqrt(2);
  double dimass=dilepton.M();
  double dipt=dilepton.Pt();
  if(direction==0)
    direction=dilepton.Pz()>0?1:-1;
  return direction*2*(lmp*lpm-lmm*lpp)/sqrt(dimass*dimass*(dimass*dimass+dipt*dipt));
}
map<TString,TH4D*> hists;
TH4D* GetHist4D(TString histname){
  TH4D *h = NULL;
  std::map<TString, TH4D*>::iterator mapit = hists.find(histname);
  if(mapit != hists.end()) return mapit->second;
  return h;
}
void FillHist(TString histname,
	      Double_t value_x, Double_t value_y, Double_t value_z, Double_t value_u,
	      Double_t weight,
	      Int_t n_binx, Double_t *xbins,
	      Int_t n_biny, Double_t *ybins,
	      Int_t n_binz, Double_t *zbins,
	      Int_t n_binu, Double_t *ubins){
  
  TH4D *this_hist = GetHist4D(histname);
  if( !this_hist ){
    this_hist = new TH4D(histname, "", n_binx, xbins, n_biny, ybins, n_binz, zbins, n_binu, ubins);
    this_hist->SetDirectory(NULL);
    hists[histname] = this_hist;
  }

  this_hist->Fill(value_x, value_y, value_z, value_u, weight);

}
void loop(vector<TString> infiles,TString outfile){
  TChain* tree=new TChain("dytree");
  for(const auto& file:infiles)
    tree->AddFile(file);
  int p0_pid,p1_pid,l0_pid,l1_pid;
  float p0_x,p1_x;
  float l0_px,l0_py,l0_pz,l0_e,l1_px,l1_py,l1_pz,l1_e;
  float l0_px_dressed,l0_py_dressed,l0_pz_dressed,l0_e_dressed,l1_px_dressed,l1_py_dressed,l1_pz_dressed,l1_e_dressed;
  float l0_px_bare,l0_py_bare,l0_pz_bare,l0_e_bare,l1_px_bare,l1_py_bare,l1_pz_bare,l1_e_bare;
  float weight;
  vector<float>* weights=new vector<float>;
  tree->SetBranchAddress("p0_pid",&p0_pid);
  tree->SetBranchAddress("p1_pid",&p1_pid);
  tree->SetBranchAddress("p0_x",&p0_x);
  tree->SetBranchAddress("p1_x",&p1_x);

  tree->SetBranchAddress("l0_pid",&l0_pid);
  tree->SetBranchAddress("l1_pid",&l1_pid);

  tree->SetBranchAddress("l0_px",&l0_px);
  tree->SetBranchAddress("l0_py",&l0_py);
  tree->SetBranchAddress("l0_pz",&l0_pz);
  tree->SetBranchAddress("l0_e",&l0_e);
  tree->SetBranchAddress("l1_px",&l1_px);
  tree->SetBranchAddress("l1_py",&l1_py);
  tree->SetBranchAddress("l1_pz",&l1_pz);
  tree->SetBranchAddress("l1_e",&l1_e);

  tree->SetBranchAddress("l0_px_dressed",&l0_px_dressed);
  tree->SetBranchAddress("l0_py_dressed",&l0_py_dressed);
  tree->SetBranchAddress("l0_pz_dressed",&l0_pz_dressed);
  tree->SetBranchAddress("l0_e_dressed",&l0_e_dressed);
  tree->SetBranchAddress("l1_px_dressed",&l1_px_dressed);
  tree->SetBranchAddress("l1_py_dressed",&l1_py_dressed);
  tree->SetBranchAddress("l1_pz_dressed",&l1_pz_dressed);
  tree->SetBranchAddress("l1_e_dressed",&l1_e_dressed);

  tree->SetBranchAddress("l0_px_bare",&l0_px_bare);
  tree->SetBranchAddress("l0_py_bare",&l0_py_bare);
  tree->SetBranchAddress("l0_pz_bare",&l0_pz_bare);
  tree->SetBranchAddress("l0_e_bare",&l0_e_bare);
  tree->SetBranchAddress("l1_px_bare",&l1_px_bare);
  tree->SetBranchAddress("l1_py_bare",&l1_py_bare);
  tree->SetBranchAddress("l1_pz_bare",&l1_pz_bare);
  tree->SetBranchAddress("l1_e_bare",&l1_e_bare);

  tree->SetBranchAddress("weight",&weight);
  tree->SetBranchAddress("weights",&weights);

  static const int afb_mbinnum=40;
  const double afb_mbin[afb_mbinnum+1]={60,65,70,74,77,80,82,84,86,88,89,90,91,92,93,94,96,98,100,103,106,110,115,120,130,140,150,175,200,240,280,340,400,500,600,700,800,1000,1500,2000,3000};
  static const int afb_ybinnum=12;
  const double afb_ybin[afb_ybinnum+1]={-2.4,-2.0,-1.6,-1.2,-0.8,-0.4,0,0.4,0.8,1.2,1.6,2.0,2.4};
  static const int afb_ptbinnum=30;
  const double afb_ptbin[afb_ptbinnum+1]={0,2,4,6,8,10,12,14,16,18,20,24,28,32,36,40,45,50,55,60,70,80,90,100,120,140,160,190,250,400,650};
  static const int afb_costbinnum=20;
  const double afb_costbin[afb_costbinnum+1]={-1,-0.9,-0.8,-0.7,-0.6,-0.5,-0.4,-0.3,-0.2,-0.1,0,0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9,1};

  int nevent=tree->GetEntries();
  for(int i=0;i<nevent;i++){
    if(i%1000==0) cout<<i<<endl;
    tree->GetEntry(i);
    TLorentzVector lp,lm,lp_dressed,lm_dressed,lp_bare,lm_bare;
    if(l0_pid>0){
      lm.SetPxPyPzE(l0_px,l0_py,l0_pz,l0_e);
      lm_dressed.SetPxPyPzE(l0_px_dressed,l0_py_dressed,l0_pz_dressed,l0_e_dressed);
      lm_bare.SetPxPyPzE(l0_px_bare,l0_py_bare,l0_pz_bare,l0_e_bare);
      lp.SetPxPyPzE(l1_px,l1_py,l1_pz,l1_e);
      lp_dressed.SetPxPyPzE(l1_px_dressed,l1_py_dressed,l1_pz_dressed,l1_e_dressed);
      lp_bare.SetPxPyPzE(l1_px_bare,l1_py_bare,l1_pz_bare,l1_e_bare);
    }else{
      lp.SetPxPyPzE(l0_px,l0_py,l0_pz,l0_e);
      lp_dressed.SetPxPyPzE(l0_px_dressed,l0_py_dressed,l0_pz_dressed,l0_e_dressed);
      lp_bare.SetPxPyPzE(l0_px_bare,l0_py_bare,l0_pz_bare,l0_e_bare);
      lm.SetPxPyPzE(l1_px,l1_py,l1_pz,l1_e);
      lm_dressed.SetPxPyPzE(l1_px_dressed,l1_py_dressed,l1_pz_dressed,l1_e_dressed);
      lm_bare.SetPxPyPzE(l1_px_bare,l1_py_bare,l1_pz_bare,l1_e_bare);
    }
    TLorentzVector dilepton=lm+lp;
    TLorentzVector dilepton_dressed=lm_dressed+lp_dressed;
    TLorentzVector dilepton_bare=lm_bare+lp_bare;

    TString prefix;
    if(abs(l0_pid)==13) prefix="mm/";
    else if(abs(l0_pid)==11) prefix="ee/";

    double cost_correct=-999;
    if(p0_pid==21){
      if(p1_pid==21) cost_correct=GetCosThetaCS(lm,lp,0);
      else if(p1_pid>0) cost_correct=GetCosThetaCS(lm,lp,-1);
      else if(p1_pid<0) cost_correct=GetCosThetaCS(lm,lp,1);
    }else if(p0_pid>0){
      if(p1_pid==21) cost_correct=GetCosThetaCS(lm,lp,1);
      else if(p1_pid>0) cost_correct=GetCosThetaCS(lm,lp,0);
      else if(p1_pid<0) cost_correct=GetCosThetaCS(lm,lp,1);
    }else if(p0_pid<0){
      if(p1_pid==21) cost_correct=GetCosThetaCS(lm,lp,-1);
      else if(p1_pid>0) cost_correct=GetCosThetaCS(lm,lp,-1);
      else if(p1_pid<0) cost_correct=GetCosThetaCS(lm,lp,0);
    }
    if(cost_correct==-999){
      cout<<"wrong parton pid"<<endl;
      exit(1);
    }
    
    FillHist(Form("%scosthetaCS",prefix.Data()),dilepton.M(),dilepton.Rapidity(),dilepton.Pt(),GetCosThetaCS(lm,lp),weight,afb_mbinnum,(double*)afb_mbin,afb_ybinnum,(double*)afb_ybin,afb_ptbinnum,(double*)afb_ptbin,afb_costbinnum,(double*)afb_costbin);
    FillHist(Form("%scosthetaCS_dressed",prefix.Data()),dilepton_dressed.M(),dilepton_dressed.Rapidity(),dilepton_dressed.Pt(),GetCosThetaCS(lm_dressed,lp_dressed),weight,afb_mbinnum,(double*)afb_mbin,afb_ybinnum,(double*)afb_ybin,afb_ptbinnum,(double*)afb_ptbin,afb_costbinnum,(double*)afb_costbin);
    FillHist(Form("%scosthetaCS_bare",prefix.Data()),dilepton_bare.M(),dilepton_bare.Rapidity(),dilepton_bare.Pt(),GetCosThetaCS(lm_bare,lp_bare),weight,afb_mbinnum,(double*)afb_mbin,afb_ybinnum,(double*)afb_ybin,afb_ptbinnum,(double*)afb_ptbin,afb_costbinnum,(double*)afb_costbin);
    FillHist(Form("%scosthetaCS_correct",prefix.Data()),dilepton.M(),dilepton.Rapidity(),dilepton.Pt(),cost_correct,weight,afb_mbinnum,(double*)afb_mbin,afb_ybinnum,(double*)afb_ybin,afb_ptbinnum,(double*)afb_ptbin,afb_costbinnum,(double*)afb_costbin);

    for(int i=0;i<(int)weights->size();i++){
      FillHist(Form("%scosthetaCS_%d",prefix.Data(),i),dilepton.M(),dilepton.Rapidity(),dilepton.Pt(),GetCosThetaCS(lm,lp),weights->at(i),afb_mbinnum,(double*)afb_mbin,afb_ybinnum,(double*)afb_ybin,afb_ptbinnum,(double*)afb_ptbin,afb_costbinnum,(double*)afb_costbin);
      FillHist(Form("%scosthetaCS_dressed_%d",prefix.Data(),i),dilepton_dressed.M(),dilepton_dressed.Rapidity(),dilepton_dressed.Pt(),GetCosThetaCS(lm_dressed,lp_dressed),weights->at(i),afb_mbinnum,(double*)afb_mbin,afb_ybinnum,(double*)afb_ybin,afb_ptbinnum,(double*)afb_ptbin,afb_costbinnum,(double*)afb_costbin);
      FillHist(Form("%scosthetaCS_bare_%d",prefix.Data(),i),dilepton_bare.M(),dilepton_bare.Rapidity(),dilepton_bare.Pt(),GetCosThetaCS(lm_bare,lp_bare),weights->at(i),afb_mbinnum,(double*)afb_mbin,afb_ybinnum,(double*)afb_ybin,afb_ptbinnum,(double*)afb_ptbin,afb_costbinnum,(double*)afb_costbin);
      FillHist(Form("%scosthetaCS_correct_%d",prefix.Data(),i),dilepton.M(),dilepton.Rapidity(),dilepton.Pt(),cost_correct,weights->at(i),afb_mbinnum,(double*)afb_mbin,afb_ybinnum,(double*)afb_ybin,afb_ptbinnum,(double*)afb_ptbin,afb_costbinnum,(double*)afb_costbin);
    }
  }
  TFile fout(outfile,"recreate");
  for(const auto& [name,hist]:hists){
    TString this_name=name(name.Last('/')+1,name.Length());
    TString this_suffix=name(0,name.Last('/'));
    TDirectory *dir = fout.GetDirectory(this_suffix);
    if(!dir){
      fout.mkdir(this_suffix);
    }
    fout.cd(this_suffix);
    hist->Write(this_name);
    fout.cd();
  }
}
