#ifndef CeM_hh_
#define CeM_hh_

#include "cetlib_except/exception.h"
#include "ConfigTools/inc/SimpleConfig.hh"

#include "RooAbsPdf.h"
#include "RooGaussian.h"

namespace TrkAnaAnalysis {

  struct CeM {
    CeM(const mu2e::SimpleConfig& config, std::string name, Observable& obs) :
      model(config.getString(name+".model")),
      eMax("eMax", "eMax", 104.97),
      signal_sigma("signal_sigma", "signal_sigma", 0.01)
    {
      if (model == "LeadingOrder") {
	pdf = new RooGaussian("cem", "cem", obs.roo_var, eMax, signal_sigma);
      }
      else {
	throw cet::exception("TrkAnaAnalysis::CeM") << "Unsupported model (" << model << ")";
      }
    }

    std::string model;
    RooAbsPdf* pdf;
    RooRealVar eMax;
    RooRealVar signal_sigma;
  };
}

#endif
