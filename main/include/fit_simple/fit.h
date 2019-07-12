#ifndef _FIT_SIMPLE_FIT_H_
#define _FIT_SIMPLE_FIT_H_

#include "plot.h"

template<typename DataSeriesType>
inline DataSeriesType getDataInRange(const DataSeriesType &data, const int tmin, const int tmax){
  return DataSeriesType(tmax - tmin + 1, [&](const int i){ return data[tmin + i]; });
}

template<typename Out, typename In>
Out pconvert(const In &in){ Out out(in.size()); for(int i=0;i<in.size();i++) out(i) = in(i); return out; }

template<typename Out, typename In>
jackknifeDistribution<Out> pconvert(const jackknifeDistribution<In> &in){ 
  jackknifeDistribution<Out> out(in.size());
  for(int s=0;s<in.size();s++) out.sample(s) = pconvert<Out,In>(in.sample(s));
  return out;
}


typedef parameterVector<double> parameterVectorD;

template<typename ArgsType, typename CMDlineType>
struct FitFuncManagerBase{
  const ArgsType &args;
  const CMDlineType &cmdline;
  
  FitFuncManagerBase(const ArgsType &args, const CMDlineType &cmdline): args(args), cmdline(cmdline){}
  
  virtual genericFitFuncBase const* getFitFunc() const = 0;
  virtual parameterVectorD getGuess() const = 0;
  virtual void plot(const jackknifeCorrelationFunctionD &data_j, const jackknifeDistribution<parameterVectorD> &params) const = 0;

  template<typename FF>
  parameterVectorD getGuessBase(const FF &ff) const{
   typename FF::ParameterType guess = ff.guess();
    if(cmdline.load_guess)
      parse(guess,cmdline.guess_file);
    return pconvert<parameterVectorD, typename FF::ParameterType>(guess);
  }
  
  virtual ~FitFuncManagerBase(){}
};

template<typename HyperbolicFitFunc, typename ArgsType, typename CMDlineType>
struct FitFuncHyperbolicManager: public FitFuncManagerBase<ArgsType,CMDlineType>{
  typedef HyperbolicFitFunc FitFunc;
  simpleFitFuncWrapper<FitFunc> fitfunc;

  FitFuncHyperbolicManager(const ArgsType &args, const CMDlineType &cmdline): FitFuncManagerBase<ArgsType,CMDlineType>(args, cmdline), fitfunc(FitFunc(args.Lt)){}

  genericFitFuncBase const* getFitFunc() const{ return (genericFitFuncBase const*)&fitfunc; }

  parameterVectorD getGuess() const{ 
    return this->getGuessBase(fitfunc.fitfunc);
  }

  void plot(const jackknifeCorrelationFunctionD &data_j, const jackknifeDistribution<parameterVectorD> &params) const{
    plotEffectiveMass(this->args,fitfunc.fitfunc,data_j,pconvert<typename FitFunc::ParameterType,parameterVectorD>(params),1);
  }
};

template<typename ArgsType, typename CMDlineType>
struct FitExpManager: public FitFuncManagerBase<ArgsType,CMDlineType>{
  typedef FitExp FitFunc;
  simpleFitFuncWrapper<FitFunc> fitfunc;

  FitExpManager(const ArgsType &args, const CMDlineType &cmdline): FitFuncManagerBase<ArgsType,CMDlineType>(args, cmdline), fitfunc(FitFunc()){}

  genericFitFuncBase const* getFitFunc() const{ return (genericFitFuncBase const*)&fitfunc; }

  parameterVectorD getGuess() const{ 
    return this->getGuessBase(fitfunc.fitfunc);
  }

  void plot(const jackknifeCorrelationFunctionD &data_j, const jackknifeDistribution<parameterVectorD> &params) const{
    plotEffectiveMass(this->args,fitfunc.fitfunc,data_j,pconvert<typename FitFunc::ParameterType,parameterVectorD>(params),1);
  }
};

template<typename ArgsType, typename CMDlineType>
struct FitConstantManager: public FitFuncManagerBase<ArgsType,CMDlineType>{
  typedef FitConstant FitFunc;
  simpleFitFuncWrapper<FitFunc> fitfunc;

  FitConstantManager(const ArgsType &args, const CMDlineType &cmdline): FitFuncManagerBase<ArgsType,CMDlineType>(args, cmdline), fitfunc(FitFunc()){}

  genericFitFuncBase const* getFitFunc() const{ return (genericFitFuncBase const*)&fitfunc; }

  parameterVectorD getGuess() const{ 
    return this->getGuessBase(fitfunc.fitfunc);
  }

  void plot(const jackknifeCorrelationFunctionD &data_j, const jackknifeDistribution<parameterVectorD> &params) const{
    plotRaw(this->args,fitfunc.fitfunc,data_j,pconvert<typename FitFunc::ParameterType,parameterVectorD>(params));
  }
};

template<typename ArgsType, typename CMDlineType>
struct FitTwoStateCoshManager: public FitFuncManagerBase<ArgsType,CMDlineType>{
  typedef FitTwoStateCosh FitFunc;
  simpleFitFuncWrapper<FitFunc> fitfunc;

  FitTwoStateCoshManager(const ArgsType &args, const CMDlineType &cmdline): FitFuncManagerBase<ArgsType,CMDlineType>(args, cmdline), fitfunc(FitFunc(args.Lt)){}

  genericFitFuncBase const* getFitFunc() const{ return (genericFitFuncBase const*)&fitfunc; }

  parameterVectorD getGuess() const{ 
    return this->getGuessBase(fitfunc.fitfunc);
  }

  void plot(const jackknifeCorrelationFunctionD &data_j, const jackknifeDistribution<parameterVectorD> &params) const{
    FitCosh fcosh(this->args.Lt);
    plotTwoStateEffectiveMass<FitTwoStateCosh, FitCosh, ArgsType>(this->args, data_j, pconvert<typename FitFunc::ParameterType,parameterVectorD>(params), fcosh, fitfunc.fitfunc, 1); 
  }
};

template<typename ArgsType, typename CMDlineType>
std::unique_ptr< FitFuncManagerBase<ArgsType,CMDlineType> > getFitFuncManager(const ArgsType &args, const CMDlineType &cmdline){
  std::unique_ptr< FitFuncManagerBase<ArgsType,CMDlineType> > fitfunc_manager;

  switch(args.fitfunc){
  case FitFuncType::FCosh:
    fitfunc_manager.reset(new FitFuncHyperbolicManager<FitCosh,ArgsType,CMDlineType>(args,cmdline)); break;
  case FitFuncType::FSinh:
    fitfunc_manager.reset(new FitFuncHyperbolicManager<FitSinh,ArgsType,CMDlineType>(args,cmdline)); break;
  case FitFuncType::FExp:
    fitfunc_manager.reset(new FitExpManager<ArgsType,CMDlineType>(args,cmdline)); break;
  case FitFuncType::FConstant:
    fitfunc_manager.reset(new FitConstantManager<ArgsType,CMDlineType>(args,cmdline)); break;
  case FitFuncType::FTwoStateCosh:
    fitfunc_manager.reset(new FitTwoStateCoshManager<ArgsType,CMDlineType>(args,cmdline)); break;
  default:
    error_exit(std::cout << "fit: Invalid fitfunc " << args.fitfunc << std::endl);
  }
  
  return fitfunc_manager;
}


template<typename ArgsType, typename CMDlineType>
void fit(jackknifeDistribution<parameterVectorD> &params,
	 jackknifeDistributionD &chisq,
	 int &dof,
	 const jackknifeCorrelationFunctionD &data_j,
	 const doubleJackknifeCorrelationFunctionD &data_dj,
	 const blockDoubleJackknifeCorrelationFunctionD &data_bdj,
	 const ArgsType &args, const CMDlineType &cmdline){
  
  bool do_dj, do_bdj;
  getDJtypes(do_dj, do_bdj, args.covariance_strategy);

  //Get the data in the fit range
  jackknifeCorrelationFunctionD data_j_inrange = getDataInRange(data_j, args.t_min, args.t_max);
  doubleJackknifeCorrelationFunctionD data_dj_inrange;
  if(do_dj) data_dj_inrange = getDataInRange(data_dj, args.t_min, args.t_max);
  blockDoubleJackknifeCorrelationFunctionD data_bdj_inrange;
  if(do_bdj) data_bdj_inrange = getDataInRange(data_bdj, args.t_min, args.t_max);

  std::cout << "All data\n";
  for(int i=0;i<data_j.size();i++){
    std::cout << (int)data_j.coord(i) << " " << data_j.value(i) << std::endl;
  }

  std::cout << "Data in range\n";
  for(int i=0;i<data_j_inrange.size();i++){
    std::cout << (int)data_j_inrange.coord(i) << " " << data_j_inrange.value(i) << std::endl;
  }


  //Get the fit function manager
  std::unique_ptr< FitFuncManagerBase<ArgsType,CMDlineType> > fitfunc_manager = getFitFuncManager(args, cmdline);

  //Set up the minimizer
  MarquardtLevenbergParameters<double> minparams;
  if(cmdline.load_mlparams){
    parse(minparams, cmdline.mlparams_file);
    std::cout << "Loaded minimizer params: " << minparams << std::endl;
  }
  
  simpleFitWrapper fitter(*fitfunc_manager->getFitFunc(), MinimizerType::MarquardtLevenberg, minparams);

  //Generate the covariance matrix
  switch(args.covariance_strategy){
  case CovarianceStrategy::CorrelatedBlockHybrid:
    fitter.generateCovarianceMatrix(data_dj_inrange, data_bdj_inrange, CostType::Correlated);
    break;
  case CovarianceStrategy::FrozenCorrelated:
    fitter.generateCovarianceMatrix(data_j_inrange, CostType::Correlated);
    break;
  case CovarianceStrategy::Correlated:
    fitter.generateCovarianceMatrix(data_dj_inrange, CostType::Correlated);
    break;
  case CovarianceStrategy::CorrelatedBlock:
    fitter.generateCovarianceMatrix(data_bdj_inrange, CostType::Correlated);
    break;
  case CovarianceStrategy::Uncorrelated:
    fitter.generateCovarianceMatrix(data_dj_inrange, CostType::Uncorrelated);
    break;
  default:
    assert(0);
  }

  //Do the fit
  parameterVectorD guess = fitfunc_manager->getGuess();

  const int nsample = data_j.value(0).size();
  params = jackknifeDistribution<parameterVectorD>(nsample, guess);
  chisq = jackknifeDistributionD(nsample);
    
  jackknifeDistributionD chisq_per_dof(nsample);
  
  fitter.fit(params, chisq, chisq_per_dof, dof, data_j_inrange);

  jackknifeDistributionD pvalue_chisq(nsample, [&](const int s){ return chiSquareDistribution::pvalue(dof, chisq.sample(s)); });
  jackknifeDistributionD pvalue_Tsq(nsample, [&](const int s){ return TsquareDistribution::pvalue(chisq.sample(s), dof, nsample-1); });

  std::cout << "Params: " << params << std::endl;
  std::cout << "Chisq: " << chisq << std::endl;
  std::cout << "Chisq/dof: " << chisq_per_dof << std::endl;
  std::cout << "Dof: " << dof << std::endl;
  std::cout << "P-value(chi^2): " << pvalue_chisq << std::endl;
  std::cout << "P-value(T^2): " << pvalue_Tsq << std::endl;
  
#ifdef HAVE_HDF5
  writeParamsStandard(chisq, "chisq.hdf5");
  writeParamsStandard(chisq_per_dof, "chisq_per_dof.hdf5");
  writeParamsStandard(params, "params.hdf5"); 
  writeParamsStandard(pvalue_chisq, "pvalue_chisq.hdf5");
  writeParamsStandard(pvalue_Tsq, "pvalue_Tsq.hdf5");
#endif

  fitfunc_manager->plot(data_j, params);
}



template<typename ArgsType, typename CMDlineType>
void fitCentral(parameterVectorD &params,
		double &chisq,
		int &dof,
		const jackknifeCorrelationFunctionD &data_j,
		const jackknifeCorrelationFunctionD &data_j_unbinned,
		const ArgsType &args, const CMDlineType &cmdline, MarquardtLevenbergParameters<double> const *minparams_in = NULL){

  //For block and block-hybrid the unbinned jackknife is used, for the former to compute the covariance matrix 
  //and for the latter the correlation matrix (with sigma computed from the binned jackknife)
  bool do_j_b, do_j_ub;
  getJtypes(do_j_b, do_j_ub, args.covariance_strategy);

  //Get the data in the fit range
  jackknifeCorrelationFunctionD data_j_inrange, data_j_ub_inrange;
  if(do_j_b) data_j_inrange = getDataInRange(data_j, args.t_min, args.t_max);
  if(do_j_ub) data_j_ub_inrange = getDataInRange(data_j_unbinned, args.t_min, args.t_max);

  //Get the central values
  const jackknifeCorrelationFunctionD & data_j_inrange_use = do_j_b ?  data_j_inrange : data_j_ub_inrange;

  jackknifeCorrelationFunctionD data_cen_inrange(data_j_inrange_use.size());  
  for(int i=0;i<data_j_inrange_use.size();i++){
    data_cen_inrange.coord(i) = data_j_inrange_use.coord(i);
    data_cen_inrange.value(i) = jackknifeDistributionD(1, data_j_inrange_use.value(i).mean());
  }

  //Get the fit function manager
  std::unique_ptr< FitFuncManagerBase<ArgsType,CMDlineType> > fitfunc_manager = getFitFuncManager(args, cmdline);

  //Set up the minimizer
  MarquardtLevenbergParameters<double> minparams;
  if(minparams_in != NULL) minparams = *minparams_in;
  
  simpleFitWrapper fitter(*fitfunc_manager->getFitFunc(), MinimizerType::MarquardtLevenberg, minparams);

  //Generate the covariance matrix
  std::vector<jackknifeDistribution<double> > tmp_sigma;
  NumericSquareMatrix<jackknifeDistribution<double> > tmp_corrmat;

  switch(args.covariance_strategy){
  case CovarianceStrategy::CorrelatedBlockHybrid:
    fitter.generateCovarianceMatrix(data_j_inrange, CostType::Correlated);
    tmp_sigma = fitter.getSigma();
    
    fitter.generateCovarianceMatrix(data_j_ub_inrange, CostType::Correlated);
    tmp_corrmat = fitter.getCorrelationMatrix();

    fitter.importCorrelationMatrix(tmp_corrmat, tmp_sigma);
    break;
  case CovarianceStrategy::FrozenCorrelated:
    //Freezing the covariance matrix doesn't make any difference if only one sample!
    fitter.generateCovarianceMatrix(data_j_inrange, CostType::Correlated);
    break;
  case CovarianceStrategy::Correlated:
    fitter.generateCovarianceMatrix(data_j_inrange, CostType::Correlated);
    break;
  case CovarianceStrategy::CorrelatedBlock:
    fitter.generateCovarianceMatrix(data_j_ub_inrange, CostType::Correlated); //unbinned data
    break;
  case CovarianceStrategy::Uncorrelated:
    fitter.generateCovarianceMatrix(data_j_inrange, CostType::Uncorrelated);
    break;
  default:
    assert(0);
  }

  //Do the fit
  jackknifeDistribution<parameterVectorD> tmp_params(1, params);
  jackknifeDistributionD tmp_chisq(1), tmp_chisq_per_dof(1);

  fitter.fit(tmp_params, tmp_chisq, tmp_chisq_per_dof, dof, data_cen_inrange);

  params = tmp_params.sample(0);
  chisq = tmp_chisq.sample(0);
  
  std::cout << "Params: " << params << std::endl;
  std::cout << "Chisq: " << chisq << std::endl;
  std::cout << "Dof: " << dof << std::endl;
}


  
#endif
