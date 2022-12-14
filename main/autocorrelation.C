//Compute the autocorrelation of a jackknife distribution
#include<distribution.h>
#include<fit/fit_wrapper/fit_wrapper_freeze.h>
#include<common.h>
#include<plot.h>

using namespace CPSfit;

std::vector<int> parseIndices(const std::string &ind){
  std::vector<int> indices;
  std::istringstream buffer(ind);
  std::string line;
  while(std::getline(buffer, line, ',')){
    int i;
    std::stringstream ss; ss << line; ss >> i;
    indices.push_back(i);
  }
  return indices;
}

std::vector<double> autoCorrelation(const jackknifeDistributionD &A){
  const int nsample = A.size();
  double Abar = A.mean();
  double Astddev = A.standardError()/sqrt(nsample - 1.);

  const int maxDelta = nsample-1;

  std::vector<double> out(maxDelta+1);
  
  for(int Delta = 0; Delta <= maxDelta; Delta++){
    double v = 0.;
    int N = 0;
    for(int i=0;i<nsample;i++){
      if(i + Delta < nsample){
	v += (A.sample(i) - Abar)*(A.sample(i+Delta) - Abar);
      }
    }
    v = v/Astddev/Astddev/nsample;
    
    out[Delta] = v;
  }
  return out;
}

std::vector<double> integratedAutoCorrelation(const std::vector<double> &C){
  int N = C.size();
  std::vector<double> out(N);
  for(int cut=1; cut < N; cut++){
    double v = 0.5;
    for(int i=1;i<=cut;i++)
      v += C[i];
    out[cut-1] = v;
  }
  return out;
}

std::vector<bootstrapDistributionD> autoCorrelation(const jackknifeDistributionD &A, const int bin_size){
  typename bootstrapDistributionD::initType boot_params;
  boot_params.boots = 500;
  boot_params.confidence = 68;

  const int nsample = A.size();
  const int maxDelta = nsample-1;

  double Abar = A.mean();
  double Astddev = A.standardError()/sqrt(nsample - 1.);

  std::vector<bootstrapDistributionD> out;
  
  std::cout << "Computing autocorrelation function:\n";

  for(int Delta = 0; Delta <= maxDelta; Delta++){
    int ndelta = 0;
    for(int i=0;i<nsample;i++)      
      if(i + Delta < nsample)
	++ndelta;

    if(ndelta < 2*bin_size) continue; //only one binned sample, no error possible

    //Construct  (Y_i - Ybar)(Y_{i+Delta} - Ybar)/sigma^2   for each sample i
    rawDataDistributionD C_samples(ndelta);

    int nn=0;
    for(int i=0;i<nsample;i++)      
      if(i + Delta < nsample){
	double v = (A.sample(i) - Abar)*(A.sample(i+Delta) - Abar)/Astddev/Astddev;
	C_samples.sample(nn++) = v;
      }  
    
    //These are many approximations to the autocorrelation function. We wish to average them. Do this under a boostrap to get a width also
    //Increase bin size until this error stops growing

    rawDataDistributionD C_samples_binned = C_samples.bin(bin_size,true);
    bootstrapDistributionD C_boot(C_samples_binned, boot_params);

    std::cout << Delta << " " << C_boot << std::endl;
    out.push_back(C_boot);
  }
  return out;
}



std::vector<bootstrapDistributionD> integratedAutoCorrelation(const std::vector<bootstrapDistributionD> &C){
  int N = C.size();
  std::vector<bootstrapDistributionD> out(N);
  for(int cut=1; cut < N; cut++){
    bootstrapDistributionD v = C[0];
    for(int i=0;i<v.size();i++) v.sample(i) = 0.5;

    for(int i=1;i<=cut;i++)
      v = v + C[i];
    out[cut-1] = v;
  }
  return out;
}


int main(const int argc, const char** argv){
  assert(argc >= 5);
  std::string file = argv[1];
  std::vector<int> idx = parseIndices(argv[2]);
  int bin_size = strToAny<int>(argv[3]);
  std::string plot_stub = argv[4];

  int stop_sep = -1;

  for(int i=5;i<argc;i++){
    std::string arg = argv[i];
    if(arg == "-stop"){
      stop_sep = strToAny<int>(argv[i+1]);
      i+=2;
    }else assert(0);
  }


  jackknifeDistributionD A;
  readHDF5file(A,file,idx);

  std::vector<bootstrapDistributionD> C = autoCorrelation(A, bin_size);
  std::vector<bootstrapDistributionD> tau_int = integratedAutoCorrelation(C);
  
  std::cout << "Integrated autocorrelation length:\n";
  for(int i=0;i<tau_int.size();i++)
    std::cout << i+1 << " " << tau_int[i] << std::endl;


  {
    MatPlotLibScriptGenerate plot;
    
    struct Accessor{
      const std::vector<bootstrapDistributionD> &vec;
      Accessor(const std::vector<bootstrapDistributionD> &vec): vec(vec){}
      
      inline double x(const int i) const{ return i+1; }
      inline double upper(const int i) const{ return vec[i].confidenceRegion().second; }
      inline double lower(const int i) const{ return vec[i].confidenceRegion().first; }
      inline int size() const{ return vec.size(); }
    };
    
    Accessor acc(tau_int);
    plot.errorBand(acc);
    plot.setXlabel(R"($\Delta_{\rm cut}$)");
    plot.setYlabel(R"($\tau_{\rm int}(\Delta_{\rm cut}$))");
    plot.write(plot_stub +".py", plot_stub+".pdf");
  }


  return 0;
}

