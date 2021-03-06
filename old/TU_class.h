#ifndef TU_class_h
#define TU_class_h

#include <iostream>
#include <complex>
#include "Rtypes.h"
#include "TT_param_class.h"
#include "TTree.h"

using namespace std;

class TU_class {
 public :

  TT_param_class *f_params;

  Double_t f_k_lower[4];
  Double_t f_k_upper[4];

  Double_t f_q_lower[4];
  Double_t f_q_upper[4];

  Double_t f_kprime_lower[4];
  Double_t f_kprime_upper[4];

  Double_t f_Re_L_lower[4][4];  
  Double_t f_Im_L_lower[4][4];  
  
  Double_t f_d3sigma_d3kprime;
  Double_t f_d3sigma_d3kprime_err;
  Int_t    f_d3sigma_d3kprime_forbidden;
  Int_t    f_d3sigma_d3kprime_problems;

  Double_t f_d2sigma_de_dcostheta;
  Double_t f_d2sigma_de_dcostheta_err;

  //TU_class(TT_param_class *params,Double_t Enu,Double_t E_lepprime);
  TU_class(TT_param_class *params,Double_t Enu,Double_t E_lepprime,TTree *tree,Double_t* tree_E_lepprime,Double_t *tree_p_lower,Int_t *tree_problem);
  Double_t *f_tree_E_lepprime;
  Double_t *f_tree_p_lower;
  Int_t    *f_tree_problem;
  TTree    *f_tree;
  virtual ~TU_class();

  virtual void Init_L();

  virtual void     Compute_XS();
  virtual void     Integrate();
  virtual Bool_t     Determine_kprime(Double_t p_lower[4]);
  virtual Double_t Evaluate_integrand(Double_t E,Double_t p_lower[4]);
  virtual Double_t SF_SM(Double_t E,Double_t p_lower[4]);
};

#endif

#ifdef TU_class_cxx
TU_class::TU_class(TT_param_class *params,Double_t Enu,Double_t E_lepprime,TTree *tree,Double_t *tree_E_lepprime,Double_t *tree_p_lower,Int_t *tree_problem)
{
  f_tree=tree;
  f_tree_E_lepprime=tree_E_lepprime;
  f_tree_p_lower=tree_p_lower;
  f_tree_problem=tree_problem;
  f_params = params;
  f_k_lower[0]=Enu;
  f_k_lower[1]=0.0;
  f_k_lower[2]=0.0;
  f_k_lower[3]=Enu;
  f_kprime_lower[0]=E_lepprime;
  f_q_lower[0]=Enu-E_lepprime;

  f_d3sigma_d3kprime_forbidden=0;
  f_d3sigma_d3kprime_problems=0;
}

TU_class::~TU_class() {
}

////////////////////////////////////////////////////////////////////////
void TU_class::Init_L()
{

  Double_t kk=0.0;
  for (Int_t i=0;i<4;i++) kk+= f_k_lower[i]*f_kprime_upper[i];

  for (Int_t mu=0;mu<4;mu++) {
    for (Int_t nu=0;nu<4;nu++) {
      f_Re_L_lower[mu][nu] = 2.0*(f_k_lower[mu]*f_kprime_lower[nu]+f_kprime_lower[mu]*f_k_lower[nu] - kk*f_params->f_g[mu][nu]);
      f_Im_L_lower[mu][nu] = 0.0;
      for (Int_t rho=0;rho<4;rho++) {
        for (Int_t sigma=0;sigma<4;sigma++) {
          f_Im_L_lower[mu][nu] += -2.0*f_params->f_epsilon_lower[mu][nu][rho][sigma]*f_k_upper[rho]*f_kprime_upper[sigma];
        }
      }
      //cout << "f_Im_L_lower[" << mu << "][" << nu << "]: " << f_Im_L_lower[mu][nu] << endl;
    }
  }
}

#endif // #ifdef TU_class_cxx

