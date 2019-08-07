#ifndef CeM_hh_
#define CeM_hh_

#include "cetlib_except/exception.h"
#include "ConfigTools/inc/SimpleConfig.hh"

#include "RooAbsPdf.h"
#include "RooGaussian.h"

#include "Main/inc/Component.hh"

namespace TrkAnaAnalysis {

  class CeM : public Component {
  public:

    CeM(const mu2e::SimpleConfig& config, std::string name, std::vector<Observable>& obs) :
      Component(config, name)
    {
      eMax = new RooRealVar("eMax", "eMax", 104.97);
      signal_sigma = new RooRealVar("signal_sigma", "signal_sigma", 0.01);
      for (auto& i_obs : obs) {
	std::string model = config.getString(name+"."+i_obs.name+".model");
	if (i_obs.name == "mom") {
	  if (model == "LeadingOrder") {
	    pdf = new RooGaussian("cem", "cem", i_obs.roo_var, *eMax, *signal_sigma);
	  }
	  else {
	    throw cet::exception("TrkAnaAnalysis::CeM") << "Unsupported mom model (" << model << ")";
	  }
	}
      }
    }

    RooRealVar* eMax;
    RooRealVar* signal_sigma;
  };
}

#endif
