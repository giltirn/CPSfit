#ifndef _ALPHA_S_COMPUTE_H
#define _ALPHA_S_COMPUTE_H

#include<cassert>
#include<gsl/gsl_errno.h>
#include<gsl/gsl_min.h>

#include<config.h>
#include<utils/macros.h>

CPSFIT_START_NAMESPACE

//Use the two-loop formula of Rev.Mod.Phys. 68 (1996) 1125-1144 to compute alpha_s
//All energies in *GeV*
class ComputeAlphaS{
  //Inputs
  double as_mz;
  double mz;
  double mb;
  double mc;
  int Nc;
  
  //Computed
  double Lambda[3];
  
  inline static double beta0(const int Nf, const int Nc = 3){
    return (11.*Nc - 2*Nf)/3.;
  }
  inline static double CF(const int Nc = 3){
    return double(Nc*Nc - 1)/2/Nc;
  }
  inline static double beta1(const int Nf, const int Nc = 3){
    return 34./3 * Nc*Nc - 10./3 * Nf*Nc - 2*CF(Nc)*Nf;
  }
  inline static double alpha_s(const double mu, const double Lambda, const int Nf, const int Nc=3){
    double b0 = beta0(Nf,Nc);
    double b1 = beta1(Nf,Nc);
    double log_mu2_over_L2 = log(mu*mu/Lambda/Lambda);
    
    double pref = 4.*M_PI/b0/log_mu2_over_L2;
    double brack = 1 - b1/b0/b0 * log(log_mu2_over_L2)/log_mu2_over_L2;
    //printf("log(mu^2/L^2)=%.8f, beta0=%.8f beta1=%.8f 0,1 loop part %.8f, 2-loop multiplier %.8f\n",log_mu2_over_L2,b0,b1,pref,brack);
    return pref*brack;
  }
  static double computeLambda(const int Nf, const double match_at, const double match_to, const int Nc=3, const bool vrb = true);

  friend struct as_accessor;

  bool vrb; //verbose printing
public:
  inline double alpha_s(double mu, const int Nf, const int Nc=3){ //mu should be in GeV
    assert(Nf >= 3 && Nf <=5);
    return alpha_s(mu,Lambda[Nf-1],Nf,Nc);
  }
  
  ComputeAlphaS(const bool _vrbose = true): vrb(_vrbose){
    as_mz = 0.1185; //[epsilon' PRL]
    mz = 91.1876; //[K.A. Olive et al. (Particle Data Group), Chin. Phys. C, 38 , 090001 (2014) and 2015 update  (PDG)]
    mb = 4.18; //[epsilon' PRL]
    mc = 1.275;//[epsilon' PRL]
    int Nc = 3;

    static bool initted = false;
    static double Lambda_init[3];

    if(!initted){
      if(vrb) printf("Initializing alpha_s computation\n");
      
      //Solve for Lambda_5
      Lambda[5 -1] = computeLambda(5, mz, as_mz, Nc, vrb);
      if(vrb) printf("Computed Lambda_5 = %.7f\n", Lambda[5 -1]);
      
      //Get a_s at mb
      double as_mb = alpha_s(mb, 5, Nc);
      
      Lambda[4 -1] = computeLambda(4, mb, as_mb, Nc, vrb);
      if(vrb) printf("Computed Lambda_4 = %.7f\n", Lambda[4 -1]);
      
      //Get a_s at mc
      double as_mc = alpha_s(mc, 4, Nc);
      
      Lambda[3 -1] = computeLambda(3, mc, as_mc, Nc, vrb);
      if(vrb) printf("Computed Lambda_3 = %.7f\n", Lambda[3 -1]);

      for(int i=0;i<3;i++) Lambda_init[i] = Lambda[i];
      initted = true;      
    }else{
      for(int i=0;i<3;i++) Lambda[i] = Lambda_init[i];
    }
    
  }

  
};





struct as_params{
  double mu;
  double match_to;
  int Nf;
  int Nc;
};

struct as_accessor{
  static double alpha_s(const as_params& p, const double Lambda){
    return ComputeAlphaS::alpha_s(p.mu, Lambda, p.Nf, p.Nc);
  }
};

inline double as_f (double Lambda, void * p) {
  const as_params& params = *((as_params*)p);
  double as = as_accessor::alpha_s(params,Lambda);
  double diff = as - params.match_to;
  double diff2 = diff*diff; //to minimize

  //printf("alpha_s(Lambda=%f)=%f, target %f,  diff2 = %f, target 0\n",Lambda,as,params.match_to,diff2);
  return diff2;
}


double ComputeAlphaS::computeLambda(const int Nf, const double match_at, const double match_to, const int Nc, bool vrb){
  if(vrb) printf("Starting computeLambda with Nf=%d, match_at=%f match_to=%f, Nc=%d\n",Nf,match_at,match_to,Nc);
  as_params p;
  p.mu = match_at;
  p.match_to = match_to;
  p.Nf = Nf;
  p.Nc = Nc;

  gsl_function F;
  F.function = &as_f;
  F.params = &p;

  //Note, due to log(log(mu^2/Lambda^2)),   Lambda must always be smaller than mu or else we get a log of a negative number
  double start = 1e-6;
  double guess = 1e-5;
  double end = 0.99999*match_at;

  const gsl_min_fminimizer_type * T = gsl_min_fminimizer_brent;
  gsl_min_fminimizer * s = gsl_min_fminimizer_alloc (T);
  gsl_min_fminimizer_set (s, &F, guess, start, end);

  int status;
  int iter = 0, max_iter = 100;
  double m,a,b;
  do{
    iter++;
    status = gsl_min_fminimizer_iterate (s);

    m = gsl_min_fminimizer_x_minimum (s);
    a = gsl_min_fminimizer_x_lower (s);
    b = gsl_min_fminimizer_x_upper (s);

    status = gsl_min_test_interval (a, b, 1e-7, 0.0);

    if(vrb && status == GSL_SUCCESS){
      printf ("Converged:\n");

      printf("%5d [%.7f, %.7f] "
	     "%.7f %.7f\n",
	     iter, a, b,
	     m, b - a);
    }
      
  }
  while (status == GSL_CONTINUE && iter < max_iter);  
  gsl_min_fminimizer_free(s);

  if(status != GSL_SUCCESS){
    printf("Converge failed\n");
    exit(-1);
  }

  return m;
}

CPSFIT_END_NAMESPACE

#endif