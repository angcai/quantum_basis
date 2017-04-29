#include <iostream>
#include <iomanip>
#include "qbasis.h"

// Heisenberg model on triangular lattice
int main() {
    std::cout << std::setprecision(10);
    // parameters
    bool matrix_free = false;
    double J1 = 1.0;
    int Lx = 4;
    int Ly = 2;
    double Sz_total_val = 0.0;

    std::cout << "Lx =      " << Lx << std::endl;
    std::cout << "Ly =      " << Ly << std::endl;
    std::cout << "J1 =      " << J1 << std::endl;
    std::cout << "Sz =      " << Sz_total_val << std::endl << std::endl;

    // lattice object
    std::vector<std::string> bc{"pbc", "pbc"};
    qbasis::lattice lattice("triangular",std::vector<uint32_t>{static_cast<uint32_t>(Lx), static_cast<uint32_t>(Ly)},bc);


    // local matrix representation
    std::vector<std::vector<std::complex<double>>> Splus(2,std::vector<std::complex<double>>(2));
    std::vector<std::vector<std::complex<double>>> Sminus(2,std::vector<std::complex<double>>(2));
    std::vector<std::complex<double>> Sz(2);
    Splus[0][0]  = 0.0;
    Splus[0][1]  = 1.0;
    Splus[1][0]  = 0.0;
    Splus[1][1]  = 0.0;
    Sminus[0][0] = 0.0;
    Sminus[0][1] = 0.0;
    Sminus[1][0] = 1.0;
    Sminus[1][1] = 0.0;
    Sz[0]        = 0.5;
    Sz[1]        = -0.5;


    // constructing the Hamiltonian in operator representation
    qbasis::model<std::complex<double>> Heisenberg;
    Heisenberg.add_orbital(lattice.total_sites(), "spin-1/2");
    qbasis::mopr<std::complex<double>> Sz_total;   // operators representating total Sz
    for (int m = 0; m < Lx; m++) {
        for (int n = 0; n < Ly; n++) {
            uint32_t site_i, site_j;
            lattice.coor2site(std::vector<int>{m,n}, 0, site_i); // obtain site label of (x,y)
            // construct operators on each site
            auto Splus_i   = qbasis::opr<std::complex<double>>(site_i,0,false,Splus);
            auto Sminus_i  = qbasis::opr<std::complex<double>>(site_i,0,false,Sminus);
            auto Sz_i      = qbasis::opr<std::complex<double>>(site_i,0,false,Sz);

            // to neighbor a_1
            if (bc[0] == "pbc" || (bc[0] == "obc" && m < Lx - 1)) {
                lattice.coor2site(std::vector<int>{m+1,n}, 0, site_j);
                auto Splus_j   = qbasis::opr<std::complex<double>>(site_j,0,false,Splus);
                auto Sminus_j  = qbasis::opr<std::complex<double>>(site_j,0,false,Sminus);
                auto Sz_j      = qbasis::opr<std::complex<double>>(site_j,0,false,Sz);
                Heisenberg.add_offdiagonal_Ham(std::complex<double>(0.5*J1,0.0) * (Splus_i * Sminus_j + Sminus_i * Splus_j));
                Heisenberg.add_diagonal_Ham(std::complex<double>(J1,0.0) * ( Sz_i * Sz_j ));
            }

            // to neighbor a_2
            if (bc[1] == "pbc" || (bc[1] == "obc" && n < Ly - 1)) {
                lattice.coor2site(std::vector<int>{m,n+1}, 0, site_j);
                auto Splus_j   = qbasis::opr<std::complex<double>>(site_j,0,false,Splus);
                auto Sminus_j  = qbasis::opr<std::complex<double>>(site_j,0,false,Sminus);
                auto Sz_j      = qbasis::opr<std::complex<double>>(site_j,0,false,Sz);
                Heisenberg.add_offdiagonal_Ham(std::complex<double>(0.5*J1,0.0) * (Splus_i * Sminus_j + Sminus_i * Splus_j));
                Heisenberg.add_diagonal_Ham(std::complex<double>(J1,0.0) * ( Sz_i * Sz_j ));
            }

            // to neighbor a_3
            if ((bc[0] == "pbc" || (bc[0] == "obc" && m > 0)) &&
                (bc[1] == "pbc" || (bc[1] == "obc" && n < Ly - 1))) {
                lattice.coor2site(std::vector<int>{m-1,n+1}, 0, site_j);
                auto Splus_j   = qbasis::opr<std::complex<double>>(site_j,0,false,Splus);
                auto Sminus_j  = qbasis::opr<std::complex<double>>(site_j,0,false,Sminus);
                auto Sz_j      = qbasis::opr<std::complex<double>>(site_j,0,false,Sz);
                Heisenberg.add_offdiagonal_Ham(std::complex<double>(0.5*J1,0.0) * (Splus_i * Sminus_j + Sminus_i * Splus_j));
                Heisenberg.add_diagonal_Ham(std::complex<double>(J1,0.0) * ( Sz_i * Sz_j ));
            }

            // total Sz operator
            Sz_total += Sz_i;
        }
    }

    // constructing the Hilbert space basis
    Heisenberg.enumerate_basis_full(lattice, {Sz_total}, {Sz_total_val});


    if (! matrix_free) {
        // generating matrix of the Hamiltonian in the full Hilbert space
        Heisenberg.generate_Ham_sparse_full();
        std::cout << std::endl;
    }


    // obtaining the eigenvals of the matrix
    Heisenberg.locate_E0_full(10,20,matrix_free);
    std::cout << std::endl;


    // for the parameters considered, we should obtain:
    assert(std::abs(Heisenberg.eigenvals_full[0] + 6.0) < 1e-8);
}
