#ifndef _SIGMA_RESAMPLED_CORRELATOR_H_
#define _SIGMA_RESAMPLED_CORRELATOR_H_

#include<config.h>
#include<utils/macros.h>

#include "data_containers.h"

CPSFIT_START_NAMESPACE

typedef correlationFunction<double,doubleJackknifeDistributionD> doubleJackCorrelationFunction;
typedef correlationFunction<double,jackknifeDistributionD> jackknifeCorrelationFunction;

//Assumed to be a resampled distribution type
template<typename DistributionType>
correlationFunction<double, DistributionType> computeSigmaVacSub(const sigmaSelfContractionBase<DistributionType> &bub){
  int Lt = bub.getLt();

  correlationFunction<double,DistributionType> out(Lt);

  for(int tsep=0;tsep<Lt; tsep++){
    out.coord(tsep) = tsep;
    for(int t0=0; t0<Lt; t0++){
      int t1 = (t0 + tsep) % Lt;
      DistributionType tmp = 0.5 * bub(t0)*bub(t1) / double(Lt);
      out.value(tsep) = t0 == 0 ? tmp : out.value(tsep) + tmp;
    }
  }
  return out;
}

template<typename resampledCorrelationFunctionType>
resampledCorrelationFunctionType computeSigmaVacSub(const sigmaSelfContraction &raw, const int bin_size){
  int Lt = raw.getLt();
  
  typedef typename resampledCorrelationFunctionType::DataType DistributionType;
  typedef sigmaSelfContractionSelect<DistributionType> sigmaSelfContractionType;
  sigmaSelfContractionType sigma_self_r(Lt);
  for(int t=0;t<Lt;t++)
    sigma_self_r(t).resample(raw(t).bin(bin_size));
  return computeSigmaVacSub(sigma_self_r);
}

CPSFIT_END_NAMESPACE

#endif
