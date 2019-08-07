#ifndef InputParameters_hh_
#define InpitParameters_hh_

#include "ConfigTools/inc/SimpleConfig.hh"

#include "Main/inc/VarMom.hh"

namespace TrkAnaAnalysis {

  struct InputParameters {
    InputParameters(const mu2e::SimpleConfig& config) : 
      mom(config) {
    }

    VarMom mom;
  };
}

#endif
