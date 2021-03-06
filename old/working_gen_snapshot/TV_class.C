#define TV_class_cxx
#include "TV_class.h"
#include "TMath.h"
#include "TRandom3.h"
#include "TCanvas.h"

////////////////////////////////////////////////////////////////
void TV_class::Evaluate_d4sigma_dw_dq_bold_domega_p()
{
  if (f_bad_event) {
    f_d4sigma_dw_dq_bold_domega_p=0.0;
    return;
  }
  else if (f_params->f_do_phony_gen) {
    Phony_differential();
    return;
  }
  else if (f_params->f_single_nucleon_mode) Single_nucleon_xs();
  else if (f_params->f_do_Smith_Moniz_gen) SM_differential();
  else if (f_params->f_do_Smith_Moniz) Compute_XS();
  else if (f_params->f_do_AS_SF)       AS_SF_integrate();

  Double_t q=0.0;
  for (Int_t iQ=1;iQ<4;iQ++) q+=pow(f_q_lower[iQ],2);
  q = sqrt(q);
  Double_t factor=pow(f_params->f_GF*f_params->f_coscab,2)/(4.0*TMath::Pi());
  factor=factor*q/pow(f_k_lower[0],2);
  f_d4sigma_dw_dq_bold_domega_p*=factor;
}

////////////////////////////////////////////////////////////////
void TV_class::SM_differential()
{
  f_d4sigma_dw_dq_bold_domega_p=0.0;

  Double_t p_lower[4];
  Double_t sinth=sqrt(1.0-f_cos_theta_p*f_cos_theta_p);

  p_lower[1]=sinth*cos(f_phi_p);
  p_lower[2]=sinth*sin(f_phi_p);
  p_lower[3]=f_cos_theta_p;

  Double_t p_mag=SM_determine_p(p_lower);
  if (p_mag<0.0) return;

  Double_t E_pprime=0.0;
  Bool_t blocked=0;

  p_lower[0]=pow(f_params->f_M,2);
  for (Int_t iComp=1;iComp<4;iComp++) {
    p_lower[iComp]*=p_mag;
    p_lower[0]+=pow(p_lower[iComp],2);
    E_pprime+=pow(p_lower[iComp]+f_q_lower[iComp],2);
  }
  blocked=E_pprime<pow(f_nucleus->f_SM_p_fermi,2);
  E_pprime+=pow(f_params->f_M_p,2);
  E_pprime=sqrt(E_pprime);
  p_lower[0]=sqrt(p_lower[0]);

  ////debug info here
  //cout << "M_i=" << sqrt(pow(p_lower[0],2)-pow(p_lower[1],2)-pow(p_lower[2],2)-pow(p_lower[3],2)) << endl;
  //cout << "M_f=" << sqrt(pow(p_lower[0]+f_q_lower[0],2)-pow(p_lower[1]+f_q_lower[1],2)-pow(p_lower[2]+f_q_lower[2],2)-pow(p_lower[3]+f_q_lower[3],2)) << endl;
  //cout << "M_lep=" << sqrt(pow(f_kprime_lower[0],2)-pow(f_kprime_lower[1],2)-pow(f_kprime_lower[2],2)-pow(f_kprime_lower[3],2)) << endl;
  //cout << "EnuQE=" << SM_compute_enuqe() << endl;

  if (f_params->f_do_Pauli_blocking && blocked) return;
  else {
    Init_L();
    f_d4sigma_dw_dq_bold_domega_p=SM_evaluate_integrand(p_lower,E_pprime);
  }
}

////////////////////////////////////////////////////////////////
void TV_class::Phony_differential()
{
  Double_t w_min=0.0;
  Double_t w_max=f_k_lower[0]-f_params->f_m_lep;
  Double_t q_bold_min=0.0;
  Double_t q_bold_max=2.0;
  Double_t cos_min=-1.0;
  Double_t cos_max=1.0;
  Double_t phi_min=-TMath::Pi();
  Double_t phi_max=TMath::Pi();

  Double_t w=f_q_lower[0];
  Double_t q=0.0;
  for (Int_t i=1;i<4;i++) q+=pow(f_q_lower[i],2);
  q=sqrt(q);
  Double_t a=(w-w_min)/(w_max-w_min);
  Double_t b=(q-q_bold_min)/(q_bold_max-q_bold_min);
  Double_t c=(f_cos_theta_p-cos_min)/(cos_max-cos_min);
  Double_t d=(f_phi_p-phi_min)/(phi_max-phi_min);

  if (a<0.5 || b<0.5) f_d4sigma_dw_dq_bold_domega_p=0.0;
  else f_d4sigma_dw_dq_bold_domega_p=a*b*c*d;
}

////////////////////////////////////////////////////////////////
void TV_class::Compute_XS()
{
  if (f_params->f_single_nucleon_mode) Single_nucleon_xs();
  else if (f_params->f_do_Smith_Moniz) SM_integrate();
  else if (f_params->f_do_AS_SF)       AS_SF_integrate();

  Double_t q=0.0;
  for (Int_t iQ=1;iQ<4;iQ++) q+=pow(f_q_lower[iQ],2);
  q = sqrt(q);
  Double_t factor=pow(f_params->f_GF*f_params->f_coscab,2)/(4.0*TMath::Pi());
  factor=factor*q/pow(f_k_lower[0],2);
  f_d2sigma_dw_dq     *=factor;
  f_d2sigma_dw_dq_err *=factor;
}

////////////////////////////////////////////////////////////////
void TV_class::Single_nucleon_xs()
{
  Double_t p_lower[4];
  p_lower[0]=f_params->f_M;
  Double_t E_pprime=pow(f_params->f_M_p,2);
  for (Int_t iComp=1;iComp<4;iComp++) {
    p_lower[iComp]=0.0;
    E_pprime+=pow(f_q_lower[iComp],2);
  }
  E_pprime=sqrt(E_pprime);
    
  Double_t Re_contraction,Im_contraction;
  Get_contraction(p_lower,Re_contraction,Im_contraction);
  //cout << "Re_contraction = " << Re_contraction << endl;
  //cout << "Im_contraction = " << Im_contraction << endl;

  Double_t ans;
  if (f_params->f_use_M_instead_of_MplusW) ans=Re_contraction/(p_lower[0]*p_lower[0]);
  else ans=Re_contraction/(p_lower[0]*E_pprime);

  f_d2sigma_dw_dq_err = 0.0;
  Double_t energy=f_q_lower[0]+f_params->f_M-E_pprime;
  if (energy>=-1.0e-10) f_d2sigma_dw_dq=ans;
  else                  f_d2sigma_dw_dq=0.0;

  //cout << "energy,ans,errs = " << energy << "," << f_d2sigma_dw_dq << "," << f_d2sigma_dw_dq_err << endl;
}

////////////////////////////////////////////////////////////////
void TV_class::AS_SF_integrate()
{
  Int_t N=600;

  Double_t results=0.0;
  Double_t results2=0.0;

  //mean field part
  for (Int_t iN=0;iN<N;iN++) {
    Double_t p_lower[4];
    Double_t costh=f_params->f_rand.Uniform(-1.0,1.0);
    Double_t sinth=sqrt(1.0-costh*costh);
    Double_t phi=f_params->f_rand.Uniform(-TMath::Pi(),TMath::Pi());

    p_lower[1]=sinth*cos(phi);
    p_lower[2]=sinth*sin(phi);
    p_lower[3]=costh;

    Double_t p_mag=AS_SF_determine_p(p_lower);
    //include a warning about two solutions
    if (p_mag<0.0) continue;

    Double_t E_pprime=0.0;
    Double_t result=0.0;
    Bool_t blocked=0;

    p_lower[0]=pow(f_params->f_M,2);
    for (Int_t iComp=1;iComp<4;iComp++) {
      p_lower[iComp]*=p_mag;
      p_lower[0]+=pow(p_lower[iComp],2);
      E_pprime+=pow(p_lower[iComp]+f_q_lower[iComp],2);
    }
    blocked=E_pprime<pow(f_nucleus->f_AS_SF_p_fermi,2);
    E_pprime+=pow(f_params->f_M_p,2);
    E_pprime=sqrt(E_pprime);
    p_lower[0]=sqrt(p_lower[0]);

    ////debug info here
    //cout << "M_i=" << sqrt(pow(p_lower[0],2)-pow(p_lower[1],2)-pow(p_lower[2],2)-pow(p_lower[3],2)) << endl;
    //cout << "M_f=" << sqrt(pow(p_lower[0]+f_q_lower[0],2)-pow(p_lower[1]+f_q_lower[1],2)-pow(p_lower[2]+f_q_lower[2],2)-pow(p_lower[3]+f_q_lower[3],2)) << endl;
    //cout << "M_lep=" << sqrt(pow(f_kprime_lower[0],2)-pow(f_kprime_lower[1],2)-pow(f_kprime_lower[2],2)-pow(f_kprime_lower[3],2)) << endl;
    //cout << "EnuQE=" << SM_compute_enuqe() << endl;

    //Double_t denom=TMath::Abs(SM_argument_of_delta_prime(p_lower));
    //if (iP_mag==0) cout << "denom0=" << denom << endl;
    //if (iP_mag==1) cout << "denom1=" << denom << endl;
    //cout << endl;
    if (f_params->f_do_Pauli_blocking && blocked) continue;
    else result+=SM_evaluate_integrand(p_lower,E_pprime);

    results+=result;
    results2+=result*result;
  }

  Double_t V=4.0*TMath::Pi();

  f_d2sigma_dw_dq     = V*results/N;
  f_d2sigma_dw_dq_err = V*sqrt(results2/N-pow(results/N,2))/sqrt(N-1);
  //cout << "ans, errs = " << f_d2sigma_dw_dq << "," << f_d2sigma_dw_dq_err << endl;
}

////////////////////////////////////////////////////////////////
void TV_class::SM_integrate()
{
  Int_t N=600;

  Double_t results=0.0;
  Double_t results2=0.0;

  for (Int_t iN=0;iN<N;iN++) {
    Double_t p_lower[4];
    Double_t costh=f_params->f_rand.Uniform(-1.0,1.0);
    Double_t sinth=sqrt(1.0-costh*costh);
    Double_t phi=f_params->f_rand.Uniform(-TMath::Pi(),TMath::Pi());

    p_lower[1]=sinth*cos(phi);
    p_lower[2]=sinth*sin(phi);
    p_lower[3]=costh;

    Double_t p_mag=SM_determine_p(p_lower);

    Double_t E_pprime=0.0;
    Double_t result=0.0;
    Bool_t blocked=0;

    if (p_mag<0.0) continue;

    p_lower[0]=pow(f_params->f_M,2);
    for (Int_t iComp=1;iComp<4;iComp++) {
      p_lower[iComp]*=p_mag;
      p_lower[0]+=pow(p_lower[iComp],2);
      E_pprime+=pow(p_lower[iComp]+f_q_lower[iComp],2);
    }
    blocked=E_pprime<pow(f_nucleus->f_SM_p_fermi,2);
    E_pprime+=pow(f_params->f_M_p,2);
    E_pprime=sqrt(E_pprime);
    p_lower[0]=sqrt(p_lower[0]);

    ////debug info here
    //cout << "M_i=" << sqrt(pow(p_lower[0],2)-pow(p_lower[1],2)-pow(p_lower[2],2)-pow(p_lower[3],2)) << endl;
    //cout << "M_f=" << sqrt(pow(p_lower[0]+f_q_lower[0],2)-pow(p_lower[1]+f_q_lower[1],2)-pow(p_lower[2]+f_q_lower[2],2)-pow(p_lower[3]+f_q_lower[3],2)) << endl;
    //cout << "M_lep=" << sqrt(pow(f_kprime_lower[0],2)-pow(f_kprime_lower[1],2)-pow(f_kprime_lower[2],2)-pow(f_kprime_lower[3],2)) << endl;
    //cout << "EnuQE=" << SM_compute_enuqe() << endl;

    //Double_t denom=TMath::Abs(SM_argument_of_delta_prime(p_lower));
    //if (iP_mag==0) cout << "denom0=" << denom << endl;
    //if (iP_mag==1) cout << "denom1=" << denom << endl;
    //cout << endl;
    if (f_params->f_do_Pauli_blocking && blocked) continue;
    else result+=SM_evaluate_integrand(p_lower,E_pprime);
    if (isnan(result)) printf("p_mag:%g; E_pprime:%g; result:%g\n",p_mag,E_pprime,result);
    results+=result;
    results2+=result*result;
  }

  Double_t V=4.0*TMath::Pi();

  f_d2sigma_dw_dq     = V*results/N;
  f_d2sigma_dw_dq_err = V*sqrt(results2/N-pow(results/N,2))/sqrt(N-1);
  //if (isnan(f_d2sigma_dw_dq)) cout << "ans, errs = " << f_d2sigma_dw_dq << "," << f_d2sigma_dw_dq_err << endl;
}

//////////////////////////////////////////////////////////////////
//old, a little broken if there are two sol's
//void TV_class::SM_integrate()
//{
//
//  Int_t N=600;
//
//  Double_t results=0.0;
//  Double_t results2=0.0;
//
//  for (Int_t iN=0;iN<N;iN++) {
//    Double_t p_lower[4];
//    Double_t costh=f_params->f_rand.Uniform(-1.0,1.0);
//    Double_t sinth=sqrt(1.0-costh*costh);
//    Double_t phi=f_params->f_rand.Uniform(-TMath::Pi(),TMath::Pi());
//
//    p_lower[1]=sinth*cos(phi);
//    p_lower[2]=sinth*sin(phi);
//    p_lower[3]=costh;
//
//    Double_t p_mags[2];
//    SM_determine_p(p_lower,p_mags);
//
//    Double_t E_pprime=0.0;
//    Double_t result=0.0;
//    Bool_t blocked=0;
//
//    Int_t count_num=0;
//    for (Int_t iComp=0;iComp<1;iComp++) {
//      if (p_mags[iComp]>=0.0) count_num++;
//    }
//    if (count_num>1) cout << "uh oh: count_num=" << count_num << endl;
//
//    //add contributions from any p_mags that were found
//    for (Int_t iP_mag=0;iP_mag<2;iP_mag++) {
//      if (p_mags[iP_mag]<0.0) continue;
//
//      p_lower[0]=pow(f_params->f_M,2);
//      for (Int_t iComp=1;iComp<4;iComp++) {
//	p_lower[iComp]*=p_mags[iP_mag];
//	p_lower[0]+=pow(p_lower[iComp],2);
//	E_pprime+=pow(p_lower[iComp]+f_q_lower[iComp],2);
//      }
//      blocked=E_pprime<pow(f_nucleus->f_SM_p_fermi,2);
//      E_pprime+=pow(f_params->f_M_p,2);
//      E_pprime=sqrt(E_pprime);
//      p_lower[0]=sqrt(p_lower[0]);
//
//      ////debug info here
//      //cout << "M_i=" << sqrt(pow(p_lower[0],2)-pow(p_lower[1],2)-pow(p_lower[2],2)-pow(p_lower[3],2)) << endl;
//      //cout << "M_f=" << sqrt(pow(p_lower[0]+f_q_lower[0],2)-pow(p_lower[1]+f_q_lower[1],2)-pow(p_lower[2]+f_q_lower[2],2)-pow(p_lower[3]+f_q_lower[3],2)) << endl;
//      //cout << "M_lep=" << sqrt(pow(f_kprime_lower[0],2)-pow(f_kprime_lower[1],2)-pow(f_kprime_lower[2],2)-pow(f_kprime_lower[3],2)) << endl;
//      //cout << "EnuQE=" << SM_compute_enuqe() << endl;
//
//      //Double_t denom=TMath::Abs(SM_argument_of_delta_prime(p_lower));
//      //if (iP_mag==0) cout << "denom0=" << denom << endl;
//      //if (iP_mag==1) cout << "denom1=" << denom << endl;
//      //cout << endl;
//      if (f_params->f_do_Pauli_blocking && blocked) continue;
//      else result+=SM_evaluate_integrand(p_lower,E_pprime);
//    }
//    results+=result;
//    results2+=result*result;
//  }
//
//  Double_t V=4.0*TMath::Pi();
//
//  f_d2sigma_dw_dq     = V*results/N;
//  f_d2sigma_dw_dq_err = V*sqrt(results2/N-pow(results/N,2))/sqrt(N-1);
//  //cout << "ans, errs = " << f_d2sigma_dw_dq << "," << f_d2sigma_dw_dq_err << endl;
//}

////////////////////////////////////////////////////////////////
Double_t TV_class::SM_evaluate_integrand(Double_t p_lower[4],Double_t E_pprime)
{
  Double_t Re_contraction,Im_contraction;
  Get_contraction(p_lower,Re_contraction,Im_contraction);
  //cout << "Re_contraction = " << Re_contraction << endl;
  //cout << "Im_contraction = " << Im_contraction << endl;

  Double_t ans=0.0;
  Double_t SF_norm=3.0*f_nucleus->f_N/4.0/TMath::Pi()/pow(f_nucleus->f_SM_p_fermi,3);
  if (f_params->f_use_M_instead_of_MplusW) ans=Re_contraction*SF_norm/(p_lower[0]*p_lower[0]);
  else                                     ans=Re_contraction*SF_norm/(p_lower[0]*E_pprime);
  
  //--we converted to spherical coordinates for the initial momentum
  //--the delta function in the SM spectral function killed the p_mag integral
  //so we need two more factors
  Double_t p_mag_squared=0.0;
  for (Int_t i=1;i<4;i++) p_mag_squared+=pow(p_lower[i],2);

  Double_t denom=TMath::Abs(SM_argument_of_delta_prime(p_lower));
  if (denom==0.0) cout <<"vanishing denominator a" << endl;
  return ans*p_mag_squared/denom;
}

////////////////////////////////////////////////////////////////
void TV_class::Get_contraction(Double_t p_lower[4],Double_t& Re_ans,Double_t& Im_ans)
{
  Double_t p_upper[4];
  f_params->Upper(f_params->f_g,p_lower,p_upper);

  Double_t q_tilde_lower[4],q_tilde_upper[4];
  for (Int_t mu=0;mu<4;mu++) q_tilde_lower[mu]=f_q_lower[mu];

  if (f_params->f_do_Smith_Moniz && f_params->f_do_deForest_prescription) {
    q_tilde_lower[0]-=f_nucleus->f_SM_e_bind;
  }
  f_params->Upper(f_params->f_g,q_tilde_lower,q_tilde_upper);

  Double_t q2_tilde=0.0;
  for (Int_t mu=0;mu<4;mu++) q2_tilde+=q_tilde_lower[mu]*q_tilde_upper[mu];

  Double_t Re_H_upper_mu_nu;
  Double_t Im_H_upper_mu_nu;

  Double_t h1_factor=f_params->H1(q2_tilde)*pow(f_params->f_M,2);
  Double_t h2_factor=f_params->H2(q2_tilde);
  Double_t h3_factor=f_params->H3(q2_tilde)/2.0;
  Double_t h4_factor=f_params->H4(q2_tilde);
  Double_t h5_factor=f_params->H5(q2_tilde)/2.0;

  Re_ans=0.0;
  Im_ans=0.0;

  //Double_t asdf=0.0;
  //for (Int_t i=0;i<4;i++) asdf+=f_q_lower[i]*f_params->f_g[i][i]*f_q_lower[i];
  //printf("q2,q2_tilde=%5.3g,%5.3g\n",asdf,q2_tilde);

  for (Int_t mu=0;mu<4;mu++) {
    for (Int_t nu=0;nu<4;nu++) {
      Re_H_upper_mu_nu = -f_params->f_g[mu][nu]*h1_factor;
      Re_H_upper_mu_nu+= p_upper[mu]*p_upper[nu]*h2_factor;
      Re_H_upper_mu_nu+= -q_tilde_upper[mu]*q_tilde_upper[nu]*h4_factor;
      Re_H_upper_mu_nu+= (p_upper[mu]*q_tilde_upper[nu]+q_tilde_upper[mu]*p_upper[nu])*h5_factor;
      Im_H_upper_mu_nu = 0.0;

      if (mu!=nu) {
	for (Int_t kappa=0;kappa<4;kappa++) {
	  if (kappa==mu || kappa==nu) continue;
	  //only one value of lambda contributes
	  Int_t lambda=6-mu-nu-kappa;
	  //epsilon_upper=-epsilon_lower (according to Mathworld)
	  Im_H_upper_mu_nu += -f_params->f_epsilon_lower[mu][nu][kappa][lambda]*p_lower[kappa]*q_tilde_lower[lambda]*h3_factor;
	}
	//cout << "Im_H_upper[" << mu << "][" << nu << "]: " << Im_H_upper_mu_nu << endl;
      }

      Re_ans+=f_Re_L_lower[mu][nu]*Re_H_upper_mu_nu;
      Re_ans-=f_Im_L_lower[mu][nu]*Im_H_upper_mu_nu;
      Im_ans+=f_Re_L_lower[mu][nu]*Im_H_upper_mu_nu;
      Im_ans+=f_Im_L_lower[mu][nu]*Re_H_upper_mu_nu;
    }
  }

}

////////////////////////////////////////////////////////////////
Double_t TV_class::SM_determine_p(Double_t p_lower[4])
{
  Double_t q_mag;
  Double_t p_mag_old;
  Double_t cos_theta_pq;
  Determine_stuff(p_lower,q_mag,p_mag_old,cos_theta_pq);
  f_histo->Fill(cos_theta_pq);

  //solving a quadratic
  Double_t D,a,b,c;
  SM_determine_coeffs(q_mag,cos_theta_pq,D,a,b,c);
  Double_t discriminant=b*b-4.0*a*c;

  if (discriminant<0.0) return -2.0;
  if (a==0.0) {
    cout << "zomg: a=0" << endl;
    return -2.0;
  }

  Double_t sq_disc=sqrt(discriminant);
  Double_t root1=(-b+sq_disc)*0.5/a;
  Double_t root2=(-b-sq_disc)*0.5/a;
  Double_t factor1=root1/p_mag_old;
  Double_t factor2=root2/p_mag_old;

  Double_t p_threshold=1.0e-5;
  Bool_t accept1=root1>p_threshold && root1<f_nucleus->f_SM_p_fermi;
  Bool_t accept2=root2>p_threshold && root2<f_nucleus->f_SM_p_fermi;

  Double_t ans1,ans2,E1,E2;
  Bool_t return_1=0;
  Bool_t return_2=0;

  if (accept1) {
    Double_t pprime_sq1=Pprime_squared(p_lower,factor1);
    E1=f_q_lower[0]+f_params->f_M-sqrt(pow(f_params->f_M_p,2)+pprime_sq1);
    Double_t E1_min=-1.0e-3;
    //Double_t E1_min=f_params->f_M+f_params->f_M_residual_nucleus-f_params->f_M_target_nucleus;
    //cout << "E1,E1_min=" << E1 << "," << E1_min << endl;
    if (E1>=E1_min) {
      //cout << "p_mag_old=" << p_mag_old << ", root1=" << root1 << ", pprime=" << sqrt(pprime_sq1) <<", q_bold=" << sqrt(pow(f_q_lower[1],2)+pow(f_q_lower[2],2)+pow(f_q_lower[3],2))<< endl;
      ans1=SM_argument_of_delta(p_lower,factor1);
      if (TMath::Abs(ans1)<1.0e-10) return_1=1;
    }
  }
  if (accept2) {
    Double_t pprime_sq2=Pprime_squared(p_lower,factor2);
    E2=f_q_lower[0]+f_params->f_M-sqrt(pow(f_params->f_M_p,2)+pprime_sq2);
    Double_t E2_min=-1.0e-3;
    //Double_t E2_min=f_params->f_M+f_params->f_M_residual_nucleus-f_params->f_M_target_nucleus;
    //cout << "E2,E2_min=" << E2 << "," << E2_min << endl;
    if (E2>=E2_min) {
      ans2=SM_argument_of_delta(p_lower,factor2);
      if (TMath::Abs(ans2)<1.0e-10) return_2=1;
    }
  }

  if (return_1 && return_2) {
    printf("2 solutions: root1=%g; ans1=%g\n",root1,ans1);
    printf("2 solutions: root2=%g; ans2=%g\n\n",root2,ans2);
  }
  else if (return_1) return root1;
  else if (return_2) return root2;
  else               return -2.0;

  //cout << "ax2+bx+c (1): " << a*root1*root1+b*root1+c << endl;
  //cout << "ax2+bx+c (2): " << a*root2*root2+b*root2+c << endl;
  //cout << "root1,root2: " << root1 << "," << root2 << endl;
  //cout << "ans1,ans2: " << ans1 << "," << ans2 << endl;

}

//////////////////////////////////////////////////////////////////
//void TV_class::SM_determine_p(Double_t p_lower[4],Double_t answers[2])
//{
//  answers[0]=-2.0;
//  answers[1]=-2.0;
//  
//  Double_t q_mag;
//  Double_t p_mag_old;
//  Double_t cos_theta_pq;
//  Determine_stuff(p_lower,q_mag,p_mag_old,cos_theta_pq);
//  f_histo->Fill(cos_theta_pq);
//
//  //solving a quadratic
//  Double_t D,a,b,c;
//  SM_determine_coeffs(q_mag,cos_theta_pq,D,a,b,c);
//  Double_t discriminant=b*b-4.0*a*c;
//
//  if (discriminant<0.0) return;
//  if (a==0.0) {
//    cout << "zomg: a=0" << endl;
//    return;
//  }
//
//  Double_t sq_disc=sqrt(discriminant);
//  Double_t root1=(-b+sq_disc)*0.5/a;
//  Double_t root2=(-b-sq_disc)*0.5/a;
//  Double_t factor1=root1/p_mag_old;
//  Double_t factor2=root2/p_mag_old;
//
//  Bool_t accept1=root1>0.0 && root1<f_nucleus->f_SM_p_fermi;
//  Bool_t accept2=root2>0.0 && root2<f_nucleus->f_SM_p_fermi;
//
//  Double_t ans1,ans2,E1,E2;
//  if (accept1) {
//    Double_t pprime_sq1=Pprime_squared(p_lower,factor1);
//    E1=f_q_lower[0]+f_params->f_M-sqrt(pow(f_params->f_M_p,2)+pprime_sq1);
//    Double_t E1_min=0.0;
//    //Double_t E1_min=f_params->f_M+f_params->f_M_residual_nucleus-f_params->f_M_target_nucleus;
//    //cout << "E1,E1_min=" << E1 << "," << E1_min << endl;
//    if (E1>=E1_min) {
//      //cout << "p_mag_old=" << p_mag_old << ", root1=" << root1 << ", pprime=" << sqrt(pprime_sq1) <<", q_bold=" << sqrt(pow(f_q_lower[1],2)+pow(f_q_lower[2],2)+pow(f_q_lower[3],2))<< endl;
//      ans1=SM_argument_of_delta(p_lower,factor1);
//      if (TMath::Abs(ans1)<1.0e-10) answers[0]=root1;
//    }
//  }
//  if (accept2) {
//    Double_t pprime_sq2=Pprime_squared(p_lower,factor2);
//    E2=f_q_lower[0]+f_params->f_M-sqrt(pow(f_params->f_M_p,2)+pprime_sq2);
//    Double_t E2_min=0.0;
//    //Double_t E2_min=f_params->f_M+f_params->f_M_residual_nucleus-f_params->f_M_target_nucleus;
//    //cout << "E2,E2_min=" << E2 << "," << E2_min << endl;
//    if (E2>=E2_min) {
//      ans2=SM_argument_of_delta(p_lower,factor2);
//      if (TMath::Abs(ans2)<1.0e-10) answers[1]=root2;
//    }
//  }
//
//  //cout << "ax2+bx+c (1): " << a*root1*root1+b*root1+c << endl;
//  //cout << "ax2+bx+c (2): " << a*root2*root2+b*root2+c << endl;
//  //cout << "root1,root2: " << root1 << "," << root2 << endl;
//  //cout << "ans1,ans2: " << ans1 << "," << ans2 << endl;
//
//  return;
//}

////////////////////////////////////////////////////////////////
Double_t TV_class::AS_SF_determine_p(Double_t p_lower[4])
{
  Double_t q_mag;
  Double_t p_mag_old;
  Double_t cos_theta_pq;
  Determine_stuff(p_lower,q_mag,p_mag_old,cos_theta_pq);
  f_histo->Fill(cos_theta_pq);

  //solving a quadratic
  Double_t D,a,b,c;
  SM_determine_coeffs(q_mag,cos_theta_pq,D,a,b,c);
  Double_t discriminant=b*b-4.0*a*c;

  if (discriminant<0.0) return -2.0;
  if (a==0.0) {
    cout << "zomg: a=0" << endl;
    return -2.0;
  }

  Double_t sq_disc=sqrt(discriminant);
  Double_t root1=(-b+sq_disc)*0.5/a;
  Double_t root2=(-b-sq_disc)*0.5/a;
  Double_t factor1=root1/p_mag_old;
  Double_t factor2=root2/p_mag_old;

  Bool_t accept1=root1>0.0 && root1<f_nucleus->f_SM_p_fermi;
  Bool_t accept2=root2>0.0 && root2<f_nucleus->f_SM_p_fermi;

  Double_t ans1,ans2,E1,E2;
  //if (accept1) {
  //  Double_t pprime_sq1=Pprime_squared(p_lower,factor1);
  //  E1=f_q_lower[0]+f_params->f_M-sqrt(pow(f_params->f_M_p,2)+pprime_sq1);
  //
  //  if (E1>-1.0e-3) {
  //    ans1=SM_argument_of_delta(p_lower,factor1);
  //    if (TMath::Abs(ans1)<1.0e-10) answers[0]=root1;
  //  }
  //}
  //if (accept2) {
  //  Double_t pprime_sq2=Pprime_squared(p_lower,factor2);
  //  E2=f_q_lower[0]+f_params->f_M-sqrt(pow(f_params->f_M_p,2)+pprime_sq2);
  //
  //  if (E2>-1.0e-3) {
  //    ans2=SM_argument_of_delta(p_lower,factor2);
  //    if (TMath::Abs(ans2)<1.0e-10) answers[1]=root2;
  //  }
  //}

  //cout << "ax2+bx+c (1): " << a*root1*root1+b*root1+c << endl;
  //cout << "ax2+bx+c (2): " << a*root2*root2+b*root2+c << endl;
  //cout << "root1,root2: " << root1 << "," << root2 << endl;
  //cout << "ans1,ans2: " << ans1 << "," << ans2 << endl;

  return -2.0;
}

////////////////////////////////////////////////////////////////
Double_t TV_class::SM_argument_of_delta(Double_t p_lower[4],Double_t factor)
{
  Double_t var1=pow(f_params->f_M,2);
  Double_t var2=pow(f_params->f_M_p,2);
  for (Int_t i=1;i<4;i++) {
    var1+=pow(p_lower[i]*factor,2);
    var2+=pow(p_lower[i]*factor+f_q_lower[i],2);
  }
  return sqrt(var1)-f_nucleus->f_SM_e_bind+f_q_lower[0]-sqrt(var2);
}

////////////////////////////////////////////////////////////////
Double_t TV_class::SM_argument_of_delta_prime(Double_t p_lower[4])
{
  Double_t q_mag,p_mag,cos_theta_pq;

  Determine_stuff(p_lower,q_mag,p_mag,cos_theta_pq);
  Double_t pq2=p_mag*p_mag+q_mag*q_mag+2.0*p_mag*q_mag*cos_theta_pq;
  Double_t ans=p_mag/sqrt(pow(f_params->f_M,2)+pow(p_mag,2));
  ans-=(p_mag+q_mag*cos_theta_pq)/sqrt(pow(f_params->f_M_p,2)+pq2);
  return ans;
}

////////////////////////////////////////////////////////////////
void TV_class::SM_determine_coeffs(Double_t q_mag,Double_t cos_theta_pq,Double_t& D,Double_t& a,Double_t& b,Double_t& c)
{
  D=pow(f_params->f_M_p,2)-pow(f_params->f_M,2)-pow(f_q_lower[0]-f_nucleus->f_SM_e_bind,2)+pow(q_mag,2);
  a=4.0*(pow(q_mag*cos_theta_pq,2)-pow(f_q_lower[0]-f_nucleus->f_SM_e_bind,2));
  b=4.0*q_mag*cos_theta_pq*D;
  c=D*D-4.0*pow((f_q_lower[0]-f_nucleus->f_SM_e_bind)*f_params->f_M,2);
}

////////////////////////////////////////////////////////////////
void TV_class::Determine_stuff(Double_t p_lower[4],Double_t& q_mag,Double_t& p_mag_old,Double_t& cos_theta_pq)
{
  q_mag=0.0;
  p_mag_old=0.0;
  cos_theta_pq=0.0;

  for (Int_t i=1;i<4;i++) {
    q_mag+=f_q_lower[i]*f_q_lower[i];
    p_mag_old+=p_lower[i]*p_lower[i];
    cos_theta_pq+=f_q_lower[i]*p_lower[i];
  }
  q_mag=sqrt(q_mag);
  p_mag_old=sqrt(p_mag_old);
  cos_theta_pq/=q_mag*p_mag_old;
  //cout << "cos_theta_pq: " << cos_theta_pq << endl;
}

////////////////////////////////////////////////////////////////
Double_t TV_class::Pprime_squared(Double_t p_lower[4],Double_t factor)
{
  Double_t ans=0.0;
  for (Int_t i=1;i<4;i++) {
    ans+=pow(p_lower[i]*factor+f_q_lower[i],2);
  }
  return ans;
}

////////////////////////////////////////////////////////////////
Double_t TV_class::SM_compute_enuqe()
{
  Double_t M=f_params->f_M+f_nucleus->f_SM_e_bind;
  Double_t num,denom;

  num=pow(f_params->f_M_p,2)-pow(f_params->f_m_lep,2)-M*M+2.0*M*f_kprime_lower[0];
  denom=M-f_kprime_lower[0]+f_kprime_lower[3];

  return 0.5*num/denom;
}
