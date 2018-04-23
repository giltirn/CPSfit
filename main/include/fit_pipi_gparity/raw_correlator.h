#ifndef _PIPI_RAW_CORRELATOR_H_
#define _PIPI_RAW_CORRELATOR_H_

template<typename DataAllMomentumType>
typename DataAllMomentumType::ContainerType project(const char fig, const DataAllMomentumType &raw_data, 
						    const PiPiProject &proj_src, const PiPiProject &proj_snk, const PiPiMomAllow &allow, 
						    const std::vector<threeMomentum> &pion_momenta){
  std::cout << "Computing projection of figure " << fig << " with DataAllMomentumType = " << printType<DataAllMomentumType>() << "\n"; 
  boost::timer::auto_cpu_timer t(std::string("Report: Computed projection of figure ") + fig + " with DataAllMomentumType = " + printType<DataAllMomentumType>() + " in %w s\n");

  const int nmom = pion_momenta.size();
  
  typename DataAllMomentumType::ContainerType out(raw_data.getLt(), raw_data.getNsample()); out.zero();

  std::complex<double> csnk, csrc;
  double m;
  for(int psnk=0;psnk<nmom;psnk++){
    if(!proj_snk(csnk, pion_momenta[psnk])) continue;

    for(int psrc=0;psrc<nmom;psrc++){
      if(!proj_src(csrc, pion_momenta[psrc])) continue;
      
      if(!allow(m,pion_momenta[psrc],pion_momenta[psnk])) continue;

      out = out + m*std::real(csnk*csrc)*raw_data(fig, momComb(pion_momenta[psnk], pion_momenta[psrc]));
    }
  }

  return out;
}

template<typename FigureDataType>
auto sourceAverage(const FigureDataType & data)->correlationFunction<double,typename std::decay<decltype(data(0,0))>::type>{
  typedef typename std::decay<decltype(data(0,0))>::type DistributionType;

  int Lt = data.getLt();
  int nsample = data.getNsample();
  correlationFunction<double,DistributionType> into(data.getLt(),
					     [nsample](int i) {  return typename correlationFunction<double,DistributionType>::ElementType(i, DistributionType(nsample,0.)); }
					     );
  std::vector<int> tsrc_include;
  for(int tsrc=0;tsrc<Lt;tsrc++){
    bool is_nonzero = !data.isZero(tsrc);
    if(is_nonzero)
      tsrc_include.push_back(tsrc);
  }
  double N(tsrc_include.size());

  for(int tsep=0;tsep<Lt;tsep++){
    auto & v = into.value(tsep);
    v.zero();
    for(int i=0;i<tsrc_include.size();i++)
      v = v + data(tsrc_include[i],tsep);
    v = v/N;
  }
  return into;
}

void outputRawData(const std::string &filename, const correlationFunction<double,rawDataDistributionD> &data, const double coeff){
  std::ofstream of(filename.c_str());
  of << std::setprecision(11) << std::scientific;
  int Lt = data.size();
  int nsample = data.value(0).size();

  for(int t=0;t<data.size();t++)
    for(int s=0;s<nsample;s++)
      of << t << " " << s << " " << coeff * data.value(t).sample(s) << " " << 0. << std::endl;
  of.close();
}

typedef correlationFunction<double,rawDataDistributionD> rawCorrelationFunction;

inline void bin(rawCorrelationFunction &raw, const int bin_size){
  for(int i=0;i<raw.size();i++) raw.value(i) = raw.value(i).bin(bin_size);
}

//Compute the raw , unbinned, unresampled pipi correlation function from the underlying contraction data
void getRawPiPiCorrFunc(rawCorrelationFunction &pipi_raw, const figureDataAllMomenta &raw_data, const bubbleDataAllMomenta &raw_bubble_data, 
			const PiPiProject &proj_src, const PiPiProject &proj_snk, const PiPiMomAllow &allow, const int isospin, const std::vector<threeMomentum> &pion_momenta,
			const int bin_size, const std::string &extra_descr = "", bool output_raw_data = true){
 
  figureData A2_C = project('C', raw_data, proj_src, proj_snk, allow, pion_momenta);
  figureData A2_D = project('D', raw_data, proj_src, proj_snk, allow, pion_momenta);
  figureData A2_R = project('R', raw_data, proj_src, proj_snk, allow, pion_momenta);
  figureData A2_V = project('V', raw_data, proj_src, proj_snk, allow, pion_momenta);
  
  rawCorrelationFunction A2_realavg_C = sourceAverage(A2_C);
  rawCorrelationFunction A2_realavg_D = sourceAverage(A2_D);
  rawCorrelationFunction A2_realavg_R = sourceAverage(A2_R);
  rawCorrelationFunction A2_realavg_V = sourceAverage(A2_V);

  if(output_raw_data){
    std::string ee = extra_descr != "" ? "_" + extra_descr : "";
    //These data are saved after binning
    rawCorrelationFunction A2_realavg_C_b(A2_realavg_C);
    rawCorrelationFunction A2_realavg_D_b(A2_realavg_D);
    rawCorrelationFunction A2_realavg_R_b(A2_realavg_R);
    rawCorrelationFunction A2_realavg_V_b(A2_realavg_V);
    bin(A2_realavg_C_b, bin_size);
    bin(A2_realavg_D_b, bin_size);
    bin(A2_realavg_R_b, bin_size);
    bin(A2_realavg_V_b, bin_size);    

    outputRawData("raw_data_Cpart"+ee+".dat", A2_realavg_C_b, 1.);
    outputRawData("raw_data_Dpart"+ee+".dat", A2_realavg_D_b, 2.);
    outputRawData("raw_data_Rpart"+ee+".dat", A2_realavg_R_b, -6.);
    outputRawData("raw_data_Vpart"+ee+".dat", A2_realavg_V_b, 3.);
  }    
  
  if(isospin == 0){
    pipi_raw = 2*A2_realavg_D + A2_realavg_C - 6*A2_realavg_R + 3*A2_realavg_V;
  }else if(isospin == 2){
    pipi_raw = 2*A2_realavg_D - 2*A2_realavg_C;
  }else error_exit(std::cout << "getRawPiPiCorrFunc only supports isospin 0,2\n");

  std::cout << "Raw data " << extra_descr << ":\n" << pipi_raw << std::endl;
}



#endif
