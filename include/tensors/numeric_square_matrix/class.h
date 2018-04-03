#ifndef _CPSFIT_NUMERIC_SQUARE_MATRIX_CLASS_H_
#define _CPSFIT_NUMERIC_SQUARE_MATRIX_CLASS_H_

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>

#include<config.h>
#include<utils/macros.h>
#include<serialize/hdf5_serialize.h>
#include<ET/generic_ET.h>

CPSFIT_START_NAMESPACE

template<typename Numeric>
class NumericSquareMatrix{ //square matrix
  std::vector<std::vector<Numeric> > m;

  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version){
    ar & m;
  }
public:
  NumericSquareMatrix():m(){}
  explicit NumericSquareMatrix(const int n): m(n, std::vector<Numeric>(n)){}
  NumericSquareMatrix(const int n, const Numeric &init): m(n, std::vector<Numeric>(n,init)){}
  NumericSquareMatrix(const NumericSquareMatrix &r) = default;
  NumericSquareMatrix(NumericSquareMatrix &&r) = default;

  template<typename Initializer> //Initializer is a lambda-type with operator()(const int, const int)
  inline NumericSquareMatrix(const int n, const Initializer &initializer): m(n, std::vector<Numeric>(n)){
    for(int i=0;i<n;i++)
      for(int j=0;j<n;j++)
	m[i][j] = initializer(i,j);
  }
  
  NumericSquareMatrix & operator=(const NumericSquareMatrix &r) = default;
  NumericSquareMatrix & operator=(NumericSquareMatrix &&r) = default;
  
  typedef NumericSquareMatrix<Numeric> ET_tag;
  template<typename U, typename std::enable_if<std::is_same<typename U::ET_tag, ET_tag>::value && !std::is_same<U,NumericSquareMatrix<Numeric> >::value, int>::type = 0>
  NumericSquareMatrix(U&& expr): NumericSquareMatrix(expr.common_properties()){
#pragma omp parallel for
    for(int i=0;i<this->size()*this->size();i++)
      getElem<NumericSquareMatrix<Numeric> >::elem(*this, i) = expr[i];
  }
  
  inline int size() const{ return m.size(); }
  
  void resize(const int n){
    if(m.size() == n) return;
    m.resize(n);
    for(int i=0;i<n;i++)
      m[i].resize(n);      
  }
  void resize(const int n, const Numeric &init){
    if(m.size() == n) return;
    m.resize(n);
    for(int i=0;i<n;i++)
      m[i].resize(n,init);     
  }
  template<typename Initializer>
  inline void resize(const int n, const Initializer &initializer){
    m.resize(n);
    for(int i=0;i<n;i++){
      m[i].resize(n);
      for(int j=0;j<n;j++) m[i][j] = initializer(i,j);
    }
  }
  
  void zero(){
    const int n = m.size();
    for(int i=0;i<n;i++)
      for(int j=0;j<n;j++)
	m[i][j] = 0.;
  }

  std::string print() const{
    std::ostringstream os;
    for(int i=0;i<m.size();i++){
      os << m[i][0];
      for(int j=1;j<m[i].size();j++)
  	os << " " << m[i][j];
      os << std::endl;
    }
    return os.str();
  }
  
  Numeric & operator()(const int i, const int j){ return m[i][j]; }
  const Numeric & operator()(const int i, const int j) const { return m[i][j]; }

  GENERATE_HDF5_SERIALIZE_METHOD((m));
};


CPSFIT_END_NAMESPACE
#endif