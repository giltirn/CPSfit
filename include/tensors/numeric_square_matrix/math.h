#ifndef _CPSFIT_NUMERIC_SQUARE_MATRIX_MATH_H_
#define _CPSFIT_NUMERIC_SQUARE_MATRIX_MATH_H_

#include<tensors/numeric_square_matrix/class.h>

CPSFIT_START_NAMESPACE

//We use the notation in http://www.math.hawaii.edu/~jb/math411/nation1  pg 41

//||m||_E  modulus operator for matrices
//Requires a definition of modulus-square |v|^2 of type T  as a lambda or class with T operator()(const T&)
template<typename T, typename mod2func>
T modE(const NumericSquareMatrix<T> &m, const mod2func &md2){
  assert(m.size() > 0);
  T out(m(0,0));
  zeroit(out);
  for(int i=0;i<m.size();i++)
    for(int j=0;j<m.size();j++)
      out = out + md2(m(i,j));
  return sqrt(out);
}

template<typename T, ENABLE_IF_FLOATINGPT(T)>
inline T modE(const NumericSquareMatrix<T> &m){
  return modE(m, [&](const T &v){ return v*v; });
}
template<typename T, ENABLE_IF_STDCOMPLEX(T)>
inline T modE(const NumericSquareMatrix<T> &m){
  return modE(m, [&](const T &v){ return std::conj(v)*v; });
}
template<typename T, 
	 typename std::enable_if< hasSampleMethod<T>::value && std::is_floating_point<typename getSampleType<T>::type>::value,
				  int >::type = 0
	 >
inline T modE(const NumericSquareMatrix<T> &m){
  return modE(m, [&](const T &v){ return T(v.size(), [&](const int s){ return v.sample(s)*v.sample(s); }); });
}


//||m||_1  maximum absolute column sum norm. 
//Requires a definition of >=  as a lambda or class with bool operator()(const T&, const T&)
//Requires a definition of absolute value |v| as a lambda or class with  T operator()(const T&)
//||m||_1  =  max_j( \sum_i |m_ij| )
template<typename T, typename geqFunc, typename absFunc>
T mod1(const NumericSquareMatrix<T> &m, const geqFunc &geq, const absFunc &abs){
  assert(m.size() > 0);
  T out(m(0,0));
  zeroit(out);

  for(int col=0;col<m.size();col++){
    T colsum(out);
    zeroit(colsum);

    for(int row=0;row<m.size();row++)
      colsum = colsum + abs(m(row,col));

    if(geq(colsum, out)) out = colsum;
  }
  return out;
}

//||m||_infty  maximum absolute row sum norm. 
//Requires a definition of >=  as a lambda or class with bool operator()(const T&, const T&)
//Requires a definition of absolute value |v| as a lambda or class with  T operator()(const T&)
//||m||_infty  =  max_i( \sum_j |m_ij| )
template<typename T, typename geqFunc, typename absFunc>
T modinfty(const NumericSquareMatrix<T> &m, const geqFunc &geq, const absFunc &abs){
  assert(m.size() > 0);
  T out(m(0,0));
  zeroit(out);

  for(int row=0;row<m.size();row++){
    T rowsum(out);
    zeroit(rowsum);

    for(int col=0;col<m.size();col++)
      rowsum = rowsum + abs(m(row,col));

    if(geq(rowsum, out)) out = rowsum;
  }
  return out;
}







CPSFIT_END_NAMESPACE
#endif
