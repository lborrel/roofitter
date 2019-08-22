#ifndef Observable_hh_
#define Observable_hh_

#include "RooWorkspace.h"
#include "ConfigTools/inc/SimpleConfig.hh"

namespace trkana {

  struct Observable {

    Observable(const std::string& obs_name, const std::string& eff_name, const std::string& res_name, const mu2e::SimpleConfig& config, RooWorkspace* ws) : name(obs_name), effName(eff_name), resName(res_name) {

      std::stringstream factory_cmd;

      // Define the observable itself
      hist_min = config.getDouble(name+".hist.min");
      hist_max = config.getDouble(name+".hist.max");
      factory_cmd.str("");
      factory_cmd << name << "[" << hist_min << ", " << hist_max << "]";
      ws->factory(factory_cmd.str().c_str());      
      hist_bin_width = config.getDouble(name+".hist.bin_width");
      ws->var(obs_name.c_str())->setBins( (hist_max-hist_min)/hist_bin_width );
      
      // Record the leaf in the tree that corresponds to this observable
      leaf = config.getString(obs_name+".leaf");

      // Record the fit region
      fit_min = config.getDouble(name+".fit.min");
      fit_max = config.getDouble(name+".fit.max");
      ws->var(obs_name.c_str())->setRange("fit", fit_min, fit_max);

      // Construct the efficiency pdf for this observable
      if (!effName.empty()) {
	std::string eff_type = config.getString(name+".eff."+effName+".type");
	  
	// If the efficiency is defined as a function
	if (eff_type == "func") {

	  RooArgList list; // need to keep track of all vars that go into the function
	  list.add(*ws->var(name.c_str())); // add the observable

	  // Create all the other parameters
	  std::vector<std::string> eff_params;
	  config.getVectorString(name+".eff."+effName+".func.params", eff_params);
	  for (const auto i_param : eff_params) {
	    factory_cmd.str("");
	    //TODO: allow for floating parameters
	    factory_cmd << i_param << "[" << config.getDouble(name+".eff."+effName+".func."+i_param) << "]";
	    ws->factory(factory_cmd.str().c_str());
	    
	    list.add(*ws->var(i_param.c_str()));
	  }

	  // Create the function itself
	  std::string eff_func = config.getString(name+".eff."+effName+".func");
	  ws->import(*( new RooFormulaVar(effName.c_str(), eff_func.c_str(), list)));
	}
	else {
	  throw cet::exception("Observable") << "Unsupported efficiency type: " << eff_type;
	}
      }

      // Construct the resolution pdf for this observable
      if (!resName.empty()) {
	std::string res_type = config.getString(name+".res."+resName+".type");
	  
	if (res_type == "pdf") {
	  std::string res_pdf = config.getString(name+".res."+resName+".pdf");
	  factory_cmd.str("");
	  factory_cmd << res_pdf;
	  ws->factory(factory_cmd.str().c_str());

	  // The resolution might only be valid in a certain resolution region
	  res_valid_min = config.getDouble(name+".res."+resName+".valid.min");
	  res_valid_max = config.getDouble(name+".res."+resName+".valid.max");
	}
	/* // Can try sorting this out in the future
	else if (res_type == "hist") {
	  std::string res_filename = config.getString(name+"res."+resName+".hist.file");
	  std::string res_histname = config.getString(name+"res."+resName+".hist.name");
	    
	  TFile* res_file = new TFile(res_filename.c_str(), "READ");
	  if (res_file->IsZombie()) {
	    throw cet::exception("Analysis::Analsysis") << "Resolution file " << res_filename << " is a zombie";
	  }
	  TH1* res_hist = (TH1*) res_file->Get(res_histname.c_str());
	  if (!res_hist) {
	    throw cet::exception("Analysis::Analysis") << "Resolution histogram " << res_histname << " is not in file";
	  }

	  ws->import(*(new RooDataHist("res_hist", "res_hist", *ws->var(name.c_str()), RooFit::Import(*res_hist))));
	  ws->import(*(new RooHistPdf("res", "", *ws->var(name.c_str()), *((RooDataHist*)ws->data("res_hist")))));
	}
	*/
	else {
	  throw cet::exception("Observable") << "Unsupported resolution type: " << res_type;
	}
      }
    }

    std::string name;

    std::string leaf;
    double hist_min;
    double hist_max;
    double hist_bin_width;
    double fit_min;
    double fit_max;

    // Efficiency
    std::string effName;

    // Resolution
    std::string resName;
    double res_valid_min;
    double res_valid_max;
    
  };

  typedef std::vector<Observable> Observables;
}

#endif
