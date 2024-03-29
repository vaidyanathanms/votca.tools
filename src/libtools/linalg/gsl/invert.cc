/* 
 * Copyright 2009-2015 The VOTCA Development Team (http://www.votca.org)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <votca/tools/linalg.h>
#include <boost/numeric/ublas/matrix_proxy.hpp>

#include <gsl/gsl_linalg.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_eigen.h>

namespace votca { namespace tools {

using namespace std;


void linalg_invert( ub::matrix<double> &A, ub::matrix<double> &V){
        // matrix inversion using gsl
        
        gsl_error_handler_t *handler = gsl_set_error_handler_off();
	const size_t N = A.size1();
	// signum s (for LU decomposition)
	int s;
        //make copy of A as A is destroyed by GSL
        ub::matrix<double> work=A;
        V.resize(N, N, false);
        
	// Define all the used matrices
        gsl_matrix_view A_view = gsl_matrix_view_array(&work(0,0), N, N);
        gsl_matrix_view V_view = gsl_matrix_view_array(&V(0,0), N, N);
	gsl_permutation * perm = gsl_permutation_alloc (N);
        
	// Make LU decomposition of matrix A_view
	gsl_linalg_LU_decomp (&A_view.matrix, perm, &s);

	// Invert the matrix A_view
	int status = gsl_linalg_LU_invert (&A_view.matrix, perm, &V_view.matrix);

        gsl_set_error_handler(handler);
        
	// return (status != 0);
}

}}
