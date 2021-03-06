////list of ambiguities/issues
//--write some docs (implementing nucl-th/0512004v4)
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
//--Double_t ok?
//--negative E/limits of integration for E?
//----has implications for norm. of SM spectral function
//----E lower threshold for binding e near 0
//--virtual functions?/fix up OO stuff
//--single nucleon lower energy thershold
//--Llewellyn-Smith does not make sense at high Q2
//
//--test small values of eb, pf
//--W boson mass?
//--how to find p_max for spectral functions

//--clean up TT_event constructor, functions, etc.
//--draw_point qbold rejection
//--cmake compiler setting
//--w<e_b ok?
//--hadronic tensor evaluation for AS_MF and AS_corr
//--muon phi/direction of qbold
//--factor of pi between AS and their reference?
//--LS comparison
//--try nu_e events
//--flexible flux file
//--fix height finding
//--check sign of E_b in TT_event::compute_enuqe_and_q2qe
//--check enuqe and Q2qe formulas
//--process==3 in setup_kinematics
//--run valgrind
//--in particular, see Init_valgrind
//--efficiency at high neutrino energy
//--e scattering

//--types for point counters
//--types in write_shuffled_tree
//--make more events than we want?
//--shuffle tree
//--put date/random label on tmp file

//--delete tree in TT_generator.h?
//--somehow ensure that flux histo is a TH1F
//--low bin rates
//--interpolation/turning off bins
//--SM isobar?

//--sigma, omega
//http://www.slac.stanford.edu/spires/find/hep/www?rawcmd=FIND+K+SIGMA+OMEGA+MODEL&FORMAT=www&SEQUENCE=ds
//http://www.slac.stanford.edu/spires/find/hep/www?irn=1301462
//http://www.slac.stanford.edu/spires/find/hep/www?irn=1333011
//http://www.slac.stanford.edu/spires/find/hep/www?irn=1498711


#include <iostream>
#include "Rtypes.h"
#include "TT_params.h"
#include "TT_nucleus.h"
#include "TT_generator.h"

using namespace std;

////////////////////////////////////////////////////////////////////////
Int_t main(Int_t argc,char **argv) {
  Bool_t help=0;

  if (help) {
    cout << "--USAGE: " << argv[0] << " CONFIG_FILE" << endl;
    cout << "--for further help, see readme" << endl;
    return 1;
  }

  Int_t* init_styles=0;
  Int_t NRuns=0;
  if (argc==1) {
    NRuns=1;
    init_styles=new Int_t[NRuns];
    init_styles[0]=-1;
  }
  if (argc>1) {
    NRuns=7;
    init_styles=new Int_t[NRuns];
    for (Int_t iRun=0;iRun<NRuns;iRun++) {
      init_styles[iRun]=iRun;
    }
  }

  TT_params params(argv[1]);
  params.Set_seed(0);

  for (Int_t iRun=0;iRun<NRuns;iRun++) {
    params.Init(init_styles[iRun]);
    Bool_t params_good=params.Check_for_problems();
    if (!params_good) return 1;

    TT_nucleus nucleus(8,8);
    //nucleus.n_plot();
  
    TT_generator gen(&params,&nucleus);
    gen.Setup_processes();
    Bool_t status=gen.Generate_events();
    gen.Make_graphs();
    gen.Write_shuffled_tree();
  }

  delete [] init_styles;
  return 0;
}
