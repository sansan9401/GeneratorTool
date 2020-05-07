#include <iostream>
#include "TLorentzVector.h"
#include "TFile.h"
#include "TTree.h"
#include "TROOT.h"
#include "external/TH4D/TH4D.h"
using namespace std;
double GetCosThetaCS(TLorentzVector lm,TLorentzVector lp){
  TLorentzVector dilepton=lp+lm;
  double lmp=(lm.E()+lm.Pz())/sqrt(2);
  double lmm=(lm.E()-lm.Pz())/sqrt(2);
  double lpp=(lp.E()+lp.Pz())/sqrt(2);
  double lpm=(lp.E()-lp.Pz())/sqrt(2);
  double dimass=dilepton.M();
  double dipt=dilepton.Pt();
  double direction=dilepton.Pz()/fabs(dilepton.Pz());
  return direction*2*(lmp*lpm-lmm*lpp)/sqrt(dimass*dimass*(dimass*dimass+dipt*dipt));
}

void loop(TString infile,TString outfile){
  if(!TClass::GetDict("TH4D")) gROOT->ProcessLine(".L TH4D.cxx+");
  TFile f(infile);
  TTree* tree=(TTree*)f.Get("dytree");
  int l0_pid,l1_pid;
  float l0_px,l0_py,l0_pz,l0_e,l1_px,l1_py,l1_pz,l1_e,weight;
  tree->SetBranchAddress("l0_pid",&l0_pid);
  tree->SetBranchAddress("l0_px",&l0_px);
  tree->SetBranchAddress("l0_py",&l0_py);
  tree->SetBranchAddress("l0_pz",&l0_pz);
  tree->SetBranchAddress("l0_e",&l0_e);
  tree->SetBranchAddress("l1_pid",&l1_pid);
  tree->SetBranchAddress("l1_px",&l1_px);
  tree->SetBranchAddress("l1_py",&l1_py);
  tree->SetBranchAddress("l1_pz",&l1_pz);
  tree->SetBranchAddress("l1_e",&l1_e);
  tree->SetBranchAddress("weight",&weight);

  static const int afb_mbinnum=36;
  const double afb_mbin[afb_mbinnum+1]={60,65,70,74,77,80,82,84,86,88,89,90,91,92,93,94,96,98,100,103,106,110,115,120,130,140,150,175,200,240,280,340,400,600,1000,2000,5000};
  static const int afb_ybinnum=7;
  const double afb_ybin[afb_ybinnum+1]={0,0.4,0.8,1.2,1.6,2.0,2.4,2.8};
  static const int afb_ptbinnum=30;
  const double afb_ptbin[afb_ptbinnum+1]={0,2,4,6,8,10,12,14,16,18,20,24,28,32,36,40,45,50,55,60,70,80,90,100,120,140,160,190,250,500,1000};
  static const int afb_costbinnum=20;
  const double afb_costbin[afb_costbinnum+1]={-1,-0.9,-0.8,-0.7,-0.6,-0.5,-0.4,-0.3,-0.2,-0.1,0,0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9,1};

  TH4D* hist=new TH4D("DY4D","DY4D",afb_mbinnum,afb_mbin,afb_ybinnum,afb_ybin,afb_ptbinnum,afb_ptbin,afb_costbinnum,afb_costbin);
  int nevent=tree->GetEntries();
  for(int i=0;i<nevent;i++){
    if(i%1000==0) cout<<i<<endl;
    tree->GetEntry(i);
    TLorentzVector l0(l0_px,l0_py,l0_pz,l0_e);
    TLorentzVector l1(l1_px,l1_py,l1_pz,l1_e);
    TLorentzVector dilepton=l0+l1;
    TLorentzVector lp,lm;
    if(l0_pid>0){
      lm=l0;
      lp=l1;
    }else{
      lm=l1;
      lp=l0;
    }
    hist->Fill(dilepton.M(),fabs(dilepton.Rapidity()),dilepton.Pt(),GetCosThetaCS(lm,lp),weight);
  }
  TFile fout(outfile,"recreate");
  hist->Write();
}
