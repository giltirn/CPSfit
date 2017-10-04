#ifndef _FIT_PIPI_GPARITY_PLOT_H_
#define _FIT_PIPI_GPARITY_PLOT_H_

#include<plot.h>
#include<effective_mass.h>

//two-point effective energy assuming cosh form with optional constant subtraction
jackknifeCorrelationFunction twoPointEffectiveEnergy(const jackknifeCorrelationFunction &data_j,
						     const jackknifeDistributionD &fitted_Epipi,
						     const jackknifeDistributionD &fitted_constant,
						     const bool subtract_constant,
						     const Args &args, const CMDline &cmdline){
  jackknifeCorrelationFunction data_j_mod(data_j);
  if(subtract_constant){
    for(int i=0;i<data_j_mod.size();i++) data_j_mod.value(i) = data_j_mod.value(i) - args.Cscale*fitted_constant;
  }
  FitCoshPlusConstant fitfunc(args.Lt, args.tsep_pipi, args.Ascale, args.Cscale);
  FitCoshPlusConstant::Params base(1,1,0); //amplitude irrelevant, E will be varied, set constant to 0
  std::cout << "Computing two-point effective energy" << std::endl;
  return effectiveMass2pt<jackknifeCorrelationFunction,FitCoshPlusConstant>(data_j,fitfunc,base,1,args.Lt);
}


template<typename FitFunc>
class Fit3ptPiPiEffectiveMass{
public:
  typedef typename FitFunc::ParameterType BaseParameterType;
  typedef typename FitFunc::ValueDerivativeType BaseDerivativeType;
  typedef double ValueType;
  typedef MLwrapper<double> ParameterType;
  typedef MLwrapper<double> ValueDerivativeType;
  typedef double GeneralizedCoordinate;
private:  
  FitFunc const* fitfunc;
  int params_mass_index;
  BaseParameterType base;
public:

  Fit3ptPiPiEffectiveMass(const FitFunc &ff, const BaseParameterType _base, const int pmi): fitfunc(&ff),  params_mass_index(pmi), base(_base){}

  //[C(t+1)-C(t)]/[C(t+2)-C(t+1)]
  
  inline double value(const double t, const ParameterType &params) const{    
    BaseParameterType p(base); p(params_mass_index) = *params;
    double value_t = fitfunc->value(t,p);
    double value_tp1 = fitfunc->value(t+1,p);
    double value_tp2 = fitfunc->value(t+2,p);

    return (value_tp1 - value_t)/(value_tp2 - value_tp1);
  }
  
  ValueDerivativeType parameterDerivatives(const double t, const ParameterType &params) const{
    ValueDerivativeType yderivs;
    BaseParameterType p(base); p(params_mass_index) = *params;
    
    double value_t = fitfunc->value(t,p);
    double deriv_t = fitfunc->parameterDerivatives(t,p)(params_mass_index);
    
    double value_tp1 = fitfunc->value(t+1,p);
    double deriv_tp1 = fitfunc->parameterDerivatives(t+1,p)(params_mass_index);
    
    double value_tp2 = fitfunc->value(t+2,p);
    double deriv_tp2 = fitfunc->parameterDerivatives(t+2,p)(params_mass_index);
    
    double value_num = value_tp1 - value_t;
    double value_den = value_tp2 - value_tp1;

    double deriv_num = deriv_tp1 - deriv_t;
    double deriv_den = deriv_tp2 - deriv_tp1;
    
    *yderivs = deriv_num/value_den - value_num/value_den/value_den*deriv_den;
    return yderivs;
  }

  inline int Nparams() const{ return 1; }
};

jackknifeCorrelationFunction threePointEffectiveEnergy(const jackknifeCorrelationFunction &data_j,
						       const jackknifeDistributionD &fitted_Epipi,
						       const Args &args, const CMDline &cmdline){
  const int Lt = args.Lt;
  const int nsample = data_j.value(0).size();
  assert(data_j.size() == Lt);
  jackknifeCorrelationFunction ratios(Lt-2);
  for(int i=0;i<Lt-2;i++){
    double t = data_j.coord(i);
    assert(t == double(i));
    assert(data_j.coord(i+1) == t+1);
    assert(data_j.coord(i+2) == t+2);

    ratios.coord(i) = t;
    ratios.value(i) = (data_j.value(i+1) - data_j.value(i))/(data_j.value(i+2) - data_j.value(i+1));
  }
  typedef Fit3ptPiPiEffectiveMass<FitCoshPlusConstant> FitEffMass;

  FitCoshPlusConstant fitfunc(args.Lt, args.tsep_pipi, args.Ascale, args.Cscale);
  FitCoshPlusConstant::Params base(1,1,0); //amplitude and constant irrelevant, E will be varied
  std::cout << "Computing three-point effective energy" << std::endl;
  FitEffMass fiteffmass(fitfunc, base, 1);
  return fitEffectiveMass<jackknifeCorrelationFunction,FitEffMass>(ratios,fiteffmass);
}

  
jackknifeCorrelationFunction effectiveEnergy(const jackknifeCorrelationFunction &data_j,
					     const jackknifeDistributionD &fitted_Epipi,
					     const jackknifeDistributionD &fitted_constant,
					     const Args &args, const CMDline &cmdline){
  switch(args.effective_energy){
  case TwoPoint:
    return twoPointEffectiveEnergy(data_j,fitted_Epipi,fitted_constant,false,args,cmdline);
  case TwoPointSubConstant:
    return twoPointEffectiveEnergy(data_j,fitted_Epipi,fitted_constant,true,args,cmdline);
  case ThreePoint:
    return threePointEffectiveEnergy(data_j,fitted_Epipi,args,cmdline);
  default:
    error_exit(std::cout << "Unknown effective energy type "<< args.effective_energy <<std::endl);
  }
}

void plot(const jackknifeCorrelationFunction &data_j,
	  const jackknifeDistributionD &fitted_Epipi,
	  const jackknifeDistributionD &fitted_constant,
	  const Args &args, const CMDline &cmdline){
  jackknifeCorrelationFunction E_eff = effectiveEnergy(data_j,fitted_Epipi,fitted_constant,args,cmdline);

  std::cout << "Effective energy:\n" << E_eff << std::endl;

  MatPlotLibScriptGenerate plotter;
  typedef MatPlotLibScriptGenerate::handleType handleType;
  MatPlotLibScriptGenerate::kwargsType plot_args;
  plot_args["color"] = "r";

  //Plot the effective energy
  typedef DataSeriesAccessor<jackknifeCorrelationFunction,ScalarCoordinateAccessor<double>,DistributionPlotAccessor<jackknifeDistributionD> > accessor;
  accessor effenergy_accessor(E_eff);

  handleType plotdata = plotter.plotData(effenergy_accessor,plot_args);
  
  //Plot the fitted energy across the fit range
  struct fitBand{
    double u;
    double l;
    int tmin;
    int tmax;    
    fitBand(double uu, double ll, int ttmin, int ttmax): u(uu),l(ll),tmin(ttmin),tmax(ttmax){}
    inline int size() const{ return tmax-tmin+1; }    
    inline double x(const int i) const{ return tmin+i; }
    inline double upper(const int i) const{ return u; }
    inline double lower(const int i) const{ return l; }
  };

  double se = fitted_Epipi.standardError();
  fitBand band(fitted_Epipi.best() + se, fitted_Epipi.best() - se, args.t_min, args.t_max);
  plot_args["alpha"] = 0.2;
  handleType fband = plotter.errorBand(band,plot_args);

  plotter.setXaxisBounds(-0.1,10.9);
  plotter.setYaxisBounds(fitted_Epipi.best()-10*se, fitted_Epipi.best()+10*se);
  plotter.setXlabel("$t$");
  plotter.setYlabel("$E_{\\pi\\pi}^{\\rm eff}$");
  
  plotter.write("effective_energy.py","effective_energy.pdf");
}


#endif