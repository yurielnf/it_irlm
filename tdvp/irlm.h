#ifndef IRLM_H
#define IRLM_H


#include "fermionic.h"
#include <tuple>

struct IRLM {
    int L=20;
    double t=0.5;
    double V=0.15;
    double U=-0.5;
    double ed=0;
    bool impCenter=false;       ///< this only affects the star geometry

    std::vector<int> impPos() const { return impCenter ? std::vector {L/2-1, L/2} : std::vector {0,1}; }
    std::vector<int> bathPos() const
    {
        auto ipos=impPos();
        std::vector<int> bpos;
        for(auto i=0; i<L; i++)
            if (! std::binary_search(ipos.begin(), ipos.end(), i) )
                bpos.push_back(i);
        return bpos;
    }

    auto matrices() const
    {
        // Kinetic energy TB Hamiltonian
        auto imp=impPos();
        auto bath=bathPos();
        arma::mat K(L,L,arma::fill::zeros);
        for(auto i=0u; i+1<bath.size(); i++)
            K(bath[i],bath[i+1])=K(bath[i+1],bath[i])=t;
        K(imp[1],bath[0])=K(bath[0],imp[1])=t;
        K(imp[0],imp[1])=K(imp[1],imp[0])=V;
        K(imp[0],imp[0])=ed;

        // U ni nj
        arma::mat Umat(L,L,arma::fill::zeros);
        Umat(imp[0],imp[1])=U;
        K(imp[1],imp[1])=-U/2;
        K(imp[0],imp[0])+=-U/2;
        return std::make_pair(K,Umat);
    }

    auto rotStar() const
    {
        auto [K, _]=matrices();
        // Diagonalize the bath
        arma::mat Rfull(L,L,arma::fill::eye);
        auto bath=arma::conv_to<arma::uvec>::from(bathPos());
        arma::vec ek;
        arma::mat R;
        arma::eig_sym( ek, R, K.submat(bath,bath) );
        if (!impCenter) {
            arma::uvec iek=arma::sort_index( arma::abs(ek) );
            Rfull(bath,bath)=R.cols(iek);
        }
        else
            Rfull(bath,bath)=R;
        return Rfull;
    }

//    auto rotStarC() const
//    {
//        using namespace arma;
//        auto [K, _]=matrices();
//        // Diagonalize the bath
//        arma::vec ek;
//        arma::mat R;
//        arma::eig_sym( ek, R, K.submat(2,2,L-1,L-1) );
//        arma::mat Rfull(L,L,arma::fill::eye);

//        Rfull.submat(2,2,L-1,L-1)=R;
//        Rfull.swap_cols(0,L/2-1);
//        Rfull.swap_cols(1,L/2);
//        return Rfull;
//    }

    HamSys Ham(arma::mat const& rot={}, bool rotateOnlyKin=false) const
    {
        auto [K,Umat]=matrices();
        if (rot.empty()) return Fermionic(K,Umat).Ham();        
        if (rotateOnlyKin) return Fermionic(rot.t()*K*rot, Umat).Ham();
        return Fermionic(K,Umat,rot).Ham();
    }

    HamSys HamStar() const { return Ham(rotStar(), true); }

    HamSys HamNO(arma::mat const& cc, bool fromStar=true) const { auto R=Fermionic::rotNO(cc); if (fromStar) R=R*rotStar(); return Ham(R); }

};


#endif // IRLM_H
