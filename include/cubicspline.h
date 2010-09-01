/* 
 * Copyright 2009 The VOTCA Development Team (http://www.votca.org)
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

#ifndef _CUBICSPLINE_H
#define	_CUBICSPLINE_H

#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/vector_proxy.hpp>
#include <boost/numeric/ublas/vector_expression.hpp>
#include <iostream>

namespace votca { namespace tools {

namespace ub = boost::numeric::ublas;

/**
    \brief A cubic spline class
  
    This class does cubic piecewise spline interpolation and spline fitting.
    As representation of a single spline, the general form
    \f[
        S_i(x) = A(x,h_i) f_i + B(x,h_i) f_{i+1} + C(x,h_i) f''_i + d(x,h_i) f''_{i+1}
    \f]
    with
    \f[
        x_i \le x < x_{i+1}\,,\\
        h_i = x_{i+1} - x_{i}
    \f]
    The \f$f_i\,,\,,f''_i\f$ are the function values and second derivates
    at point \f$x_i\f$.

    The parameters \f$f''_i\f$ are no free parameters, they are determined by the 
    smoothing condition that the first derivatives are continuous. So the only free
    paramers are the grid points x_i as well as the functon values f_i at these points. A spline can be generated in several ways:
    - Interpolation spline
    - Fitting spline (fit to noisy data)
    - calculate the parameters elsewere and fill the spline class
 */

class CubicSpline
{    
public:
    // default constructor
    CubicSpline() :
        _boundaries(splineNormal) {}
    
    // destructor
    ~CubicSpline() {};    
    
    /// enum for type of boundary condition
    enum eBoundary {
        splineNormal = 0,  ///< normal boundary conditions: \f$f_0=f_N=0\f$ 
        splinePeriodic    ///< periodic boundary conditions: \f$f_0=f_N\f$ 
    };
    
    /// set the boundary type of the spline
    void setBC(eBoundary bc) {_boundaries = bc; }
    
    /// Generates the r_k, returns the number of grid points
    /// max is included in the interval.
    int GenerateGrid(double min, double max, double h);
    
    /// determine the interval the point r is in
    /// returns i for interval r_i r_{i+1}, -1 for out of range
    int getInterval(const double &r);
    
    /// ERROR-PRONE implementation, make it better!!!
    double getGridPoint(const int &i) {return _r[i];}

    // give string in rangeparser format: e.g. 1:0.1:10;11:1:20
    //int GenerateGrid(string range);

    /// \brief construct an interpolation spline
    ///
    ///   x, y are the the points to construct interpolation,
    /// both vectors must be of same size
    void Interpolate(ub::vector<double> &x, ub::vector<double> &y);
    
    /// \brief fit spline through noisy data
    ///
    /// x,y are arrays with noisy data, both vectors must be of same size
    void Fit(ub::vector<double> &x, ub::vector<double> &y);
    
    
    /// Calculate the function value
    double Calculate(const double &x);

    /// Calculate the function derivative
    double CalculateDerivative(const double &x);
    
    
    /// Calculate the function value for a whole array, story it in y
    template<typename vector_type1, typename vector_type2>
    void Calculate(vector_type1 &x, vector_type2 &y);
    template<typename vector_type1, typename vector_type2>
    void CalculateDerivative(vector_type1 &x, vector_type2 &y);
       
    /// the spline parameters were calculated elsewere, store that data
    template<typename vector_type>
    void setSplineData(vector_type &f, vector_type &f2) { _f = f; _f2 = f2;}
    
    /// print out results
    void Print(std::ostream &out, double interval = 0.0001 );
        
    /// get the grid array x
    ub::vector<double> &getX() {return _r; }
    ///  \brief get the spline data
    ub::vector<double> &getSplineF() { return _f; }
    ///  \brief get second derivatives
    ub::vector<double> &getSplineF2() { return _f2; }
    
    // stuff to construct fitting matrices
    
    /// \brief add a point to fitting matrix
    ///
    /// When creating a matrix to fit data with a spline, this function creates
    /// one entry in that fitting matrix. 
    template<typename matrix_type>
    void AddToFitMatrix(matrix_type &A, double x,
            int offset1, int offset2=0, double scale=1);
    
    template<typename matrix_type, typename vector_type>
    void AddToFitMatrix(matrix_type &M, vector_type &x, 
            int offset1, int offset2=0);
    
    // add the boundary conditions
    template<typename matrix_type>
    void AddBCToFitMatrix(matrix_type &A,
            int offset1, int offset2=0);


protected:    
    // the grid points
    ub::vector<double> _r;
    
    // y values of grid points
    ub::vector<double> _f;
    // second derivatives of grid points
    ub::vector<double> _f2;

    eBoundary _boundaries;
    
    // A spline can be written in the form
    // S_i(x) =   A(x,x_i,x_i+1)*f_i     + B(x,x_i,x_i+1)*f''_i 
    //          + C(x,x_i,x_i+1)*f_{i+1} + D(x,x_i,x_i+1)*f''_{i+1}
    double A(const double &r);
    double B(const double &r);
    double C(const double &r);
    double D(const double &r);

    double Aprime(const double &r);
    double Bprime(const double &r);
    double Cprime(const double &r);
    double Dprime(const double &r);
  
    // tabulated derivatives at grid points. Second argument: 0 - left, 1 - right    
    double A_prime_l(int i);     
    double A_prime_r(int i);     
    double B_prime_l(int i);    
    double B_prime_r(int i);    
    double C_prime_l(int i);
    double C_prime_r(int i);
    double D_prime_l(int i);
    double D_prime_r(int i);
};

inline int CubicSpline::GenerateGrid(double min, double max, double h)
{
    int vec_size = 1 + (int)((max-min)/h);  //check it!
    _r.resize(vec_size);
    int i;
   
    double r_init;
   
    for (r_init = min, i=0; i < vec_size - 1; r_init += h ) {
            _r[i++]= r_init;
    }
    _r[i] = max;
    _f.resize(_r.size(), false);
    _f2.resize(_r.size(), false);
    return _r.size();
}

inline double CubicSpline::Calculate(const double &r)
{
    int interval =  getInterval(r);
    return  A(r)*_f[interval] 
            + B(r)*_f[interval + 1] 
            + C(r)*_f2[interval]
            + D(r)*_f2[interval + 1];
}

inline double CubicSpline::CalculateDerivative(const double &r)
{
    int interval =  getInterval(r);
    return  Aprime(r)*_f[interval]
            + Bprime(r)*_f[interval + 1]
            + Cprime(r)*_f2[interval]
            + Dprime(r)*_f2[interval + 1];
}

template<typename vector_type1, typename vector_type2>
inline void CubicSpline::Calculate(vector_type1 &x, vector_type2 &y)
{
    for(size_t i=0; i<x.size(); ++i) 
        y(i) = Calculate(x(i));
}
template<typename vector_type1, typename vector_type2>
inline void CubicSpline::CalculateDerivative(vector_type1 &x, vector_type2 &y)
{
    for(size_t i=0; i<x.size(); ++i)
        y(i) = CalculateDerivative(x(i));
}
inline void CubicSpline::Print(std::ostream &out, double interval)
{
    for (double x = _r[0]; x < _r[_r.size() - 1]; x += interval)
        out << x << " " << Calculate(x) << "\n";    
}

template<typename matrix_type>
inline void CubicSpline::AddToFitMatrix(matrix_type &M, double x, 
            int offset1, int offset2, double scale)
{
    int spi = getInterval(x);
    M(offset1, offset2 + spi) += A(x)*scale;
    M(offset1, offset2 + spi+1) += B(x)*scale;
    M(offset1, offset2 + spi + _r.size()) += C(x)*scale;
    M(offset1, offset2 + spi + _r.size() + 1) += D(x)*scale;
}

template<typename matrix_type, typename vector_type>
inline void CubicSpline::AddToFitMatrix(matrix_type &M, vector_type &x, 
            int offset1, int offset2)
{
    for(size_t i=0; i<x.size(); ++i) {
        int spi = getInterval(x(i));
        M(offset1+i, offset2 + spi) = A(x(i));
        M(offset1+i, offset2 + spi+1) = B(x(i));
        M(offset1+i, offset2 + spi + _r.size()) = C(x(i));
        M(offset1+i, offset2 + spi + _r.size() + 1) = D(x(i));
    }
}

template<typename matrix_type>
inline void CubicSpline::AddBCToFitMatrix(matrix_type &M,
            int offset1, int offset2)
{
    for(size_t i=0; i<_r.size() - 2; ++i) {
            M(offset1+i+1, offset2 + i) = A_prime_l(i);
            M(offset1+i+1, offset2 + i+1) = B_prime_l(i) - A_prime_r(i);
            M(offset1+i+1, offset2 + i+2) = -B_prime_r(i);

            M(offset1+i+1, offset2 + _r.size() + i) = C_prime_l(i);
            M(offset1+i+1, offset2 + _r.size() + i+1) = D_prime_l(i) - C_prime_r(i);
            M(offset1+i+1, offset2 + _r.size() + i+2) = -D_prime_r(i);
    }
    // currently only natural boundary conditions:
    switch(_boundaries) {
        case splineNormal:
            M(offset1, offset2 + _r.size()) = 1;
            M(offset1 + _r.size() - 1, offset2 + 2*_r.size()-1) = 1;
            break;
        case splinePeriodic:
            M(offset1, offset2) = 1;
            M(offset1, offset2 + _r.size()-1) = -1;
            M(offset1 + _r.size() - 1, offset2 + _r.size()) = 1;
            M(offset1 + _r.size() - 1, offset2 + 2*_r.size()-1) = -1;
            break;
    }
    
}


inline double CubicSpline::A(const double &r)
{
    return ( 1.0 - (r -_r[getInterval(r)])/(_r[getInterval(r)+1]-_r[getInterval(r)]) );
}

inline double CubicSpline::Aprime(const double &r)
{
    return  -1.0/(_r[getInterval(r)+1]-_r[getInterval(r)]);
}

inline double CubicSpline::B(const double &r)
{
    return  (r -_r[getInterval(r)])/(_r[getInterval(r)+1]-_r[getInterval(r)]) ;
}

inline double CubicSpline::Bprime(const double &r)
{
    return  1.0/(_r[getInterval(r)+1]-_r[getInterval(r)]);
}

inline double CubicSpline::C(const double &r)
{
    double xxi, h;
    xxi = r -_r[getInterval(r)];
    h   = _r[getInterval(r)+1]-_r[getInterval(r)];
    
    return ( 0.5*xxi*xxi - (1.0/6.0)*xxi*xxi*xxi/h - (1.0/3.0)*xxi*h) ;
}
inline double CubicSpline::Cprime(const double &r)
{
    double xxi, h;
    xxi = r -_r[getInterval(r)];
    h   = _r[getInterval(r)+1]-_r[getInterval(r)];

    return (xxi - 0.5*xxi*xxi/h - h/3);
}
inline double CubicSpline::D(const double &r)
{
    double xxi, h;
    xxi = r -_r[getInterval(r)];
    h   = _r[getInterval(r)+1]-_r[getInterval(r)]; 
    
    return ( (1.0/6.0)*xxi*xxi*xxi/h - (1.0/6.0)*xxi*h ) ;
}
inline double CubicSpline::Dprime(const double &r)
{
    double xxi, h;
    xxi = r -_r[getInterval(r)];
    h   = _r[getInterval(r)+1]-_r[getInterval(r)];

    return ( 0.5*xxi*xxi/h - (1.0/6.0)*h ) ;
}
//inline int CubicSpline::getInterval(double &r)
//{
//    if (r < _r[0] || r > _r[_r.size() - 1]) return -1;
//    return int( (r - _r[0]) / (_r[_r.size()-1] - _r[0]) * (_r.size() - 1) );
//}

inline int CubicSpline::getInterval(const double &r)
{
    if (r < _r[0]) return 0;
    if(r > _r[_r.size() - 2]) return _r.size()-2;
    size_t i;
    for(i=0; i<_r.size(); ++i)
        if(_r[i]>r) break;
    return i-1;
} 

inline double CubicSpline::A_prime_l(int i)
{
    return -1.0/(_r[i+1]-_r[i]);
}

inline double CubicSpline::B_prime_l(int i)
{
    return 1.0/(_r[i+1]-_r[i]);
}

inline double CubicSpline::C_prime_l(int i)
{
    return (1.0/6.0)*(_r[i+1]-_r[i]);
}

inline double CubicSpline::D_prime_l(int i)
{
    return (1.0/3.0)*(_r[i+1]-_r[i]);
}

inline double CubicSpline::A_prime_r(int i)
{
    return -1.0/(_r[i+2]-_r[i+1]);
}

inline double CubicSpline::B_prime_r(int i)
{
    return 1.0/(_r[i+2]-_r[i+1]);
}

inline double CubicSpline::C_prime_r(int i)
{
    return -(1.0/3.0)*(_r[i+2]-_r[i+1]);
}

inline double CubicSpline::D_prime_r(int i)
{
    return -(1.0/6.0)*(_r[i+2]-_r[i+1]);
}

}}

#endif	/* _CUBICSPLINE_H */

