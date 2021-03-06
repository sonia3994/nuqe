#define TT_generator2_cxx
#include "TT_generator2.h"
#include "TGraph.h"

////////////////////////////////////////////////////////////////////////
void TT_generator2::Setup_processes()
{
  f_total_accepted=0;
  for (Int_t iProcess=0;iProcess<f_N_Processes;iProcess++) {
    f_process[iProcess]=iProcess;
    f_accepted_points[iProcess]=0;
    f_total_points[iProcess]=0;
    f_rate_est[iProcess]=0.0;
    f_drawer[iProcess]=0;

    if (!f_params->f_processes_on[iProcess]) continue;

    f_rate_est[iProcess]=1.0/f_N_Processes;
    f_drawer[iProcess] = new TT_drawer(f_flux_histo,f_params,f_nucleus,f_process[iProcess]);
    f_drawer[iProcess]->Init_randomly();
    f_drawer[iProcess]->Compute_integral();
  }
  cout << "Generator2 initialized." << endl;
}

////////////////////////////////////////////////////////////////////////
Bool_t TT_generator2::Generate_events()
{
  f_keep_going=1;
  f_event_display=1;

  while (f_keep_going) {
    Long64_t total_accepted_old=f_total_accepted;

    for (Int_t iProcess=0;iProcess<f_N_Processes;iProcess++) {
      if ((!f_params->f_processes_on[iProcess]) || (!f_keep_going) || Reject_process(iProcess)) continue;
      Bool_t got_one=0;
      while (!got_one) {
	f_total_points[iProcess]++;
	got_one=f_drawer[iProcess]->Draw_point();
      }

      f_accepted_points[iProcess]++;
      f_total_accepted++;

      f_keep_going=Keep_going();
      Report_progress();
      Fill_tree(f_drawer[iProcess]->f_event);
      Update_rates(iProcess);    
    } //end process loop

    if (total_accepted_old==f_total_accepted) {
      cout << "caught in an infinite loop" << endl;
      return kFALSE;
    }

  } //end accepted_event loop

  return kTRUE;
}


////////////////////////////////////////////////////////////////////////
Bool_t TT_generator2::Keep_going()
{
  Bool_t ans=f_total_accepted<f_total_to_accept;
  for (Int_t iProcess=0;iProcess<f_N_Processes;iProcess++) ans = ans || (f_params->f_processes_on[iProcess] && f_accepted_points[iProcess]<2);
  return ans;
}

////////////////////////////////////////////////////////////////////////
void TT_generator2::Report_progress()
{
  if (f_total_accepted==f_event_display || !f_keep_going) {
    cout << "total accepted: " << f_total_accepted << endl;
    f_event_display*=2;
  }
}   

////////////////////////////////////////////////////////////////////////
Bool_t TT_generator2::Reject_process(Int_t iProc)
{
  Long64_t accepted_total=0;
  Double_t rate_total=0.0;
  for (Int_t jProcess=0;jProcess<f_N_Processes;jProcess++) {
    accepted_total+=f_accepted_points[jProcess];
    rate_total+=f_rate_est[jProcess];
  }
  Double_t frac_desired=f_rate_est[iProc]/rate_total;
  Double_t frac_actual =(f_accepted_points[iProc]+0.0)/f_total_accepted;
  return frac_actual>frac_desired;
}

////////////////////////////////////////////////////////////////////////
void TT_generator2::Fill_tree(TT_event *event)
{
  for (Int_t iComp=0;iComp<4;iComp++) {
    f_tree_k     [iComp]=event->f_k_lower     [iComp];
    f_tree_kprime[iComp]=event->f_kprime_lower[iComp];
    f_tree_q     [iComp]=event->f_q_lower     [iComp];
    f_tree_p     [iComp]=event->f_p_lower     [iComp];
    f_tree_pprime[iComp]=event->f_pprime_lower[iComp];
  }
  f_tree_xs=event->f_dsigma_dall;
  f_tree_mag_p=event->f_mag_p;
  f_tree_cth_pq=event->f_cos_theta_pq;
  f_tree_phi_p=event->f_phi_p;
  f_tree_process=event->f_process;
  f_tree->Fill();
}

////////////////////////////////////////////////////////////////////////
void TT_generator2::Update_rates(Int_t iProc)
{
  Double_t eff=(f_accepted_points[iProc]+0.0)/f_total_points[iProc];
  f_rate_est[iProc]=eff*f_drawer[iProc]->f_integral;
  f_rate_err[iProc]=sqrt(eff*(1.0-eff)/f_total_points[iProc])*f_drawer[iProc]->f_integral;
}

////////////////////////////////////////////////////////////////////////
void TT_generator2::Finish_up()
{
  for (Int_t iProcess=0;iProcess<f_N_Processes;iProcess++) {
    if (!f_params->f_processes_on[iProcess]) continue;
    Double_t eff=(f_accepted_points[iProcess]+0.0)/f_total_points[iProcess];
    printf("process: %d, eff.: %6.4g, rate: %6.4g, rate_error: %6.4g\n",f_process[iProcess],eff,f_rate_est[iProcess],f_rate_err[iProcess]);
  }
  
  TGraph graph_rate      (f_N_Processes,(Double_t*)f_process,f_rate_est);
  TGraph graph_rate_error(f_N_Processes,(Double_t*)f_process,f_rate_err);
  
  graph_rate.SetName("ccqe_rate");
  graph_rate_error.SetName("ccqe_rate_error");
  
  f_tree->Write();
  graph_rate.Write();
  graph_rate_error.Write();
}
