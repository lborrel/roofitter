#ifndef Observable_hh_
#define Observable_hh_

#include "ConfigTools/inc/SimpleConfig.hh"

#include "RooRealVar.h"

namespace TrkAnaAnalysis {

  struct Observable {
    Observable(const mu2e::SimpleConfig& config, std::string name) :
      name(name),
      leaf(config.getString(name+".leaf")),
      min(config.getDouble(name+".hist.min")),
      max(config.getDouble(name+".hist.max")),
      bin_width(config.getDouble(name+".hist.bin_width")),
      roo_var(RooRealVar(name.c_str(), name.c_str(), min, max))
    {
    }

    int n_bins() { return (max - min)/bin_width; }
    RooPlot* frame() { return roo_var.frame(); }

    std::string name;
    std::string leaf;
    double min;
    double max;
    double bin_width;
    RooRealVar roo_var;
  };
}

#endif
