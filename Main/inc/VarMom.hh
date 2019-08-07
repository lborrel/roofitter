#ifndef VarMom_hh_
#define VarMom_hh_

#include "ConfigTools/inc/SimpleConfig.hh"

#include "RooRealVar.h"

namespace TrkAnaAnalysis {

  struct VarMom {
    VarMom(const mu2e::SimpleConfig& config) :
      min(config.getDouble("mom.hist.min")),
      max(config.getDouble("mom.hist.max")),
      bin_width(config.getDouble("mom.hist.bin_width")),
      roo_var(RooRealVar("mom", "mom", min, max))
    {
    }

    int n_bins() { return (max - min)/bin_width; }
    double min;
    double max;
    double bin_width;
    RooRealVar roo_var;
  };
}

#endif
