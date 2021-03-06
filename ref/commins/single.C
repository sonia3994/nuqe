//--implementing pages 297-8 from Commins + Bucksbaum

#include <iostream>
#include "Rtypes.h"
#include "TMath.h"
#include "TGraph.h"
#include "TFile.h"

Double_t d_sigma_d_Q2(Double_t w,Double_t q2);
Double_t d_sigma_d_E_mu(Double_t E_nu,Double_t E_mu);
Double_t f1(Double_t q2);
Double_t f2(Double_t q2);
Double_t g1(Double_t q2);

using namespace std;

///////////////////////////////////////////////////////////
Int_t main(Int_t argc,char **argv) {

  TFile f("single.root","RECREATE");

  const Int_t NE_nu=20;
  const Int_t N_points=2000;

  Double_t E_nu_min=0.8;
  Double_t E_nu_max=40.8;
  Double_t E_nu[NE_nu];
  Double_t XS[NE_nu];

  for (Int_t iE_nu=0;iE_nu<NE_nu;iE_nu++) {

    E_nu[iE_nu]=E_nu_min + (E_nu_max-E_nu_min)*iE_nu/NE_nu;

    Double_t    Q2[N_points];
    Double_t  E_mu[N_points];
    Double_t xs_Q2  [N_points];
    Double_t xs_E_mu[N_points];

    XS[iE_nu]=0.0;
    for (Int_t iPoint=0;iPoint<N_points;iPoint++) {
      Double_t Q2_min=0.0;
      Double_t Q2_max=2.0;

      Double_t E_mu_min=0.0;//E_nu[iE_nu]*7.0/8.0;
      Double_t E_mu_max=E_nu[iE_nu];

      Q2  [iPoint]=Q2_min   + (Q2_max-Q2_min)*iPoint/N_points;
      E_mu[iPoint]=E_mu_min + (E_mu_max-E_mu_min)*iPoint/N_points;

      xs_Q2  [iPoint]=0.389379e-27*d_sigma_d_Q2(E_nu[iE_nu],-Q2[iPoint]);
      xs_E_mu[iPoint]=0.389379e-27*d_sigma_d_E_mu(E_nu[iE_nu],E_mu[iPoint]);

      XS[iE_nu]+=xs_E_mu[iPoint];
    }
    XS[iE_nu]*=(E_mu[N_points-1]-E_mu[0])/N_points;


    TGraph gr_Q2(N_points,Q2,xs_Q2);
    gr_Q2.SetName(Form("xs_Q2_%d",iE_nu));
    gr_Q2.SetTitle(Form("E_{#nu}=%g GeV;Q^{2} (GeV^{2});d#sigma/dQ^{2} (cm^{2}/GeV^{2})",E_nu[iE_nu]));
    gr_Q2.Write();

    TGraph gr_E_mu(N_points,E_mu,xs_E_mu);
    gr_E_mu.SetName(Form("xs_E_mu_%d",iE_nu));
    gr_E_mu.SetTitle(Form("E_{#nu}=%g GeV;E_{#mu} (GeV);d#sigma/dE_{#mu} (cm^{2}/GeV)",E_nu[iE_nu]));
    gr_E_mu.Write();

  }

  TGraph gr_XS(NE_nu,E_nu,XS);
  gr_XS.SetName("XS");
  gr_XS.SetTitle(";E_{#nu} (GeV);#sigma (cm^{2})");
  gr_XS.Write();

  f.Close();

  return 0;
}

///////////////////////////////////////////////////////////
Double_t d_sigma_d_E_mu(Double_t E_nu,Double_t E_mu) {
  const Double_t M_p=0.93827;
  
  Double_t Q2=2.0*M_p*(E_nu-E_mu);
  Double_t ans=d_sigma_d_Q2(E_nu,-Q2)*(2.0*M_p);
  return ans;
}
///////////////////////////////////////////////////////////
Double_t d_sigma_d_Q2(Double_t w,Double_t q2) {

  //from PDG
  const Double_t M_p=0.93827;
  const Double_t GF=1.1664e-5;
  const Double_t coscab=0.974;

  Double_t ans=0.0;

  ans += pow(f1(q2),2)*(1.0 + q2/2.0/M_p/w + q2/4.0/w/w + q2*q2/8.0/M_p/M_p/w/w);
  ans += pow(f2(q2),2)*(-q2 - q2*q2/2.0/M_p/w + q2*q2/4.0/w/w);
  ans += pow(g1(q2),2)*(1.0 + q2/2.0/M_p/w + q2/4.0/w/w + q2*q2/8.0/M_p/M_p/w/w);
  ans += f1(q2)*f2(q2)*q2*q2/2.0/M_p/w/w;
  ans -= f1(q2)*g1(q2)*(q2/M_p/w + q2*q2/4.0/M_p/M_p/w/w);
  ans -= f2(q2)*g1(q2)*(2.0*q2/w + q2*q2/4.0/M_p/w/w);

  ans*=GF*GF*coscab*coscab/2.0/TMath::Pi();
  return ans;
}

///////////////////////////////////////////////////////////
Double_t f1(Double_t q2) {
  const Double_t MV=0.84;
  const Double_t M_p=0.93827;

  return (1.0 - 4.70*q2/4.0/M_p/M_p)/(1.0-q2/4.0/M_p/M_p)/pow(1.0-q2/MV/MV,2);
}

///////////////////////////////////////////////////////////
Double_t f2(Double_t q2) {
  const Double_t MV=0.84;
  const Double_t M_p=0.93827;

  return (3.70)/(1.0-q2/4.0/M_p/M_p)/pow(1.0-q2/MV/MV,2);
}

///////////////////////////////////////////////////////////
Double_t g1(Double_t q2) {
  const Double_t MA=1.03;
  return (1.25)/pow(1.0-q2/MA/MA,2);
}
