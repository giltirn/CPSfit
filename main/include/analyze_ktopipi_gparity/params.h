#ifndef _ANALYZE_KTOPIPI_PARAMS_H_
#define _ANALYZE_KTOPIPI_PARAMS_H_

#include<parser.h>

GENERATE_ENUM_AND_PARSER(PhaseShiftDerivativeSource, (DerivSchenk)(DerivLinearEpipi)(DerivLinearQpipi) );
GENERATE_ENUM_AND_PARSER(RIscheme, (QslashQslash)(GammaGamma) );

#define FILE_IDX_PAIR_MEMBERS (std::string, file)(int, idx)
struct FileIdxPair{
  GENERATE_MEMBERS(FILE_IDX_PAIR_MEMBERS);
  FileIdxPair(){}
  FileIdxPair(const std::string &f, const int i): file(f),idx(i){}
};
GENERATE_PARSER(FileIdxPair, FILE_IDX_PAIR_MEMBERS);

#define FIT_RESULTS_MEMBERS (FileIdxPair, M_lat)(FileIdxPair, mK)(FileIdxPair, Epi)(FileIdxPair, Epipi)
struct FitResults{ //these are all expected to be hdf5 files currently
  GENERATE_MEMBERS(FIT_RESULTS_MEMBERS);
  FitResults(): M_lat("32c_216cfgs_results/ktopipi.hdf5",4), mK("32c_216cfgs_results/mk.hdf5", 1), Epi("32c_216cfgs_results/Epi.hdf5", 1), Epipi("32c_216cfgs_results/pipi.hdf5", 1){}
};
GENERATE_PARSER(FitResults, FIT_RESULTS_MEMBERS);

#define OTHER_INPUTS_MEMBERS (FileIdxPair, ainv)(FileIdxPair, omega_expt)(FileIdxPair, mod_eps)(FileIdxPair, ReA0_expt)(FileIdxPair, ReA2_expt)(FileIdxPair, ReA2_lat)(FileIdxPair, ImA2_lat)(FileIdxPair, delta_2_lat)
struct OtherInputs{ //these are all expected to be xml-format superjackknife files currently
  GENERATE_MEMBERS(OTHER_INPUTS_MEMBERS);
  OtherInputs(): ainv("32c_216cfgs_results/ainv_gaussian.bootxml",0),
		 omega_expt("32c_216cfgs_results/fit_inputs/omega.bootxml",0),
		 mod_eps("32c_216cfgs_results/fit_inputs/mod_eps.bootxml",0),
		 ReA0_expt("32c_216cfgs_results/fit_inputs/reA0_expt.bootxml",0),
		 ReA2_expt("32c_216cfgs_results/fit_inputs/reA2_expt.bootxml",0),
		 ReA2_lat("32c_216cfgs_results/fit_inputs/reA2_lat_gaussian.bootxml",0), ImA2_lat("32c_216cfgs_results/fit_inputs/imA2_lat.bootxml",0),
		 delta_2_lat("32c_216cfgs_results/fit_inputs/delta2_gaussian.bootxml",0){}
};
GENERATE_PARSER(OtherInputs,OTHER_INPUTS_MEMBERS);

#define RENORMALIZATION_MEMBERS (RIscheme, scheme)(double, mu)(std::string, file)
struct Renormalization{
  GENERATE_MEMBERS(RENORMALIZATION_MEMBERS);
  Renormalization(): scheme(QslashQslash), mu(1.531), file("32c_216cfgs_results/qslash_1.53GeV_noG1.xml"){}
};
GENERATE_PARSER(Renormalization, RENORMALIZATION_MEMBERS);

#define CONSTANT_MEMBERS (double, G_F)(double, Vud)(double, Vus)(double, phi_epsilon)
struct Constants{ //Currently treated as constants but some could be upgraded to jackknifes to add in errors
  GENERATE_MEMBERS(CONSTANT_MEMBERS);
  Constants(): G_F(1.1663787e-05), Vud(0.97425), Vus(0.2253), phi_epsilon(0.75956729046793223188){}
};
GENERATE_PARSER(Constants, CONSTANT_MEMBERS);



#define ARGS_MEMBERS \
  (FitResults, fit_results)			\
  (OtherInputs, other_inputs)			\
  (Renormalization, renormalization)		\
  (Constants, constants)			\
  (std::string, wilson_coeffs_file)		\
  (std::vector<int>, twists)			\
  (int, L)					\
  (PhaseShiftDerivativeSource, deriv_source)	\
  (bool, lattice_dispersion_reln)

struct Args{
  GENERATE_MEMBERS(ARGS_MEMBERS);
  Args(): wilson_coeffs_file("32c_216cfgs_results/WilsonCoeffs.dat"), //cf WilsonCoeffs.h for format
	  twists({1,1,1}),
	  L(32),
	  deriv_source(DerivLinearQpipi),
	  lattice_dispersion_reln(false){}
};
GENERATE_PARSER(Args, ARGS_MEMBERS);


#endif