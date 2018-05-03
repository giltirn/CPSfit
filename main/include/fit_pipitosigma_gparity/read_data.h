#ifndef _PIPI_TO_SIGMA_READ_DATA_H_
#define _PIPI_TO_SIGMA_READ_DATA_H_

std::string pipiToSigmaFile(const std::string &data_dir, const int traj, const threeMomentum &p_pi, const threeMomentum &p_quark_sigma){
  //data/traj_0_pipitosigma_sigmawdagmom_13_1_pionmom_2_2_2_v2
  std::ostringstream os;
  os << data_dir << "/traj_" << traj << "_pipitosigma_sigmawdagmom"<< momStr(p_quark_sigma) << "_pionmom" << momStr(p_pi) << "_v2";
  return os.str();
}


void readPiPiToSigma(figureData &raw_data, const std::string &data_dir, const int Lt,
		     const int traj_start, const int traj_inc, const int traj_lessthan){
  std::cout << "Reading pipi->sigma data\n"; boost::timer::auto_cpu_timer t("Read pipi->sigma in %w s\n");
  int nsample = (traj_lessthan - traj_start)/traj_inc;

  raw_data.setup(Lt,nsample);
  raw_data.zero();

  std::vector<threeMomentum> quark_mom = { {1,1,1}, {-1,-1,-1},
					   {-3,1,1}, {3,-1,-1},
					   {1,-3,1}, {-1,3,-1},
					   {1,1,-3}, {-1,-1,3} };

  
  std::vector<threeMomentum> pion_mom = { {2,2,2}, {-2,-2,-2},
					  {-2,2,2}, {2,-2,-2},
					  {2,-2,2}, {-2,2,-2},
					  {2,2,-2}, {-2,-2,2} };

  figureData tmp_raw_data(Lt,nsample);

  for(int ppiidx = 0 ; ppiidx < 8 ; ppiidx++){
    for(int psigqidx = 0 ; psigqidx < 8 ; psigqidx++){
#pragma omp parallel for
      for(int sample=0; sample < nsample; sample++){
	int traj = traj_start + sample * traj_inc;
	std::string filename = pipiToSigmaFile(data_dir, traj, pion_mom[ppiidx], quark_mom[psigqidx]);
	std::cout << "Parsing " << filename << std::endl;
	tmp_raw_data.parseCDR(filename, sample);
      }

      raw_data = raw_data + tmp_raw_data;
    }
  }
  
  raw_data = raw_data/64.;
}

bubbleData getPiPiBubble(const std::string &data_dir, const int traj_start, const int traj_inc, const int traj_lessthan, const int tsep_pipi, const int Lt){
  int nsample_raw = (traj_lessthan - traj_start)/traj_inc;

  readBubbleStationaryPolicy pipi_policy(false,Source);
  bubbleDataAllMomenta pipi_self_data(Lt, tsep_pipi, nsample_raw);
  std::vector<threeMomentum> pion_mom = { {1,1,1}, {-1,-1,-1},
					  {-1,1,1}, {1,-1,-1},
					  {1,-1,1}, {-1,1,-1},
					  {1,1,-1}, {-1,-1,1} };
  PiPiProjectA1 pion_proj;

  readBubble<readBubbleStationaryPolicy>(pipi_self_data, data_dir, tsep_pipi, Lt, traj_start, traj_inc, traj_lessthan, pipi_policy, pion_mom, pion_proj, Source);
  
  bubbleData out(Source,Lt,tsep_pipi,nsample_raw);
  out.zero();

  for(int t=0;t<Lt;t++)
    for(int p=0;p<pion_mom.size();p++)
      out(t) = out(t) + pipi_self_data(Source,pion_mom[p])(t)/double(pion_mom.size()); //A1 project
  
  return out;
}

#endif
