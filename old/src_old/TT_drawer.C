#define TT_drawer_cxx
#include "TT_drawer.h"
#include "TT_event.h"

////////////////////////////////////////////////////////////////////////
Double_t TT_drawer::Flux_histo_height()
{
  Double_t Enu=f_event->f_k_lower[0];
  return f_params->f_flux_histo->GetBinContent(f_params->f_flux_histo->FindBin(Enu));
}

////////////////////////////////////////////////////////////////////////
void TT_drawer::Init_randomly()
{
  f_height=0.0;
  Int_t iSuccess=0;
  while (iSuccess<f_params->f_N_successes) {
    Draw_point();
    Double_t result=f_event->f_dsigma_dall*Flux_histo_height();
    if (result!=0.0) iSuccess++;
    if (result>f_height) {
      f_height=result;
    }
  }
  f_height *=f_params->f_rate_factor;
}

////////////////////////////////////////////////////////////////////////
void TT_drawer::Compute_integral()
{
  f_integral=f_height;
  f_integral*=(f_Enu_max  -f_Enu_min);
  f_integral*=(f_w_max    -f_w_min);
  if (f_process>0) {
    f_integral*=(f_qbold_max-f_qbold_min);
    f_integral*=(f_mag_p_max-f_mag_p_min);
    f_integral*=(f_phi_p_max-f_phi_p_min);
  }
  if (f_process==3) f_integral*=(f_cos_theta_pq_max-f_cos_theta_pq_min);
}

////////////////////////////////////////////////////////////////////////
Bool_t TT_drawer::Draw_point()
{
  Double_t Enu=f_params->f_rand.Uniform(f_Enu_min,f_Enu_max);
  Double_t w=f_params->f_rand.Uniform(f_w_min,f_w_max);
  if (w>Enu-f_params->f_m_lep) return kFALSE;

  if (f_process==0) f_event->Init(Enu,w);
  else {
    Double_t p_lep=sqrt((Enu-f_w_min)*(Enu-f_w_min)-(f_params->f_m_lep)*(f_params->f_m_lep));
    Double_t qbold_min=Enu-p_lep;
    Double_t qbold_max=Enu+p_lep;
    Double_t qbold=f_params->f_rand.Uniform(f_qbold_min,f_qbold_max);
    if (qbold<qbold_min || qbold>qbold_max) return kFALSE;

    Double_t mag_p=f_params->f_rand.Uniform(f_mag_p_min,f_mag_p_max);
    Double_t phi_p=f_params->f_rand.Uniform(f_phi_p_min,f_phi_p_max);

    if (f_process==3) {
      Double_t cos_theta_pq=f_params->f_rand.Uniform(-1.0,1.0);
      f_event->Init(Enu,w,qbold,mag_p,cos_theta_pq,phi_p);
    }
    else f_event->Init(Enu,w,qbold,mag_p,phi_p);
  }

  if (f_event->f_bad_event) return kFALSE;
  f_event->Evaluate_dsigma_dall();
  Double_t rate=f_event->f_dsigma_dall*Flux_histo_height();
  if (f_height<rate) {
    printf("drawer err: proc=%d; bin=%d; ratio=%8.6g; h=%8.6g; rate=%8.6g;\n",f_process,f_bin,rate/f_height,f_height,rate);
  }
  
  //rejection method
  Double_t y=f_params->f_rand.Uniform(0.0,f_height);
  if (y<rate) return kTRUE;
  else return kFALSE;
}

