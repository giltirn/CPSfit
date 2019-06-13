#ifndef _SIMPLE_FIT_WRAPPER_FREEZE_H_
#define _SIMPLE_FIT_WRAPPER_FREEZE_H_

//Convenience functions and types for loading data for frozen fits

#include<config.h>
#include<utils/macros.h>
#include<fit/fit_wrapper/fit_wrapper_freeze.h>
#include<fit/simple_fit_wrapper/fitter.h>

CPSFIT_START_NAMESPACE

//The main function - read and import the  frozen parameters. A struct "FreezeParams" is read in from "freeze_file" and used for perform the required actions
//For parameter types that don't have a default constructor the user should provide a pointer 'psetup' to a setup instance of the parameter type
void readFrozenParams(simpleFitWrapper &fitter, const std::string &freeze_file, const int nsample){
  if(!fileExists(freeze_file)){
    FreezeParams templ;
    std::ofstream of("freeze_template.dat");
    of << templ;
    of.close();
    error_exit(std::cout << "Failed to read freeze file " << freeze_file << "; wrote template to freeze_template.dat\n");
  } 
  
  std::vector<int> freeze;
  std::vector<jackknifeDistribution<double> > freeze_vals;
  FreezeParams fparams;
  parse(fparams,freeze_file);
  
  for(int i=0;i<fparams.sources.size();i++){
    std::cout << "readFrozenParams loading freeze data for parameter " << fparams.sources[i].param_idx << std::endl;
    freeze.push_back(fparams.sources[i].param_idx);

    jackknifeDistribution<double> fval;

    //Different sources of data
    FreezeDataReaderType reader = fparams.sources[i].reader;
    
    if(reader == FreezeDataReaderType::UKfitXMLvectorReader){
      readUKfitVectorEntry(fval, fparams.sources[i].filename, fparams.sources[i].input_idx[0]);
    }else if(reader == FreezeDataReaderType::HDF5fileReader){
      readHDF5file(fval, fparams.sources[i].filename, fparams.sources[i].input_idx);
    }else if(reader == FreezeDataReaderType::ConstantValue){
      fval.resize(nsample);
    }else{
      error_exit(std::cout << "readFrozenParams unknown reader " << fparams.sources[i].reader << std::endl);
    }

    if(fval.size() != nsample) error_exit(std::cout << "readFrozenParams read jackknife of size " << fval.size() << ", expected " << nsample << std::endl);

    applyOperation(fval, fparams.sources[i].operation, reader);

    std::cout << "readFrozenParams read " << fval << std::endl;

    freeze_vals.push_back(fval);
  }

  fitter.freeze(freeze, freeze_vals);
}

CPSFIT_END_NAMESPACE

#endif