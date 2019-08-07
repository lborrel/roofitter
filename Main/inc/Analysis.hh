#ifndef Analysis_hh_
#define Analysis_hh_

#include "ConfigTools/inc/SimpleConfig.hh"

#include "Main/inc/Observable.hh"

namespace TrkAnaAnalysis {

  struct Analysis {
    Analysis(const mu2e::SimpleConfig& config) : 
      mom(config, "mom"),
      t0(config, "t0"),
      hMomT0(TH2F("hMomT0", "", t0.n_bins(),t0.min,t0.max, mom.n_bins(),mom.min,mom.max))
    {
      
      std::vector<std::string> all_cuts;
      config.getVectorString("cuts", all_cuts);
      for (const auto& i_cut : all_cuts) {
	cuts.push_back(TCut(config.getString("cut."+i_cut).c_str()));
      }
    }

    std::string drawcmd() {
      return mom.leaf + ":" + t0.leaf + ">>hMomT0";
    }

    TCut cutcmd() { 
      TCut result;
      for (const auto& i_cut : cuts) {
	result += i_cut;
      }
      return result;
    }

    Observable mom;
    Observable t0;
    TH2F hMomT0;
    std::vector<TCut> cuts;
  };
}

#endif
