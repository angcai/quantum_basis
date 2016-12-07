#ifndef SPARSE_H
#define SPARSE_H
#include <vector>
#include <forward_list>
#include <armadillo>
#include "mkl_interface.h"

namespace qbasis {
    // Note: sparse matrices in this code are using zero-based convention

    // By default, all diagonal elements are stored, even if they are zero (to be compatible with pardiso)

    static const double sparse_precision = 1e-14;

    inline // double
    bool eigen_by_arma(arma::Col<double> &eigval, arma::Mat<double> &eigvec,
                       const arma::SpMat<double> &X, const arma::uword n_eigvals, const char* form)
    {
        return arma::eigs_sym(eigval, eigvec, X, n_eigvals, form);
    }
    inline // complex double
    bool eigen_by_arma(arma::Col<std::complex<double>> &eigval, arma::Mat<std::complex<double>> &eigvec,
                       const arma::SpMat<std::complex<double>> &X, const arma::uword n_eigvals, const char* form)
    {
        return arma::eigs_gen(eigval, eigvec, X, n_eigvals, form);
    }
    
    template <typename> class csr_mat;
    //template <typename T> void csrXvec(const csr_mat<T>&, const std::vector<T>&, std::vector<T>&);

    template <typename T> struct lil_mat_elem {
        T val;
        MKL_INT col;
    };

    template <typename T> class lil_mat {
        friend class csr_mat<T>;
    public:
        // default constructor
        lil_mat() = default;

        // constructor with the Hilbert space dimension
        lil_mat(const MKL_INT &n, bool sym_ = false) : dim(n), nnz(n), sym(sym_),
        mat(std::vector<std::forward_list<lil_mat_elem<T>>>(n, std::forward_list<lil_mat_elem<T>>(1)))
        {
            mat.shrink_to_fit();
            for (MKL_INT i = 0; i < n; i++) {
                mat[i].front().col = i;
                mat[i].front().val = 0.0;
            }
        }

        // add one element
        void add(const MKL_INT &row, const MKL_INT &col, const T &val);


        // explicitly destroy, free space
        void destroy()
        {
            mat.clear();
            mat.shrink_to_fit();
        }

        // destructor
        ~lil_mat() {};

        MKL_INT dimension() const { return dim; }

        MKL_INT num_nonzero() const { return nnz; }

        // print
        void prt();

    private:
        MKL_INT dim;    // dimension of the matrix
        MKL_INT nnz;    // number of non-zero entries
        bool sym;       // if storing only upper triangle
        std::vector<std::forward_list<lil_mat_elem<T>>> mat;
    };


    // 3-array form of csr sparse matrix format, zero based
    template <typename T> class csr_mat {
        //    friend void csrXvec <> (const csr_mat<T>&, const std::vector<T>&, std::vector<T>&);
    public:
        // default constructor
        csr_mat() = default;

        // construcotr from a lil_mat, and if sym_ == true, use only the upper triangle
        csr_mat(const lil_mat<T> &old);

        // explicitly destroy, free space
        void destroy()
        {
            if(val != nullptr) {
                delete [] val;
                val = nullptr;
            }
            if(ja != nullptr){
                delete [] ja;
                ja = nullptr;
            }
            if(ia != nullptr){
                delete [] ia;
                ia = nullptr;
            }
        }

        // destructor
        ~csr_mat()
        {
            if(val != nullptr) delete [] val;
            if(ja != nullptr) delete [] ja;
            if(ia != nullptr) delete [] ia;
        }

        // matrix vector product
        void MultMv(const T *x, T *y) const;

        // matrix matrix product, x and y of shape dim * n
        //void MultMm(const T *x, T *y, MKL_INT n) const;


        MKL_INT dimension() const {return dim; }

        // print
        void prt();

        // transform to armadillo sparse matrix, written in dirty way, discard when the handed coded lanczos become stable
        void to_arma(arma::SpMat<T> &csc_sparse);

    private:
        MKL_INT dim;
        MKL_INT nnz;        // number of non-zero entries
        bool sym;           // if storing only upper triangle
        T *val;
        MKL_INT *ja;
        MKL_INT *ia;
    };

}


#endif
