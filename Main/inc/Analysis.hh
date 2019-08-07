#ifndef Analysis_hh_
#define Analysis_hh_

#include "RooDataHist.h"

#include "ConfigTools/inc/SimpleConfig.hh"

#include "Main/inc/Observable.hh"

namespace TrkAnaAnalysis {

  struct Analysis {
    Analysis(std::string name, const mu2e::SimpleConfig& config) : 
      name(name)
    {
      std::vector<std::string> all_obs;
      config.getVectorString(name+".observables", all_obs);
      for (const auto& i_obs : all_obs) {
	obs.insert(std::pair<std::string, Observable>(i_obs, Observable(config, i_obs)));
      }
      
      std::vector<std::string> all_cuts;
      config.getVectorString(name+".cuts", all_cuts);
      for (const auto& i_cut : all_cuts) {
	cuts.push_back(TCut(config.getString("cut."+i_cut).c_str()));
      }

      hMomT0 = new TH2F("hMomT0", "", obs.at("t0").n_bins(),obs.at("t0").min,obs.at("t0").max,obs.at("mom").n_bins(),obs.at("mom").min,obs.at("mom").max);
      data = new RooDataHist("data", "data", RooArgSet(obs.at("t0").roo_var, obs.at("mom").roo_var), RooFit::Import(*hMomT0));
    }

    std::string drawcmd() {
      return obs.at("mom").leaf + ":" + obs.at("t0").leaf + ">>hMomT0";
    }

    TCut cutcmd() { 
      TCut result;
      for (const auto& i_cut : cuts) {
	result += i_cut;
      }
      return result;
    }

    std::string name;

    std::map<std::string, Observable> obs;
    std::vector<TCut> cuts;

    TH2F* hMomT0;
    RooDataHist* data;
  };
}

#endif
