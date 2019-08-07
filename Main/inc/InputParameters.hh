#ifndef InputParameters_hh_
#define InpitParameters_hh_

#include "ConfigTools/inc/SimpleConfig.hh"

#include "Main/inc/Observable.hh"

namespace TrkAnaAnalysis {

  struct InputParameters {
    InputParameters(const mu2e::SimpleConfig& config) : 
      mom(config, "mom") {
    }

    Observable mom;
  };
}

#endif
