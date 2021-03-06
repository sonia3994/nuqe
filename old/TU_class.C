#define TU_class_cxx
#include "TU_class.h"
#include "TMath.h"
#include "TRandom3.h"
#include "TCanvas.h"

////////////////////////////////////////////////////////////////
void TU_class::Compute_XS()
{
  Integrate();

  Double_t factor=pow(f_params->f_GF*f_params->f_coscab,2)/8.0;
  factor=factor/pow(TMath::Pi(),2)/f_k_lower[0]/f_kprime_lower[0];
  f_d3sigma_d3kprime     *=factor;
  f_d3sigma_d3kprime_err *=factor;

  Double_t p_mag=sqrt(pow(f_kprime_lower[1],2)+pow(f_kprime_lower[2],2)+pow(f_kprime_lower[3],2));
  Double_t another_factor=2.0*TMath::Pi()*p_mag*f_kprime_lower[0];
  f_d2sigma_de_dcostheta     = f_d3sigma_d3kprime    *another_factor;
  f_d2sigma_de_dcostheta_err = f_d3sigma_d3kprime_err*another_factor;
}

////////////////////////////////////////////////////////////////
void TU_class::Integrate()
{
  Int_t N=100;

  Double_t results=0.0;
  Double_t results2=0.0;

  for (Int_t iN=0;iN<N;iN++) {
    Double_t p_lower[4];
    Double_t E=0.0;
    Double_t result=0.0;
    Bool_t blocked=0;
    Bool_t kinematically_allowed=0;
    Bool_t inside_volume=0;

    if (f_params->f_do_Smith_Moniz) {
      //not correct, but fine for current tests
      Double_t p_mag=f_params->f_rand.Uniform(0.0,f_params->f_SM_p_fermi);
      Double_t costh=f_params->f_rand.Uniform(-1.0,1.0);
      Double_t sinth=sqrt(1.0-costh*costh);
      Double_t phi=f_params->f_rand.Uniform(0.0,2*TMath::Pi());
      p_lower[1]=p_mag*sinth*cos(phi);
      p_lower[2]=p_mag*sinth*sin(phi);
      p_lower[3]=p_mag*costh;
      inside_volume=1;
      //correct but slower
      //Double_t p2=0.0;
      //for (Int_t iComp=1;iComp<4;iComp++) {
      //  p_lower[iComp]=f_params->f_rand.Uniform(-1.0*f_params->f_SM_p_fermi,f_params->f_SM_p_fermi);
      //  p2 += pow(p_lower[iComp],2);
      //}
      //if (p2<pow(f_params->f_SM_p_fermi,2)) inside_volume=1;

      p_lower[0]=sqrt(pow(p_lower[1],2)+pow(p_lower[2],2)+pow(p_lower[3],2)+pow(f_params->f_M,2));
      //E not needed

      kinematically_allowed=Determine_kprime(p_lower);

      Double_t p2_fin=0.0;
      for (Int_t i=1;i<4;i++) p2_fin+=pow(p_lower[i]+f_q_lower[i],2);
	
      //for (Int_t i=0;i<4;i++) {
      //  cout << "p_final_lower[" << i << "]: " << p_lower[i]+f_q_lower[i] << endl;
      //}
	
      blocked=p2_fin<pow(f_params->f_SM_p_fermi,2);
      //cout << "p2_fin: " << p2_fin << endl;
      //cout << "blocked: " << blocked << endl << endl;
	
    }
    
    if (kinematically_allowed && inside_volume) result=Evaluate_integrand(E,p_lower);
    if (!inside_volume) {
      result=0.0;
      cout << "outside volume!" << endl;
    }
    if (!kinematically_allowed) {
      result=0.0;
      f_d3sigma_d3kprime_forbidden++;
      //cout << "kinematically not allowed" << endl;
    }
    if (f_params->f_do_Pauli_blocking && blocked) result=0.0;
    
    results+=result;
    results2+=result*result;
  }

  Double_t V=0.0;
  //for current tests
  if (f_params->f_do_Smith_Moniz) V=4.0/3.0*TMath::Pi()*pow(f_params->f_SM_p_fermi,3);
  //correct but slower
  //if (f_params->f_do_Smith_Moniz) V=8.0*pow(f_params->f_SM_p_fermi,3);

  f_d3sigma_d3kprime     = V*results/N;
  f_d3sigma_d3kprime_err = V*sqrt(results2/N-pow(results/N,2))/sqrt(N);
  //cout << "ans, errs = " << f_d3sigma_d3kprime << "," << f_d3sigma_d3kprime_err << endl;
}

////////////////////////////////////////////////////////////////
Double_t TU_class::Evaluate_integrand(Double_t E,Double_t p_lower[4])
{

  Double_t p_upper[4];
  f_params->Upper(f_params->f_g,p_lower,p_upper);
  Double_t q2=0.0;
  for (Int_t mu=0;mu<4;mu++) q2+=f_q_lower[mu]*f_q_upper[mu];

  Double_t Re_H_upper[4][4];
  Double_t Im_H_upper[4][4];
  Double_t M=f_params->f_M;

  for (Int_t mu=0;mu<4;mu++) {
    for (Int_t nu=0;nu<4;nu++) {
      Re_H_upper[mu][nu] = -f_params->f_g[mu][nu]*f_params->H1(q2);
      Re_H_upper[mu][nu]+= p_upper[mu]*p_upper[nu]*f_params->H2(q2)/M/M;
      Re_H_upper[mu][nu]+= -f_q_upper[mu]*f_q_upper[nu]*f_params->H4(q2)/M/M;
      Re_H_upper[mu][nu]+= (p_upper[mu]*f_q_upper[nu]+f_q_upper[mu]*p_upper[nu])*f_params->H5(q2)/2.0/M/M;
      Im_H_upper[mu][nu] = 0.0;
      for (Int_t kappa=0;kappa<4;kappa++) {
        for (Int_t lambda=0;lambda<4;lambda++) {
	  //epsilon_upper=-epsilon_lower (according to Mathworld)
          Im_H_upper[mu][nu] += -f_params->f_epsilon_lower[mu][nu][kappa][lambda]*p_lower[kappa]*f_q_lower[lambda]*f_params->H3(q2)/(2.0*M*M);
        }
      }
      Re_H_upper[mu][nu]*=M*M;
      Im_H_upper[mu][nu]*=M*M;
      //cout << "Im_H_upper[" << mu << "][" << nu << "]: " << Im_H_upper[mu][nu] << endl;
    }
  }

  Double_t Re_contraction=0.0;
  Double_t Im_contraction=0.0;
  for (Int_t mu=0;mu<4;mu++) {
    for (Int_t nu=0;nu<4;nu++) {
      Re_contraction+=f_Re_L_lower[mu][nu]*Re_H_upper[mu][nu];
      Re_contraction-=f_Im_L_lower[mu][nu]*Im_H_upper[mu][nu];
      Im_contraction+=f_Re_L_lower[mu][nu]*Im_H_upper[mu][nu];
      Im_contraction+=f_Im_L_lower[mu][nu]*Re_H_upper[mu][nu];
    }
  }
  //cout << "Re_contraction = " << Re_contraction << endl;
  //cout << "Im_contraction = " << Im_contraction << endl;

  Double_t ans=0.0;
  if (f_params->f_do_Smith_Moniz) {
    //from equation (19)
    Double_t E_pprime=f_q_lower[0]-f_params->f_SM_e_bind+sqrt(M*M+pow(p_lower[1],2)+pow(p_lower[2],2)+pow(p_lower[3],2));
    ans=Re_contraction*SF_SM(E,p_lower)/p_lower[0]/E_pprime;
  }
  //cout << "Re_contraction: " << Re_contraction << endl;
  //cout << "           ans: " << ans << endl;
  //cout << "            SF: " << SF_SM(E,p_lower) << endl;
  //cout << "      E_pprime: " << f_q_lower[0]-f_params->f_SM_e_bind+sqrt(M*M+pow(p_lower[1],2)+pow(p_lower[2],2)+pow(p_lower[3],2)) << endl;

  Double_t a_number=Re_contraction*pow(f_params->f_GF,2)*pow(f_params->f_coscab,2);
  a_number /=(8.0*pow(TMath::Pi(),2)*f_k_lower[0]*f_kprime_lower[0]);
  cout << "a number:" << a_number << endl;
  return ans;
}

////////////////////////////////////////////////////////////////
Double_t TU_class::SF_SM(Double_t E,Double_t p_lower[4]) {
  //(equation 21)
  Double_t p2=pow(p_lower[1],2)+pow(p_lower[2],2)+pow(p_lower[3],2);
  if ( p2>pow(f_params->f_SM_p_fermi,2) ) return 0.0;
  else return 3.0*f_params->f_N_neutrons/4.0/TMath::Pi()/pow(f_params->f_SM_p_fermi,3);
}

////////////////////////////////////////////////////////////////
Bool_t TU_class::Determine_kprime(Double_t p_lower[4]) 
{
  //(equation 18)
  Double_t e_bind;
  if (f_params->f_do_Smith_Moniz) {
    e_bind=f_params->f_SM_e_bind;
  }
  
  Double_t p_lepprime=sqrt(f_kprime_lower[0]*f_kprime_lower[0]-f_params->f_m_lep*f_params->f_m_lep);
  if (p_lepprime<0.05*f_params->f_m_lep) return 0;
  
  Double_t B=(p_lower[0]-e_bind+f_q_lower[0])*(p_lower[0]-e_bind+f_q_lower[0])-f_params->f_M_p*f_params->f_M_p;
  B*=-1.0;
  for (Int_t i=1;i<4;i++) B+=p_lower[i]*p_lower[i];
  B+=p_lepprime*p_lepprime+f_k_lower[0]*f_k_lower[0]+2.0*p_lower[3]*f_k_lower[0];

  Double_t d=2.0*p_lower[2]*p_lepprime;
  Double_t e=-2.0*(p_lower[3]+f_k_lower[0])*p_lepprime;
  Double_t f=B;

  Double_t a=d*d+e*e;
  Double_t b=2.0*e*f;
  Double_t c=f*f-d*d;

  //these match numerically
  Double_t disc0=b*b-4.0*a*c;
  Double_t disc1=4.0*d*d*(d*d+e*e-f*f);
  Double_t disc2=64.0*pow(p_lepprime,4)*pow(p_lower[2],2);
  disc2*=(pow(p_lower[3]+f_k_lower[0],2)+pow(p_lower[2],2));
  disc2-=16.0*p_lepprime*p_lepprime*p_lower[2]*p_lower[2]*B*B;

  Double_t re_root1,re_root2;
  Double_t im_root1,im_root2;
  if (a==0) {
    if (b==0) return 0;
    else {
      re_root1=-c/b;
      re_root2=re_root1;
    }
  }
  else {
    if (disc1>=0.0) {
      im_root1=0.0;
      im_root2=0.0;

      Double_t t=-0.5;
      if (b>0) t=-0.5*(b+sqrt(disc1));
      if (b<=0) t=-0.5*(b-sqrt(disc1));
      re_root1=t/a;
      if (t!=0.0) re_root2=c/t;
      else re_root2=0.0;
    }
    else {
      re_root1=-b/2.0/a;
      re_root2=re_root1;
      im_root1=sqrt(TMath::Abs(disc1));
      im_root2=-im_root1;
    }
  }

  if (im_root1!=0.0) {
    cout << "re_root1: " << re_root1 << endl;
    cout << "im_root1: " << im_root1 << endl;
    cout << "re_root2: " << re_root2 << endl;
    cout << "im_root2: " << im_root2 << endl;
    return 0;
  }
  if (TMath::Abs(re_root1)>1.0 && TMath::Abs(re_root2)>1.0) return 0;
  //cout << "disc0,disc1,disc2: " << disc0 << "," << disc1 << "," << disc2 << endl;
  //cout << "re_root1,re_root2: " << re_root1 << "," << re_root2 << endl;
  Double_t LHS1=d*sqrt(1.0-re_root1*re_root1);
  Double_t RHS1=e*re_root1+f;
  Double_t LHS2=d*sqrt(1.0-re_root2*re_root2);
  Double_t RHS2=e*re_root2+f;

  //cout << "LHS1,RHS1: " << LHS1 << "," << RHS1 << endl;
  //cout << "LHS2,RHS2: " << LHS2 << "," << RHS2 << endl;
  //cout << "sol1: " << a*re_root1*re_root1+b*re_root1+c << endl;
  //cout << "sol2: " << a*re_root2*re_root2+b*re_root2+c << endl;
  //cout << endl;

  //find the root we want
  Double_t costheta;

  Double_t tol=1.0e-10;
  Bool_t s1=TMath::Abs(LHS1-RHS1)<tol;
  Bool_t s2=TMath::Abs(LHS2-RHS2)<tol;

  if (s1 && s2) {
    f_d3sigma_d3kprime_problems++;
    //cout << "costheta problems!" << endl;
    //cout << "disc0,disc1,disc2: " << disc0 << "," << disc1 << "," << disc2 << endl;
    //cout << "re_root1,re_root2: " << re_root1 << "," << re_root2 << endl;
    //cout << "LHS1,RHS1: " << LHS1 << "," << RHS1 << endl;
    //cout << "LHS2,RHS2: " << LHS2 << "," << RHS2 << endl;
    //cout << "sol1: " << a*re_root1*re_root1+b*re_root1+c << endl;
    //cout << "sol2: " << a*re_root2*re_root2+b*re_root2+c << endl;
    //for (Int_t i=0;i<4;i++) cout << "p_lower[" << i << "]: " << p_lower[i] << endl;
    //cout << endl;

    cout << "d,-e,f: " << d << "," << -e << "," << f << endl;
    for (Int_t i=0;i<4;i++) f_tree_p_lower[i]=p_lower[i];
    f_tree_problem[0]=1;
    f_tree_E_lepprime[0]=f_kprime_lower[0];
    f_tree->Fill();

    //costheta=re_root1;
    return 0;

  }
  else if (s1) {
    for (Int_t i=0;i<4;i++) f_tree_p_lower[i]=p_lower[i];
    f_tree_problem[0]=0;
    f_tree_E_lepprime[0]=f_kprime_lower[0];
    f_tree->Fill();
    costheta=re_root1;
  }
  else if (s2) {
    for (Int_t i=0;i<4;i++) f_tree_p_lower[i]=p_lower[i];
    f_tree_problem[0]=0;
    f_tree_E_lepprime[0]=f_kprime_lower[0];
    f_tree->Fill();
    costheta=re_root2;
  }
  else {
    return 0;
  }

  f_kprime_lower[1]=p_lepprime*sqrt(1.0-costheta*costheta);
  f_kprime_lower[2]=0.0;
  f_kprime_lower[3]=p_lepprime*costheta;

  for (Int_t i=1;i<4;i++) f_q_lower[i]=f_k_lower[i]-f_kprime_lower[i];
  
  f_params->Upper(f_params->f_g,f_k_lower,f_k_upper);
  f_params->Upper(f_params->f_g,f_kprime_lower,f_kprime_upper);
  f_params->Upper(f_params->f_g,f_q_lower,f_q_upper);

  Init_L();

  return 1;
}
