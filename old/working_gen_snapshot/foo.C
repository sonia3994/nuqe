//implementing nucl-th/0512004v4
// ./m to compile

////list of ambiguities/issues
//--de Forest prescription
//--ksi in F2?
//--Mpi+-0?
//--poles in Smith-Moniz form factors?
//--M value fixed?
//--use delta function to eliminate the integral over E?
//--overall sign of epsilon
//--epsilon upper/lower -1 proof
//--what if contraction is complex?
//--args, options, seed
//--form factors
//--integrate over a flux?
//--nuclear magneton
//--hadronic tensor (M!=MA?)
//--proton mass!=neutron mass
//--generalize target and recoil masses (call M_prime) for e scattering?
//--average e_bind for SM
//--Double_t ok?
//--direction of q?
//--negative E/limits of integration for E?
//----has implications for norm. of SM spectral function
//----E lower threshold for binding e near 0
//--does having two p_mag solutions make sense?
//--virtual functions?
//--check cos_theta_pq uniformity
//--single nucleon lower energy thershold
//--overall factor of M/(M+w) between Katori and polish limit?
//--Llewellyn-Smith does not make sense at high Q2
//
//--for (pf,eb)=(0.001,0.0), nothing      >
//--for (pf,eb)=(0.03,0.0), looks good    >  understand
//--for (pf,eb)=(0.225,0.0), very little  >
//--for (pf,eb)=(0.225,0.025), seems ok   >
//--seems caused by a N_W that's too small
//
//--non-PB SM case disagrees at high E_mu
//--W boson mass? (and, perhaps cause of NUANCE disagreement)
//--FP included in NUANCE?
//--how to find p_max for spectral functions
//--fix double solutions for p

#include <iostream>
#include "Rtypes.h"
#include "TRandom3.h"
#include "TGraph2D.h"
#include "TGraphErrors.h"
#include "TGraph.h"
#include "TFile.h"
#include "TTree.h"
#include "TT_param_class.h"
#include "TV_class.h"
#include "TT_nucleus.h"
#include "TT_surface.h"

using namespace std;

Int_t gen(Double_t,TT_param_class*,TT_nucleus*);
Int_t test_algo(Double_t,TT_param_class*,TT_nucleus*);
Int_t xs_e(TT_param_class*);
Int_t xs_w_q(TT_param_class*);
Int_t xs2_w_q(TT_param_class*);
Int_t xs_w_Q2(TT_param_class*,TT_nucleus*);
Int_t xs_Q2(TT_param_class*);

////////////////////////////////////////////////////////////////////////
Int_t main(Int_t argc,char **argv) {
  Bool_t help=0;

  if (help) {
    cout << "--USAGE: " << argv[0] << " CONFIG_FILE" << endl;
    cout << "--for further help, see readme" << endl;
    return 0;
  }
  
  //read in config file
  TT_param_class *params = new TT_param_class(argv[1]);
  TT_nucleus *nucleus = new TT_nucleus(8,8);

  //xs_w_Q2(params,nucleus);

  gen(0.8,params,nucleus);

  //TT_nucleus nuc(8,8);
  //nuc.n_plot();

  delete params;
  delete nucleus;
  return 0;
}

////////////////////////////////////////////////////////////////////////
Int_t gen(Double_t Enu,TT_param_class *params,TT_nucleus *nucleus) {

  //make a "surface" for importance sampling
  TT_surface surf(Enu,params,nucleus,4);
  surf.Init_surface();
  cout << "a" << endl;
  surf.Fix_surface(4);//4, eventually
  cout << "b" << endl;
  surf.Adjust_surface_with_neighbors();
  cout << "c" << endl;
  surf.Test_surface_randomly(16);
  cout << "d" << endl;
  surf.Compute_volume();
  cout << "e" << endl;

  TTree *tree=0;
  Double_t tree_xs;
  Double_t tree_w;
  Double_t tree_q_bold;
  Double_t tree_cth;
  Double_t tree_phi;
  Double_t tree_inv_weight;

  if (tree) delete tree;
  tree=new TTree("tree","");
  tree->Branch("xs_branch",&tree_xs,"xs/D");
  tree->Branch("w_branch",&tree_w,"w/D");
  tree->Branch("q_bold_branch",&tree_q_bold,"q_bold/D");
  tree->Branch("cth_branch",&tree_cth,"cth/D");
  tree->Branch("phi_branch",&tree_phi,"phi/D");
  tree->Branch("inv_weight_branch",&tree_inv_weight,"inv_weight/D");
  //tree->Branch("q_lower_branch",tree_q_lower,"q_lower[4]/D");
  //tree->Branch("k_lower_branch",tree_k_lower,"k_lower[4]/D");
  //tree->Branch("kprime_lower_branch",tree_kprime_lower,"kprime_lower[4]/D");

  Int_t accepted_points=0;
  Int_t total_points=0;
  while (accepted_points<100000) {
    total_points++;
    Double_t w,q_bold,cth,phi,xs,inv_weight;
    surf.Draw_point(w,q_bold,cth,phi,xs,inv_weight);

    if (xs>0.0) {
      accepted_points++;

      tree_xs=xs;
      tree_w=w;
      tree_q_bold=q_bold;
      tree_cth=cth;
      tree_phi=phi;
      tree_inv_weight=inv_weight;
      tree->Fill();
    }
  }

  Double_t eff=(accepted_points+0.0)/total_points;

  Double_t unit_conv=0.389379e-27;  

  const Int_t array_length=1;
  Double_t E_nu[array_length];
  E_nu[0]=Enu;
  Double_t xs_total[array_length];
  xs_total[0]=eff*surf.f_total_volume*unit_conv;
  Double_t xs_error[array_length];
  xs_error[0]=sqrt(eff*(1.0-eff)/total_points)*surf.f_total_volume*unit_conv;

  printf("total_volume: %6.4g, eff.: %6.4g, xs: %6.4g, xs_error: %6.4g\n",surf.f_total_volume,eff,xs_total[0],xs_error[0]);

  TGraph graph_xs      (array_length,E_nu,xs_total);
  TGraph graph_xs_error(array_length,E_nu,xs_error);

  graph_xs.SetName("graph_xs");
  graph_xs_error.SetName("graph_xs_error");

  TFile f("~elaird/ccqe/xs_ted/xs.root","RECREATE");
  tree->Write();
  graph_xs.Write();
  graph_xs_error.Write();

  surf.f_volume_histo->Write();
  surf.f_volume_check_histo->Write();
  f.Close();

  return 0;
}

////////////////////////////////////////////////////////////////////////
Int_t test_algo(Double_t Enu,TT_param_class *params,TT_nucleus *nucleus) {
  TT_surface surf(Enu,params,nucleus,4);
  surf.Init_surface();
  surf.Fix_surface(4);
  surf.Adjust_surface_with_neighbors();

  const Int_t iterations=3;

  Double_t zero[iterations];
  Double_t prob[iterations];

  cout << "a" << endl;
  zero[0]=surf.Count_surface_zeroes();
  prob[0]=surf.Test_surface_randomly(5*5*5*5);

  surf.Fix_surface(2);
  surf.Adjust_surface_with_neighbors();

  zero[1]=surf.Count_surface_zeroes();
  prob[1]=surf.Test_surface(3);
  
  //zero[2]=surf.Count_surface_zeroes();
  //prob[2]=surf.Test_surface(4);
  
  for (Int_t it=0;it<iterations;it++){
    printf("zero[%d]=%5.3g; prob[%d]=%5.3g\n",it,zero[it],it,prob[it]);
  }

  return 0;
}

//////
//////////////////////////////////////////////////////////////////////////////
//////Int_t gen_old(TT_param_class *params) {
//////
//////  Double_t Enu=0.8;
//////
//////  const Double_t unit_conv=0.389379e-27;  
//////
//////  Double_t w_min=0.0;
//////  Double_t w_max=Enu-params->f_m_lep;
//////  Double_t q_bold_min=0.0;
//////  Double_t q_bold_max=2.0;//4.0*params->f_M*w;//do a better job here
//////
//////  TRandom3 rand;
//////  rand.SetSeed(0);
//////
//////  TTree *tree=0;
//////  Double_t tree_q_lower[4];
//////  Double_t tree_k_lower[4];
//////  Double_t tree_kprime_lower[4];
//////  Double_t tree_xs;
//////
//////  Int_t N_Events=200;
//////  Int_t iEvent=0;
//////  Double_t min=-80.0;
//////  Double_t max_log_xs=min;
//////
//////  while (iEvent<N_Events) {
//////
//////    Double_t q_bold=rand.Uniform(q_bold_min,q_bold_max);
//////    Double_t w=rand.Uniform(w_min,w_max);
//////    Double_t cth=rand.Uniform(-1.0,1.0);
//////    Double_t phi=rand.Uniform(-TMath::Pi(),TMath::Pi());
//////
//////    Double_t p_lep_sq=pow(Enu-w,2)-pow(params->f_m_lep,2);
//////    Double_t cos_theta_q  =(pow(Enu,2)-p_lep_sq+pow(q_bold,2))/(2.0*Enu*q_bold);
//////    Double_t cos_theta_lep=(pow(Enu,2)+p_lep_sq-pow(q_bold,2))/(2.0*Enu*sqrt(p_lep_sq));
//////    if (TMath::Abs(cos_theta_q)>1.0 || TMath::Abs(cos_theta_lep)>1.0) continue;
//////
//////    Double_t q_lower[4];
//////    q_lower[0]=w;
//////    q_lower[1]=0.0;
//////    q_lower[2]=-q_bold*sqrt(1.0-pow(cos_theta_q,2));
//////    q_lower[3]=q_bold*cos_theta_q;
//////    Double_t asdf=pow(q_lower[0],2)-pow(q_lower[1],2)-pow(q_lower[2],2)-pow(q_lower[3],2);
//////
//////    TV_class foo(params,Enu,q_lower,cth,phi);
//////    foo.Evaluate_d4sigma_dw_dq_bold_domega_p();
//////
//////    Double_t log_xs;
//////    if (foo.f_d4sigma_dw_dq_bold_domega_p>0.0) log_xs=TMath::Log(foo.f_d4sigma_dw_dq_bold_domega_p);
//////    else log_xs=min;
//////
//////    if (log_xs>max_log_xs) {
//////      Double_t old_max=max_log_xs;
//////      max_log_xs=log_xs+1.0;
//////      cout << "old_max,new_max: " << old_max << "," << max_log_xs << endl;
//////      iEvent=0;
//////
//////      if (tree) delete tree;
//////      tree=new TTree("tree","");
//////      tree->Branch("xs_branch",&tree_xs,"xs/D");
//////      tree->Branch("q_lower_branch",tree_q_lower,"q_lower[4]/D");
//////      tree->Branch("k_lower_branch",tree_k_lower,"k_lower[4]/D");
//////      tree->Branch("kprime_lower_branch",tree_kprime_lower,"kprime_lower[4]/D");
//////    }
//////
//////    else {
//////      Double_t value=rand.Uniform(0.0,max_log_xs);
//////
//////      if (value<=log_xs) {
//////	iEvent++;
//////	for (Int_t i=0;i<4;i++) {
//////	  tree_q_lower[i]=foo.f_q_lower[i];
//////	  tree_k_lower[i]=foo.f_k_lower[i];
//////	  tree_kprime_lower[i]=foo.f_kprime_lower[i];
//////	}
//////	tree_xs=foo.f_d4sigma_dw_dq_bold_domega_p;
//////	tree->Fill();
//////      }
//////    }
//////  } //end while
//////  
//////  TFile f("~elaird/ccqe/xs_ted/xs.root","RECREATE");
//////  tree->Write();
//////  f.Close();
//////
//////  return 0;
//////}
//////

////////////////////////////////////////////////////////////////////////
Int_t xs_w_Q2(TT_param_class *params,TT_nucleus *nucleus) {

  Double_t Enu=0.8;

  const Double_t unit_conv=0.389379e-27;  

  const Int_t N_W=500;
  const Int_t N_Q2=150;
  
  Double_t *w       = new Double_t[N_W*N_Q2];
  Double_t *Q2      = new Double_t[N_W*N_Q2];
  Double_t *E       = new Double_t[N_W*N_Q2];
  Double_t *C       = new Double_t[N_W*N_Q2];
  Double_t *XS2d    = new Double_t[N_W*N_Q2];
  Double_t *XS2d_err= new Double_t[N_W*N_Q2];

  Double_t Q21d[N_Q2];
  Double_t XS1d[N_Q2];
  Double_t XS1d_err[N_Q2];
  
  Double_t Q2_min=0.0;
  //Double_t Q2_max=2.0*params->f_M*w_max;
  Double_t Q2_max=1.4;

  for (Int_t iQ2=0;iQ2<N_Q2;iQ2++) {
    XS1d[iQ2]=0.0;
    Double_t w_min=0.0;
    Double_t w_max=Enu-params->f_m_lep;

    //Double_t Q2_local=iQ2*(Q2_max-Q2_min)/N_Q2+Q2_min;
    //Double_t possible_min=Q2_local/(2.0*params->f_M)-0.05;
    //if (possible_min>w_min) w_min=possible_min;
    //Double_t possible_max=Q2_local/(2.0*params->f_M)+0.05;
    //if (possible_max<w_max) w_max=possible_max;

    for (Int_t iW=0;iW<N_W;iW++) {
    
      Int_t index=iQ2*N_W+iW;
      XS2d    [index]=0.0;
      XS2d_err[index]=0.0;
    
      w[index]=iW*(w_max-w_min)/N_W+w_min;
      Q2[index]=iQ2*(Q2_max-Q2_min)/N_Q2+Q2_min;
    
      Double_t q_bold_mag=sqrt(Q2[index]+w[index]*w[index]);
      Double_t p_lep_sq=pow(Enu-w[index],2)-pow(params->f_m_lep,2);
      Double_t cos_theta_q=(pow(Enu,2)-p_lep_sq+pow(q_bold_mag,2))/(2.0*Enu*q_bold_mag);
      if (cos_theta_q>1.0) continue;
      Double_t sin_theta_q=sqrt(1.0-pow(cos_theta_q,2));
      //Double_t sin_theta_lep=-Q2[index]*sin_theta_q/sqrt(p_lep_sq);
      //printf("w=%5.3g; q=%5.3g; c_q=%5.3g; s_l=%5.3g\n",w[index],Q2[index],cos_theta_q,sin_theta_q);
    
      Double_t q_lower[4];
      q_lower[0]=w[index];
      q_lower[1]=0.0;
      q_lower[2]=q_bold_mag*sin_theta_q;
      q_lower[3]=q_bold_mag*cos_theta_q;
    
      TV_class foo(params,nucleus,Enu,q_lower);
      foo.Compute_XS();
      
      Double_t jacobian=0.5/q_bold_mag;
      XS2d    [index]=unit_conv*jacobian*foo.f_d2sigma_dw_dq;
      XS2d_err[index]=unit_conv*jacobian*foo.f_d2sigma_dw_dq_err;

      //printf("index=%5.3d; Q2=%5.3g; w=%5.3g; xs=%5.3g\n",index,Q2[index],w[index],XS2d[index]);

      ////Riemann sum
      //XS1d[iQ2] += XS2d[index]*(w_max-w_min)/N_W;

      //Simpson's rule
      Double_t coeff;
      if (iW==0 || iW==N_W-1) {
        coeff=1.0;
      }
      else if (iW%2==0) {
        coeff=2.0;
      }
      else {
        coeff=4.0;
      }
      //cout << iW << "," << coeff << endl;
      XS1d[iQ2]+= coeff*XS2d[index]*(w_max-w_min)/(3.0*(N_W-1));

      //if (XS2d[index]>0.0) printf("enuqe=%5.3g; E=%5.3g; C=%5.3g; XS=%5.3g; err=%5.3g; \n",foo.SM_compute_enuqe(),w[index],Q2[index],XS2d[index],XS2d_err[index]);
      //printf("enuqe=%5.3g; E=%5.3g; C=%5.3g; XS=%5.3g; err=%5.3g; \n",foo.SM_compute_enuqe(),w[index],Q2[index],XS2d[index],XS2d_err[index]);
    }

    Q21d[iQ2]=iQ2*(Q2_max-Q2_min)/N_Q2+Q2_min;
    printf("Q2=%5.3g;xs=%5.3g\n",Q21d[iQ2],XS1d[iQ2]);
  }

  TGraph xs1d_graph(N_Q2,Q21d,XS1d);
  xs1d_graph.SetName("xs1d");
  xs1d_graph.SetTitle(";Q^{2} (GeV^{2});d#sigma/dQ^{2} (cm^{2}/GeV^{2})");
  
  TGraph2D xs2d_graph(N_W*N_Q2,Q2,w,XS2d);
  xs2d_graph.SetName("xs2d");
  xs2d_graph.SetTitle(";E_{#mu} (GeV);cos(#theta_{#mu#nu});d#sigma/dE_{#mu}dcos(#theta) (cm^{2}/GeV)");
  TGraph2D xs2d_err_graph(N_W*N_Q2,E,C,XS2d_err);
  xs2d_err_graph.SetName("xs2d_errs");
  xs2d_err_graph.SetTitle(";x-title;y-title;z-title");
  TFile f("~elaird/ccqe/xs_ted/xs.root","RECREATE");
  xs1d_graph.Write();
  xs2d_graph.Write();
  xs2d_err_graph.Write();
  f.Close();

  delete [] w       ;
  delete [] Q2      ;
  delete [] E       ;
  delete [] C       ;
  delete [] XS2d    ;
  delete [] XS2d_err;

  return 0;
}

//////////////////////////////////////////////////////////////////////////////
//////Int_t xs_w_q(TT_param_class *params) {
//////
//////  Double_t Enu=0.8;
//////
//////  const Double_t unit_conv=0.389379e-27;  
//////
//////  Int_t N_E=200;
//////  Int_t N_C=200;
//////  
//////  Double_t      w[N_E*N_C];
//////  Double_t q_bold[N_E*N_C];
//////  Double_t  E[N_E*N_C];
//////  Double_t  C[N_E*N_C];
//////  Double_t XS2d[N_E*N_C];
//////  Double_t XS2d_err[N_E*N_C];
//////
//////  Double_t  E1d[N_E];
//////  Double_t XS1d[N_E];
//////  Double_t XS1d_err[N_E];
//////  
//////  Double_t E_min=params->f_m_lep;
//////  Double_t E_max=Enu;
//////  Double_t C_min=-1.0;
//////  Double_t C_max=1.0;
//////
//////  for (Int_t iE=0;iE<N_E;iE++) {
//////    XS1d[iE]=0.0;
//////    for (Int_t iC=0;iC<N_C;iC++) {
//////  
//////      Int_t index=iE*N_C+iC;
//////      E[index]=iE*(E_max-E_min)/N_E+E_min;
//////      C[index]=iC*(C_max-C_min)/N_C+C_min;
//////
//////      Double_t P=sqrt(pow(E[index],2)-pow(params->f_m_lep,2));
//////      Double_t q_lower[4];
//////      q_lower[0]=Enu-E[index];
//////      q_lower[1]=0.0;
//////      q_lower[2]=-P*sqrt(1.0-pow(C[index],2));
//////      q_lower[3]=Enu-P*C[index];
//////
//////      w[index]=q_lower[0];
//////      q_bold[index]=0.0;
//////      for (Int_t i=1;i<4;i++) q_bold[index]+=pow(q_lower[i],2);
//////      q_bold[index]=sqrt(q_bold[index]);
//////
//////      XS2d    [index]=0.0;
//////      XS2d_err[index]=0.0;
//////
//////      TV_class foo(params,Enu,q_lower);
//////      Double_t var=TMath::Abs(Enu-foo.SM_compute_enuqe())/Enu;
//////      if (var<0.05) {
//////	foo.Compute_XS();
//////	
//////	XS2d    [index]=unit_conv*foo.f_d2sigma_dw_dq;
//////	XS2d_err[index]=unit_conv*foo.f_d2sigma_dw_dq_err;
//////
//////	Double_t jacobian=Enu*P/sqrt(Enu*Enu-2.0*Enu*P*C[index]+P*P);
//////	XS1d[iE] += jacobian*XS2d[index]*(C_max-C_min)/N_C;
//////      }
//////      //if (XS2d[index]>0.0) printf("enuqe=%5.3g; E=%5.3g; C=%5.3g; XS=%5.3g; err=%5.3g; \n",foo.SM_compute_enuqe(),E[index],C[index],XS2d[index],XS2d_err[index]);
//////      //printf("enuqe=%5.3g; E=%5.3g; C=%5.3g; XS=%5.3g; err=%5.3g; \n",foo.SM_compute_enuqe(),E[index],C[index],XS2d[index],XS2d_err[index]);
//////    }
//////    E1d[iE]=E[iE*N_C];
//////    cout << "E=" << E1d[iE] << endl;
//////  }
//////
//////  TGraph xs1d_graph(N_E,E1d,XS1d);
//////  xs1d_graph.SetName("xs1d");
//////  xs1d_graph.SetTitle(";E_{#mu} (GeV);d#sigma/dE_{#mu} (cm^{2}/GeV)");
//////  
//////  TGraph2D xs2d_graph(N_E*N_C,E,C,XS2d);
//////  xs2d_graph.SetName("xs2d");
//////  xs2d_graph.SetTitle(";E_{#mu} (GeV);cos(#theta_{#mu#nu});d#sigma/dE_{#mu}dcos(#theta) (cm^{2}/GeV)");
//////  TGraph2D xs2d_err_graph(N_E*N_C,E,C,XS2d_err);
//////  xs2d_err_graph.SetName("xs2d_errs");
//////  xs2d_err_graph.SetTitle(";x-title;y-title;z-title");
//////  TFile f("~elaird/ccqe/xs_ted/xs.root","RECREATE");
//////  xs1d_graph.Write();
//////  xs2d_graph.Write();
//////  xs2d_err_graph.Write();
//////  f.Close();
//////
//////  return 0;
//////}
//////
//////////////////////////////////////////////////////////////////////////////
//////Int_t xs2_w_q(TT_param_class *params) {
//////
//////  Double_t Enu=0.8;
//////
//////  const Double_t unit_conv=0.389379e-27;  
//////
//////  Int_t N_W=200;
//////  Int_t N_Q_BOLD=200;
//////  
//////  Double_t      w[N_W*N_Q_BOLD];
//////  Double_t q_bold[N_W*N_Q_BOLD];
//////  Double_t  E[N_W*N_Q_BOLD];
//////  Double_t  C[N_W*N_Q_BOLD];
//////  Double_t XS2d[N_W*N_Q_BOLD];
//////  Double_t XS2d_err[N_W*N_Q_BOLD];
//////
//////  Double_t  w1d[N_W];
//////  Double_t XS1d[N_W];
//////  Double_t XS1d_err[N_W];
//////  
//////  Double_t w_min=0.0;
//////  Double_t w_max=Enu-params->f_m_lep;
//////
//////  for (Int_t iW=0;iW<N_W;iW++) {
//////    XS1d[iW]=0.0;
//////    for (Int_t iQ_BOLD=0;iQ_BOLD<N_Q_BOLD;iQ_BOLD++) {
//////  
//////      Int_t index=iW*N_Q_BOLD+iQ_BOLD;
//////      XS2d    [index]=0.0;
//////      XS2d_err[index]=0.0;
//////
//////      w[index]=iW*(w_max-w_min)/N_W+w_min;
//////
//////      Double_t q_bold_min=0.0;
//////      Double_t q_bold_max=2.0*sqrt(pow(w[index],2)+2.0*params->f_M*w[index]);
//////
//////      q_bold[index]=(iQ_BOLD+1)*(q_bold_max-q_bold_min)/N_Q_BOLD+q_bold_min;
//////
//////      Double_t p_lep_sq=pow(Enu-w[index],2)-pow(params->f_m_lep,2);
//////      Double_t cos_theta_q=(pow(Enu,2)-p_lep_sq+pow(q_bold[index],2))/(2.0*Enu*q_bold[index]);
//////      if (cos_theta_q>1.0) continue;
//////      Double_t sin_theta_q=sqrt(1.0-pow(cos_theta_q,2));
//////      Double_t sin_theta_lep=-q_bold[index]*sin_theta_q/sqrt(p_lep_sq);
//////      //printf("w=%5.3g; q=%5.3g; c_q=%5.3g; s_l=%5.3g\n",w[index],q_bold[index],cos_theta_q,sin_theta_q);
//////
//////      Double_t q_lower[4];
//////      q_lower[0]=w[index];
//////      q_lower[1]=0.0;
//////      q_lower[2]=q_bold[index]*sin_theta_q;
//////      q_lower[3]=q_bold[index]*cos_theta_q;
//////
//////
//////      TV_class foo(params,Enu,q_lower);
//////      {
//////	foo.Compute_XS();
//////	
//////	XS2d    [index]=unit_conv*foo.f_d2sigma_dw_dq;
//////	XS2d_err[index]=unit_conv*foo.f_d2sigma_dw_dq_err;
//////
//////	XS1d[iW] += XS2d[index]*(q_bold_max-q_bold_min)/N_Q_BOLD;
//////      }
////////      //if (XS2d[index]>0.0) printf("enuqe=%5.3g; E=%5.3g; C=%5.3g; XS=%5.3g; err=%5.3g; \n",foo.SM_compute_enuqe(),w[index],q_bold[index],XS2d[index],XS2d_err[index]);
////////      //printf("enuqe=%5.3g; E=%5.3g; C=%5.3g; XS=%5.3g; err=%5.3g; \n",foo.SM_compute_enuqe(),w[index],q_bold[index],XS2d[index],XS2d_err[index]);
//////    }
//////    w1d[iW]=w[iW*N_Q_BOLD];
//////    cout << "w=" << w1d[iW] << endl;
//////  }
//////
//////  TGraph xs1d_graph(N_W,w1d,XS1d);
//////  xs1d_graph.SetName("xs1d");
//////  xs1d_graph.SetTitle(";#omega (GeV);d#sigma/d#omega} (cm^{2}/GeV)");
//////  
//////  TGraph2D xs2d_graph(N_W*N_Q_BOLD,E,C,XS2d);
//////  xs2d_graph.SetName("xs2d");
//////  xs2d_graph.SetTitle(";E_{#mu} (GeV);cos(#theta_{#mu#nu});d#sigma/dE_{#mu}dcos(#theta) (cm^{2}/GeV)");
//////  TGraph2D xs2d_err_graph(N_W*N_Q_BOLD,E,C,XS2d_err);
//////  xs2d_err_graph.SetName("xs2d_errs");
//////  xs2d_err_graph.SetTitle(";x-title;y-title;z-title");
//////  TFile f("~elaird/ccqe/xs_ted/xs.root","RECREATE");
//////  xs1d_graph.Write();
//////  xs2d_graph.Write();
//////  xs2d_err_graph.Write();
//////  f.Close();
//////
//////  return 0;
//////}
//////
//////////////////////////////////////////////////////////////////////////////
//////Int_t xs_e(TT_param_class *params) {
//////
//////  Double_t Enu=0.8;
//////
//////  const Double_t unit_conv=0.389379e-27;  
//////
//////  const Int_t N_q_bold=500;
//////  const Int_t N_E=150;
//////  
//////  Double_t E[N_E];
//////  Double_t XS_E[N_E];
//////  
//////  Double_t E_min=params->f_m_lep;
//////  Double_t E_max=Enu;
//////  //Double_t E_min=0.75;//params->f_m_lep;
//////  //Double_t E_max=0.805;//Enu;
//////
//////  for (Int_t iE=0;iE<N_E;iE++) {
//////    E[iE]=iE*(E_max-E_min)/N_E+E_min;
//////    XS_E[iE]=0.0;
//////
//////    Double_t q_bold_min=0.0;
//////    Double_t q_bold_max=2.0;//do a better job here
//////
//////    for (Int_t iq_bold=0;iq_bold<N_q_bold;iq_bold++) {
//////    
//////      Double_t q_bold=iq_bold*(q_bold_max-q_bold_min)/N_q_bold+q_bold_min;
//////      Double_t p_lep_sq=pow(E[iE],2)-pow(params->f_m_lep,2);
//////      Double_t cos_theta_q  =(pow(Enu,2)-p_lep_sq+pow(q_bold,2))/(2.0*Enu*q_bold);
//////      Double_t cos_theta_lep=(pow(Enu,2)+p_lep_sq-pow(q_bold,2))/(2.0*Enu*sqrt(p_lep_sq));
//////      if (TMath::Abs(cos_theta_q)>1.0 || TMath::Abs(cos_theta_lep)>1.0) continue;
//////
//////      Double_t q_lower[4];
//////      q_lower[0]=Enu-E[iE];
//////      q_lower[1]=0.0;
//////      q_lower[2]=-q_bold*sqrt(1.0-pow(cos_theta_q,2));
//////      q_lower[3]=q_bold*cos_theta_q;
//////      Double_t asdf=pow(q_lower[0],2)-pow(q_lower[1],2)-pow(q_lower[2],2)-pow(q_lower[3],2);
//////      if (asdf>0.0) cout << "q2=" << asdf << endl;
//////      TV_class foo(params,Enu,q_lower);
//////      foo.Compute_XS();
//////
//////      ////Riemann sum
//////      //XS1d[iQ2] += XS2d[index]*(w_max-w_min)/N_W;
//////
//////      //Simpson's rule
//////      Double_t coeff;
//////      if      (iq_bold==0 || iq_bold==N_q_bold-1) coeff=1.0;
//////      else if (iq_bold%2==0)                      coeff=2.0;
//////      else                                        coeff=4.0;
//////      //cout << iq_bold << "," << coeff << endl;
//////      XS_E[iE]+= coeff*unit_conv*foo.f_d2sigma_dw_dq*(q_bold_max-q_bold_min)/(3.0*(N_q_bold-1));
//////
//////      //if (foo.f_d2sigma_dw_dq>0.0) printf("enuqe=%5.3g; E=%5.3g; q_bold=%5.3g; XS=%5.3g \n",foo.SM_compute_enuqe(),E[iE],q_bold,XS_E[iE]);
//////
//////    }
//////
//////    printf("E=%5.3g;XS=%5.3g\n",E[iE],XS_E[iE]);
//////  }
//////
//////  TGraph xs1d_graph(N_E,E,XS_E);
//////  xs1d_graph.SetName("xs1d");
//////  xs1d_graph.SetTitle(";E_{#mu} (GeV);d#sigma/dE_{#mu} (cm^{2}/GeV)");
//////  
//////  TFile f("~elaird/ccqe/xs_ted/xs.root","RECREATE");
//////  xs1d_graph.Write();
//////  f.Close();
//////
//////  return 0;
//////}
//////
//////////////////////////////////////////////////////////////////////////////
//////Int_t xs_Q2(TT_param_class *params) {
//////
//////  Double_t Enu=0.8;
//////
//////  const Double_t unit_conv=0.389379e-27;  
//////
//////  const Int_t N_Q2=200;
//////  
//////  Double_t Q2      [N_Q2];
//////  Double_t XS1d    [N_Q2];
//////  Double_t XS1d_err[N_Q2];
//////  
//////  Double_t Q2_min=pow(params->f_M,2)-pow(params->f_M_p,2);
//////  Double_t Q2_max=Q2_min+2.0*params->f_M*(Enu-params->f_m_lep);
//////
//////  for (Int_t iQ2=0;iQ2<N_Q2;iQ2++) {
//////  
//////    Q2      [iQ2]=iQ2*(Q2_max-Q2_min)/N_Q2+Q2_min;
//////    XS1d    [iQ2]=0.0;
//////    XS1d_err[iQ2]=0.0;
//////
//////    Double_t w=(Q2[iQ2]-pow(params->f_M,2)+pow(params->f_M_p,2))/(2.0*params->f_M);
//////    Double_t q_bold_mag=sqrt(Q2[iQ2]+w*w);
//////
//////    Double_t p_lep_sq=pow(Enu-w,2)-pow(params->f_m_lep,2);
//////    Double_t cos_theta_q=(pow(Enu,2)-p_lep_sq+pow(q_bold_mag,2))/(2.0*Enu*q_bold_mag);
//////    if (cos_theta_q>1.0) continue;
//////    printf("w=%5.3g; Q2=%5.3g; q_bold_mag=%5.3g; c_q=%5.3g\n",w,Q2[iQ2],q_bold_mag,cos_theta_q);
//////    Double_t sin_theta_q=sqrt(1.0-pow(cos_theta_q,2));
//////    //Double_t sin_theta_lep=-Q2[iQ2]*sin_theta_q/sqrt(p_lep_sq);
//////
//////    Double_t q_lower[4];
//////    q_lower[0]=w;
//////    q_lower[1]=0.0;
//////    q_lower[2]=q_bold_mag*sin_theta_q;
//////    q_lower[3]=q_bold_mag*cos_theta_q;
//////
//////    params->f_single_nucleon_mode=1;
//////    params->f_use_M_instead_of_MplusW=0;
//////    TV_class foo(params,Enu,q_lower);
//////    foo.Compute_XS();
//////
//////    Double_t jacobian=0.5/q_bold_mag;
//////    XS1d    [iQ2]=unit_conv*jacobian*foo.f_d2sigma_dw_dq;
//////    XS1d_err[iQ2]=unit_conv*jacobian*foo.f_d2sigma_dw_dq_err;
//////    
//////    //      //if (XS2d[iQ2]>0.0) printf("enuqe=%5.3g; E=%5.3g; C=%5.3g; XS=%5.3g; err=%5.3g; \n",foo.SM_compute_enuqe(),w[iQ2],Q2[iQ2],XS2d[iQ2],XS2d_err[iQ2]);
//////    //      //printf("enuqe=%5.3g; E=%5.3g; C=%5.3g; XS=%5.3g; err=%5.3g; \n",foo.SM_compute_enuqe(),w[iQ2],Q2[iQ2],XS2d[iQ2],XS2d_err[iQ2]);
//////  }
//////
//////  TGraph xs1d_graph(N_Q2,Q2,XS1d);
//////  xs1d_graph.SetName("xs1d");
//////  xs1d_graph.SetTitle(";Q^{2} (GeV^{2});d#sigma/dQ^{2} (cm^{2}/GeV^{2})");
//////  
//////  TFile f("~elaird/ccqe/xs_ted/xs.root","RECREATE");
//////  xs1d_graph.Write();
//////  f.Close();
//////
//////  return 0;
//////}
