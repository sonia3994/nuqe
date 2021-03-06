#include <iostream>
#include "TFile.h"
#include "TH1F.h"

Int_t make_flux(void) {

  Int_t energy_point=5000;
  TFile ff(Form("flux_%d.root",energy_point),"RECREATE");

  Float_t epsilon=1.0e-4;
  Float_t float_e=energy_point/1.0e3;
  cout << "1=" << float_e+epsilon << endl;
  //TH1F flux("flux","",1,0.299999,0.3000001);
  TH1F flux("flux","",1,float_e-epsilon,float_e+epsilon);
  
  for (Int_t iBin=0;iBin<flux.GetNbinsX();iBin++) {
    //Double_t content=TMath::Exp(-(iBin+0.0)/flux.GetNbinsX()*2.0);
    Float_t content=1.0;
    flux.SetBinContent(iBin+1,content);
  }
  
  flux.Scale(1.0/flux.Integral("width"));
  flux.DrawCopy();
  flux.Write();

  ff.Close();

  return 0;
}
