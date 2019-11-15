#include"Plotter/SherpadayPlotter.cc"
void SaveAll(){
  SherpadayPlotter p;
  DEBUG=2;
  p.Setup(0);
  //p.AddPlot("W_m","norm 2:widewidey");
  //p.SaveCanvas("W_m");
  p.plots.clear();
  p.AddPlots(".*","norm 2:widewidey");
  p.RemovePlots("weight");
  p.RemovePlots("sumw");
  p.SaveCanvases(".*");
  
  p.Setup(1);
  p.AddPlot("W_m","norm 2:widewidey");
  p.SaveCanvas("W_m");

  p.Setup(2);
  p.AddPlot("t_pt","rebin:5 2:base:0");
  p.SaveCanvas("t_pt");

  p.Setup(3);
  p.AddPlot("lldelphi","2:base:0 1:bottomleg");
  p.AddPlot("t_pt","rebin:5 2:base:0");
  p.SaveCanvas("lldelphi");
  p.SaveCanvas("t_pt");
}
