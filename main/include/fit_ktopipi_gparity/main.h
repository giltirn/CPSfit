#ifndef _FIT_KTOPIPI_MAIN_H
#define _FIT_KTOPIPI_MAIN_H

template<typename resampledDistributionType, typename Resampler>
void computeAlphaAndVacuumSubtractions(NumericTensor<resampledDistributionType,1> &alpha,
				       NumericTensor<resampledDistributionType,1> &A0_type4_srcavg_vacsub,
				       NumericTensor<resampledDistributionType,1> &mix4_srcavg_vacsub,
				       const NumericTensor<rawDataDistributionD,3> &A0_type4_nobub_alltK,
				       const NumericTensor<rawDataDistributionD,2> &mix4_nobub_alltK,
				       const NumericTensor<resampledDistributionType,1> &bubble_rs,
				       const int q,
				       const std::vector<int> &type4_nonzerotK,
				       const int tsep_k_pi,
				       const Args &args, const Resampler &resampler){
  //Compute mix4 double-jackknife and tK average
  resampledDistributionType zro = bubble_rs({0}); zeroit(zro);
  
#pragma omp parallel for
  for(int t=0;t<args.Lt;t++){
    mix4_srcavg_vacsub(&t) = zro;
    A0_type4_srcavg_vacsub(&t) = zro;

    resampledDistributionType mix4_nobub_srcavg = zro;
    resampledDistributionType A0_type4_nobub_srcavg = zro;
    
    for(int ii=0;ii<type4_nonzerotK.size();ii++){
      const int tK = type4_nonzerotK[ii];
      const int tB = (tK + tsep_k_pi) % args.Lt;      

      resampledDistributionType mix4_nobub_rs; resampler.resample(mix4_nobub_rs, mix4_nobub_alltK({tK,t}) );
      resampledDistributionType mix4_vacsub_rs = mix4_nobub_rs * bubble_rs(&tB);

      resampledDistributionType A0_type4_nobub_rs;  resampler.resample(A0_type4_nobub_rs, A0_type4_nobub_alltK({q,tK,t}) );
      resampledDistributionType A0_type4_vacsub_rs = A0_type4_nobub_rs * bubble_rs(&tB);

      mix4_srcavg_vacsub(&t) = mix4_srcavg_vacsub(&t) + mix4_vacsub_rs;
      mix4_nobub_srcavg = mix4_nobub_srcavg + mix4_nobub_rs;

      A0_type4_srcavg_vacsub(&t) = A0_type4_srcavg_vacsub(&t) + A0_type4_vacsub_rs;
      A0_type4_nobub_srcavg = A0_type4_nobub_srcavg + A0_type4_nobub_rs;
    }
    double n(type4_nonzerotK.size());
    
    mix4_srcavg_vacsub(&t) = mix4_srcavg_vacsub(&t)/n;
    A0_type4_srcavg_vacsub(&t) = A0_type4_srcavg_vacsub(&t)/n;
    mix4_nobub_srcavg = mix4_nobub_srcavg/n;
    A0_type4_nobub_srcavg = A0_type4_nobub_srcavg/n;

    alpha(&t) = A0_type4_nobub_srcavg/mix4_nobub_srcavg;
  }
}


template<typename DistributionType, typename Resampler>
NumericTensor<DistributionType,1> resampleAverageTypeData(const NumericTensor<rawDataDistributionD,3> &typedata_alltK,
							  const int q,
							  const std::vector<int> &typedata_nonzerotK,
							  const Args &args, const Resampler &resampler){
  NumericTensor<DistributionType,1> out({args.Lt}); //[t]
  for(int t=0;t<args.Lt;t++)
    resampleAverage(out(&t), resampler, [&](const int i){ return typedata_alltK({q,typedata_nonzerotK[i],t}); }, typedata_nonzerotK.size());
  return out;
}

template<typename DistributionType, typename Resampler>
NumericTensor<DistributionType,1> resampleAverageMixDiagram(const NumericTensor<rawDataDistributionD,2> &mixdata_alltK,
							    const std::vector<int> &mixdata_nonzerotK,
							    const Args &args, const Resampler &resampler){
  NumericTensor<DistributionType,1> out({args.Lt}); //[t]
  for(int t=0;t<args.Lt;t++)
    resampleAverage(out(&t), resampler, [&](const int i){ return mixdata_alltK({mixdata_nonzerotK[i],t}); }, mixdata_nonzerotK.size());
  return out;
}



template<typename DistributionType, typename Resampler>
NumericTensor<DistributionType,1> computeQamplitude(const int q, const int tsep_k_pi, const RawKtoPiPiData &raw, const BubbleData &bubble_data, const Args &args, const CMDline &cmdline, const std::string &descr, const Resampler &resampler){
  //Compute alpha and type4/mix4 vacuum subtractions
  std::cout << "Computing " << descr << " alpha and vacuum subtractions\n";
  NumericTensor<DistributionType,1> alpha_r({args.Lt}), A0_type4_srcavg_vacsub_r({args.Lt}), mix4_srcavg_vacsub_r({args.Lt}); //[t]
  computeAlphaAndVacuumSubtractions(alpha_r, A0_type4_srcavg_vacsub_r, mix4_srcavg_vacsub_r,
				    raw.A0_type4_alltK_nobub, raw.mix4_alltK_nobub, getResampledBubble<DistributionType>::get(bubble_data),q, raw.nonzerotK(4),tsep_k_pi,args,resampler);

  //Compute tK-averages type4 and mix4 diagrams from data including bubble-------------//
  std::cout << "Computing " << descr << " tK averages and mix diagrams\n";
  IndexedContainer<NumericTensor<DistributionType,1>, 4, 1> A0_srcavg_r; //[t]
  IndexedContainer<NumericTensor<DistributionType,1>, 2, 3> mix_srcavg_r; //[t]
  for(int i=1;i<=4;i++) A0_srcavg_r(i) = resampleAverageTypeData<DistributionType>(raw.A0_alltK(i), q, raw.nonzerotK(i), args, resampler); //[t]
  for(int i=3;i<=4;i++) mix_srcavg_r(i) = resampleAverageMixDiagram<DistributionType>(raw.mix_alltK(i), raw.nonzerotK(i), args, resampler);

  //Subtract the pseudoscalar operators and mix4 vacuum term
  std::cout << "Subtracting pseudoscalar operators and mix4 vacuum term under " << descr << "\n";
  A0_srcavg_r(3) = A0_srcavg_r(3).transform([&](int const* t, const DistributionType &from){ return DistributionType(from - alpha_r(t)*mix_srcavg_r(3)(t)); }); 
  A0_srcavg_r(4) = A0_srcavg_r(4).transform([&](int const* t, const DistributionType &from){
      return DistributionType(from - alpha_r(t)*( mix_srcavg_r(4)(t) - mix4_srcavg_vacsub_r(t) ) );
    }); 

  //Perform the type 4 vacuum subtraction
  std::cout << "Performing type-4 vacuum subtraction\n";
  A0_srcavg_r(4) = A0_srcavg_r(4) - A0_type4_srcavg_vacsub_r;

  //Get the full double-jackknife amplitude
  std::cout << "Computing full amplitudes\n";
  NumericTensor<DistributionType,1> A0_full_srcavg_r = A0_srcavg_r(1) + A0_srcavg_r(2) + A0_srcavg_r(3) + A0_srcavg_r(4);

  return A0_full_srcavg_r;
}

//Read and prepare the data for a particular tsep_k_pi_idx
template<typename Resampler>
void getData(std::vector<correlationFunction<amplitudeDataCoord, jackknifeDistributionD> > &A0_all_j, 
	     std::vector<correlationFunction<amplitudeDataCoord, doubleJackknifeA0StorageType> > &A0_all_dj,
	     const BubbleData &bubble_data, const int tsep_k_pi_idx, const Args &args, const CMDline &cmdline, const Resampler &resampler){
  int tsep_k_pi = args.tsep_k_pi[tsep_k_pi_idx];
  std::cout << "Getting data for tsep_k_pi = " <<  tsep_k_pi << std::endl;
  printMem("getData called");
  
  RawKtoPiPiData raw(tsep_k_pi, bubble_data, args, cmdline);
  
  for(int q=0;q<10;q++){
    std::cout << "Starting Q" << q+1 << std::endl;

    printMem("Starting new Q");
 
    NumericTensor<doubleJackknifeDistributionD,1> A0_full_srcavg_dj = computeQamplitude<doubleJackknifeDistributionD>(q, tsep_k_pi, raw, bubble_data, args, cmdline, "double jackknife", resampler);
    NumericTensor<jackknifeDistributionD,1> A0_full_srcavg_j = computeQamplitude<jackknifeDistributionD>(q, tsep_k_pi, raw, bubble_data, args, cmdline, "single jackknife", resampler);

    //Insert data into output containers    
    for(int t=0;t<args.Lt;t++){
      A0_all_j[q].push_back(amplitudeDataCoord(t,tsep_k_pi), A0_full_srcavg_j(&t));
      A0_all_dj[q].push_back(amplitudeDataCoord(t,tsep_k_pi), A0_full_srcavg_dj(&t));
    }
  }
}

template<typename Resampler>
void getData(std::vector<correlationFunction<amplitudeDataCoord, jackknifeDistributionD> > &A0_all_j, 
	     std::vector<correlationFunction<amplitudeDataCoord, doubleJackknifeA0StorageType> > &A0_all_dj,
	     const Args &args, const CMDline &cmdline, const Resampler &resampler){
  if(cmdline.load_amplitude_data){
#ifdef HAVE_HDF5
    HDF5reader reader(cmdline.load_amplitude_data_file);
    read(reader, A0_all_j, "A0_all_j");
    read(reader, A0_all_dj, "A0_all_dj");
#else
    error_exit("getData: Reading amplitude data requires HDF5\n");
#endif
  }else{
    //Read the bubble data
    BubbleData bubble_data(args,cmdline, resampler);
    
    //Read and prepare the amplitude data for fitting
    scratch scratch_store(args,cmdline); //Setup scratch space if in use

    for(int tsep_k_pi_idx=0;tsep_k_pi_idx<args.tsep_k_pi.size();tsep_k_pi_idx++){
      if(scratch_store.doSkipLoad(tsep_k_pi_idx)) continue;
      
      getData(A0_all_j,A0_all_dj,bubble_data,tsep_k_pi_idx,args,cmdline, resampler);

      scratch_store.writeScratch(A0_all_j, A0_all_dj, tsep_k_pi_idx);
    }	

    scratch_store.reloadScratch(A0_all_j, A0_all_dj);      
  }

  if(cmdline.save_amplitude_data){
#ifdef HAVE_HDF5
    HDF5writer writer(cmdline.save_amplitude_data_file);
    write(writer, A0_all_j, "A0_all_j");
    write(writer, A0_all_dj, "A0_all_dj");
#else
    error_exit("getData: Saving amplitude data requires HDF5\n");
#endif
  }

  
}


void getData(std::vector<correlationFunction<amplitudeDataCoord, jackknifeDistributionD> > &A0_all_j, 
	     std::vector<correlationFunction<amplitudeDataCoord, doubleJackknifeA0StorageType> > &A0_all_dj,
	     const Args &args, const CMDline &cmdline){
  basic_resampler resampler;
  getData(A0_all_j, A0_all_dj, args, cmdline, resampler);
}


void checkpointRawOnly(const Args &args, const CMDline &cmdline){
  if(!cmdline.save_data_checkpoint) error_exit(std::cout << "checkpointRawOnly expect cmdline.save_data_checkpoint to be true\n");
  basic_resampler resampler;
  BubbleData bubble_data(args,cmdline, resampler);
  for(int tsep_k_pi_idx=0;tsep_k_pi_idx<args.tsep_k_pi.size();tsep_k_pi_idx++){
    int tsep_k_pi = args.tsep_k_pi[tsep_k_pi_idx];
    std::cout << "Getting data for tsep_k_pi = " <<  tsep_k_pi << std::endl;
    RawKtoPiPiData raw(tsep_k_pi, bubble_data, args, cmdline);
  }
}


#endif
