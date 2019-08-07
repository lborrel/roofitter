#ifndef InputParameters_hh_
#define InpitParameters_hh_

#include "ConfigTools/inc/SimpleConfig.hh"

namespace TrkAnaAnalysis {

  struct InputParameters {
    void Import(const mu2e::SimpleConfig& config) {
      input_filename = config.getString("input.filename");
      input_treename = config.getString("input.treename");
    }
    std::string input_filename;
    std::string input_treename;
  };
}

#endif
