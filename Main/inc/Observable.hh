#ifndef Observable_hh_
#define Observable_hh_

#include "RooWorkspace.h"
#include "ConfigTools/inc/SimpleConfig.hh"
#include "fhiclcpp/types/OptionalAtom.h"
#include "fhiclcpp/types/OptionalTable.h"

#include "Main/inc/Configs.hh"

namespace roofitter {

  struct EffModelConfig {
    fhicl::Atom<std::string> name{fhicl::Name("name"), fhicl::Comment("Efficiency model name")};
    fhicl::OptionalTable<FormulaConfig> formula{fhicl::Name("formula"), fhicl::Comment("Configuration for the formula for the efficiency model")};
  };

  struct RespModelConfig {
    fhicl::Atom<std::string> name{fhicl::Name("name"), fhicl::Comment("Response model name")};
    fhicl::OptionalTable<PdfConfig> pdf{fhicl::Name("pdf"), fhicl::Comment("PDF for the response model")};
    fhicl::Atom<double> validMin{fhicl::Name("validMin"), fhicl::Comment("Minimum of region of validity")};
    fhicl::Atom<double> validMax{fhicl::Name("validMax"), fhicl::Comment("Maximum of region of validity")};
  };

  struct ObservableConfig {
    fhicl::Atom<std::string> name{fhicl::Name("name"), fhicl::Comment("Observable name")};
    fhicl::Atom<double> min{fhicl::Name("min"), fhicl::Comment("Minimum value of observable")};
    fhicl::Atom<double> max{fhicl::Name("max"), fhicl::Comment("Maximum value of observable")};
    fhicl::Atom<std::string> leaf{fhicl::Name("leaf"), fhicl::Comment("Leaf name for this observable")};
    fhicl::Atom<double> fitMin{fhicl::Name("fitMin"), fhicl::Comment("Fit range min")};
    fhicl::Atom<double> fitMax{fhicl::Name("fitMax"), fhicl::Comment("Fit range max")};
    fhicl::Atom<double> binWidth{fhicl::Name("binWidth"), fhicl::Comment("Width of bin for histogram")};

    fhicl::OptionalTable<EffModelConfig> efficiencyModel{fhicl::Name("efficiencyModel"), fhicl::Comment("Efficiency model config for this observable")};
    fhicl::OptionalTable<RespModelConfig> responseModel{fhicl::Name("responseModel"), fhicl::Comment("Response model config for this observable")};
  };

  class Observable {
  private: 
    ObservableConfig _obsConf;
    EffModelConfig _effModelConf;
    RespModelConfig _respModelConf;

  public:
    const ObservableConfig& getConf() const { return _obsConf; }

    std::string getName() const { return _obsConf.name(); }
    std::string getLeaf() const { return _obsConf.leaf(); }
    double getMin() const { return _obsConf.min(); }
    double getMax() const { return _obsConf.max(); }
    double getBinWidth() const { return _obsConf.binWidth(); }

    std::string getEffName() const { return _effModelConf.name(); }

    std::string getRespName() const { return _respModelConf.name(); }
    double getRespValidMin() const { return _respModelConf.validMin(); }
    double getRespValidMax() const { return _respModelConf.validMax(); }

    Observable (const ObservableConfig& cfg, RooWorkspace* ws) : _obsConf(cfg) {
      std::stringstream factory_cmd;

      std::cout << _obsConf.name() << std::endl;

      factory_cmd.str("");
      factory_cmd << _obsConf.name() << "[" << _obsConf.min() << ", " << _obsConf.max() << "]";
      ws->factory(factory_cmd.str().c_str());      

      ws->var(_obsConf.name().c_str())->setBins( (_obsConf.max()-_obsConf.min())/_obsConf.binWidth() );

      ws->var(_obsConf.name().c_str())->setRange("fit", _obsConf.fitMin(), _obsConf.fitMax());

      // Construct the efficiency pdf for this observable
      if (_obsConf.efficiencyModel(_effModelConf)) {
	std::cout << _effModelConf.name() << std::endl;
	FormulaConfig formulaConf;
	if (_effModelConf.formula(formulaConf)) {
	  RooArgList list; // need to keep track of all vars that go into the function
	  list.add(*ws->var(_obsConf.name().c_str())); // add the observable

	  // Create all the other parameters
	  for (const auto& i_param_cfg : formulaConf.parameters()) {
	    factory_cmd.str("");
	    factory_cmd << i_param_cfg.name() << "[";

	    double val;
	    if (i_param_cfg.value(val)) {
	      factory_cmd << val;
	    }
	    double min_val, max_val;
	    if (i_param_cfg.minValue(min_val) && i_param_cfg.maxValue(max_val)) {
	      if (i_param_cfg.value(val)) { // there was an initial value specified
		factory_cmd << ", ";
	      }
	      factory_cmd << min_val << ", " << max_val;
	    }
	    factory_cmd << "]";
	    //	    std::cout << factory_cmd.str() << std::endl;
	    ws->factory(factory_cmd.str().c_str());
	    
	    list.add(*ws->var(i_param_cfg.name().c_str()));
	  }

	  // Create the formula itself
	  ws->import(*( new RooFormulaVar(_effModelConf.name().c_str(), formulaConf.formula().c_str(), list)));
	}
      }

      // Construct the response PDF this observable
      if (_obsConf.responseModel(_respModelConf)) {
	PdfConfig pdfConf;
	if (_respModelConf.pdf(pdfConf)) {
	  factory_cmd.str("");
	  factory_cmd << pdfConf.formula();
	  ws->factory(factory_cmd.str().c_str());
	}
      }
    }
  };

  typedef std::vector<Observable> Observables;
}

#endif
