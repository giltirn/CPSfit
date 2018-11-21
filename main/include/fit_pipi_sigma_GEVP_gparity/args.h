#ifndef _PIPI_SIGMA_GEVP_FIT_ARGS_H_
#define _PIPI_SIGMA_GEVP_FIT_ARGS_H_

#define PIPI_SIGMA_GEVP_ARGS_MEMBERS \
  ( std::string, data_dir )			\
  ( std::string, pipi2pt_figure_file_fmt )	\
  ( std::string, sigma2pt_file_fmt )		\
  ( std::string, pipitosigma_file_fmt )		\
  ( std::string, pipi_bubble_file_fmt )		\
  ( std::string, sigma_bubble_file_fmt )		\
  ( int, Lt) \
  ( int, tsep_pipi ) \
  ( int, tstep_pipi2pt )	  \
  ( int, tstep_pipitosigma )	  \
  ( bool, do_vacuum_subtraction ) \
  ( int, t_max) \
  ( int, bin_size) \
  ( int, traj_start ) \
  ( int, traj_inc ) \
  ( int, traj_lessthan ) \
  ( double, Ascale)


struct PiPiSigmaGEVPargs{
  GENERATE_MEMBERS( PIPI_SIGMA_GEVP_ARGS_MEMBERS)

  PiPiSigmaGEVPargs(): data_dir("data"), Lt(64), t_max(15), traj_start(0), traj_inc(1), traj_lessthan(2), Ascale(1e13),
    do_vacuum_subtraction(true), bin_size(1), tsep_pipi(4),tstep_pipi2pt(8), tstep_pipitosigma(1), 
    pipi2pt_figure_file_fmt("traj_<TRAJ>_Figure<FIG>_sep<TSEP_PIPI>_mom<P1SRC>_mom<P1SNK>"), 
    sigma2pt_file_fmt("traj_<CONF>_sigmacorr_mompsrc<PSRC_QUARK>psnk<PSNK_QUARK>_v2"),
    pipitosigma_file_fmt("traj_<CONF>_pipitosigma_sigmawdagmom<MOM_QUARK_SIGMA>_pionmom<MOM_PI>_v2"),
    pipi_bubble_file_fmt("traj_<TRAJ>_FigureVdis_sep<TSEP_PIPI>_mom<PB>"),
    sigma_bubble_file_fmt("traj_<CONF>_sigmaself_mom<PQUARK>_v2")
  {}
   
};
GENERATE_PARSER(PiPiSigmaGEVPargs, PIPI_SIGMA_GEVP_ARGS_MEMBERS);

#endif
