#include "irlm.h"
#include "it_dmrg.h"
#include "it_tdvp.h"

#include <iostream>
#include <iomanip>
#include <fstream>

using namespace std;

/// ./irlm_star <len> [star]
int main(int argc, char **argv)
{
    bool star=true;
    int len=20;
    if (argc>=2) len=atoi(argv[1]);
    if (argc==3) star=atoi(argv[2]);
    IRLM model {.L=len, .t=0.5, .V=0.1, .U=0.25, .ed=-10};
    HamSys sys= star ? model.HamStar() : model.Ham();
    cout<<setprecision(14);
    cout<<"bond dimension of H: "<< maxLinkDim(sys.ham) << endl;

    // solve the gs of system
    it_dmrg sol_gs {sys};
    sol_gs.bond_dim=128;
    sol_gs.noise=1e-3;
    cout<<"\nsweep bond-dim energy\n";
    for(auto i=0u; i<6; i++) {
        if (i==3) {
            sol_gs.noise=1e-8;
            sol_gs.nIter_diag=32;
        }
        sol_gs.iterate();
        cout<<i+1<<" "<<maxLinkDim(sol_gs.psi)<<" "<<sol_gs.energy<<endl;
    }

    //sol_gs.psi.orthogonalize({"Cutoff",1e-9});
    for(auto i=0; i<sol_gs.psi.length(); i++)
        cout<<itensor::leftLinkIndex(sol_gs.psi,i+1).dim()<<" ";
    cout << "\n";
     //return 0;

    cout<<"\n-------------------------- evolve the psi with new Hamiltonian ----------------\n";

    IRLM model2 {.L=len, .t=0.5, .V=0.1, .U=0.25, .ed=0.0};
    auto sys2= star ? model2.HamStar() : model2.Ham();
    cout<<"bond dimension of H: "<< maxLinkDim(sys2.ham) << endl;
    it_tdvp sol {sys2, sol_gs.psi};
    sol.bond_dim=512;
    sol.err_goal=1e-7;
    sol.epsilonM=1e-4;
    sol.do_normalize=true;
    sol.rho_cutoff=1e-14;
    sol.silent=false;
    ofstream out("irlm_star_L"s+to_string(sol.hamsys.ham.length())+".txt");
    out<<"sweep bond-dim energy n0\n"<<setprecision(14);
    out<<"0 "<<maxLinkDim(sol.psi)<<" "<<sol.energy<<" "<<itensor::expectC(sol.psi, sol.hamsys.sites, "N",{1}).at(0).real()<<endl;
    for(auto i=0u; i<len*10/2; i++) {
        sol.epsilonM=(i%10==0) ? 1e-4 : 0;
        if (i%10==0) {
            auto cc=Fermionic::cc_matrix(sol.psi, sol.hamsys.sites);
            cc.diag().raw_print("ni=");
            string filename="eval_L"s+to_string(sol.hamsys.ham.length())+"_t"+to_string(i)+".txt";
            arma::eig_sym(cc).save(filename,arma::raw_ascii);
        }
        sol.iterate();
        double n0=itensor::expectC(sol.psi, sol.hamsys.sites, "N",{1}).at(0).real();
        out<<(i+1)*abs(sol.dt)<<" "<<maxLinkDim(sol.psi)<<" "<<sol.energy<<" "<<n0<<endl;
    }

    return 0;
}

