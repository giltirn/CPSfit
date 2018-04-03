#ifndef _CPSFIT_BASE_TYPEDEFS_H_
#define _CPSFIT_BASE_TYPEDEFS_H_

//There are a number of basic types and compound types needed for the framework. These are controlled by a baseFitTypedefs, which takes a policy some user-defined input types

#include<config.h>
#include<utils/macros.h>
#include<tensors/numeric_vector.h>
#include<tensors/numeric_square_matrix.h>
#include<distribution/jackknife.h>
#include<data_series/correlationfunction.h>
#include<data_series/sample_series.h>

CPSFIT_START_NAMESPACE

#define INHERIT_TYPEDEF(FROM,DEF) typedef typename FROM::DEF DEF

template<typename BaseTypes>
struct baseFitTypedefs{
  //BaseTypes must typedef the following:
#define INHERIT_INPUT_FIT_TYPEDEFS(FROM)			       \
  INHERIT_TYPEDEF(FROM,DistributionType); \
  INHERIT_TYPEDEF(FROM,CorrelationFunctionDistribution)

  INHERIT_INPUT_FIT_TYPEDEFS(BaseTypes);
  
  typedef NumericSquareMatrix<DistributionType> MatrixDistribution;
  typedef NumericVector<DistributionType> VectorDistribution;

  typedef sampleSeries<const CorrelationFunctionDistribution> sampleSeriesType;
  typedef NumericSquareMatrixSampleView<const MatrixDistribution> sampleInvCorrType;
};

#define INHERIT_BASE_FIT_TYPEDEFS(FROM)			       \
  INHERIT_INPUT_FIT_TYPEDEFS(FROM);			       \
  INHERIT_TYPEDEF(FROM,MatrixDistribution); \
  INHERIT_TYPEDEF(FROM,VectorDistribution); \
  INHERIT_TYPEDEF(FROM,sampleSeriesType); \
  INHERIT_TYPEDEF(FROM,sampleInvCorrType)



//This is the standard implementation of BaseTypes for some generate coordinate type, with the data contained in a correlationFunction object comprising jackknife distributions
template<typename GeneralizedCoordinate>
struct standardInputFitTypes{
  typedef jackknifeDistribution<double> DistributionType;
  typedef correlationFunction<GeneralizedCoordinate,DistributionType> CorrelationFunctionDistribution;
};


CPSFIT_END_NAMESPACE
#endif