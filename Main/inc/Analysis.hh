#ifndef Analysis_hh_
#define Analysis_hh_

#include "TFile.h"
#include "TH1.h"
#include "TF1.h"
#include "TLeaf.h"
#include "TTreeFormula.h"

#include "RooWorkspace.h"
#include "RooDataHist.h"
#include "RooDataSet.h"
#include "RooPlot.h"
#include "RooAddPdf.h"
#include "RooFFTConvPdf.h"
#include "RooRealVar.h"
#include "RooFitResult.h"
#include "RooHistPdf.h"
#include "RooEffProd.h"

#include "RooNumIntConfig.h"

#include "ConfigTools/inc/SimpleConfig.hh"

#include "Main/inc/Configs.hh"
#include "Main/inc/Observable.hh"
#include "Main/inc/Component.hh"

#include <string>

namespace roofitter {

  typedef std::string PdfName;
  typedef std::string Calculation;

  typedef std::vector<Calculation> Calculations;

  struct CutConfig {
    fhicl::Atom<std::string> name{fhicl::Name("name"), fhicl::Comment("Cut name")};
    fhicl::Atom<std::string> leaf{fhicl::Name("leaf"), fhicl::Comment("Leaf of this tree cut")};
    fhicl::Atom<bool> invert{fhicl::Name("invert"), fhicl::Comment("Set to true if you want to invert the cut"), false};
  };

  struct AnalysisConfig {
    fhicl::Atom<std::string> name{fhicl::Name("name"), fhicl::Comment("Analysis name")};
    fhicl::Atom<std::string> fit_type{fhicl::Name("fit_type"), fhicl::Comment("Choose between binned and unbinned fit")};
    fhicl::Atom<std::string> flat_tree_filename{fhicl::Name("flat_tree_filename"), fhicl::Comment("Name of the ROOT file containing the flat tree data")};
    fhicl::Sequence< fhicl::Table<ObservableConfig> > observables{fhicl::Name("observables"), fhicl::Comment("List of observables")};
    fhicl::Sequence< fhicl::Table<ComponentConfig> > components{fhicl::Name("components"), fhicl::Comment("List of components")};
    fhicl::Sequence< fhicl::Table<CutConfig> > cuts{fhicl::Name("cuts"), fhicl::Comment("List of cuts to apply")};
    fhicl::Table<PdfConfig> model{fhicl::Name("model"), fhicl::Comment("The PDF for the full final model to fit")};
    fhicl::Atom<bool> unfold{fhicl::Name("unfold"), fhicl::Comment("Set to tru if you want to unfold the efficiency and response effects"), false};
    fhicl::Atom<bool> allow_failure{fhicl::Name("allow_failure"), fhicl::Comment("If set to true, then roofitter will not throw an exception for a failed fit."), false};
    fhicl::Sequence<std::string> calculations{fhicl::Name("calculations"), fhicl::Comment("A list of supplemental calculations that you want to calculate"), std::vector<std::string>()};
  };

  class Analysis {
  private:
    AnalysisConfig _anaConf;
    RooWorkspace* _ws;

    Observables _observables;
    Components _components;

    TH1* _hist;

    RooFitResult* _fitResult;

  public:
    Analysis(const AnalysisConfig& cfg) : 
      _anaConf(cfg),
      _ws(new RooWorkspace(_anaConf.name().c_str(), true))
    {
      std::cout << _anaConf.name() << std::endl;

      // Construct the observables
      if (_anaConf.observables().size() > 2) {
	throw cet::exception("Analysis Constructor") << "More than 2 observables is not currently supported";
      }
      for (const auto& i_obs_cfg : _anaConf.observables()) {
	Observable i_obs(i_obs_cfg, _ws);
	_observables.push_back(i_obs);
      }

      // Construct the components
      for (const auto& i_comp_cfg : _anaConf.components()) {
	Component i_comp(i_comp_cfg, _ws, _observables);
	_components.push_back(i_comp);
      }

      // Construct the final model
      std::stringstream factory_cmd;
      factory_cmd.str("");
      factory_cmd << _anaConf.model().formula();
      _ws->factory(factory_cmd.str().c_str());
    }

    TCut cutcmd() { 
      TCut result;
      for (const auto& i_cut_cfg : _anaConf.cuts()) {
	if (i_cut_cfg.invert()) {
	  result += !TCut(i_cut_cfg.leaf().c_str());
	}
	else {
	  result += TCut(i_cut_cfg.leaf().c_str());
	}
      }
      return result;
    }

    TTree* flattenTree(TTree* tree)
    {
        std::vector<std::string> branchleaf;
        std::vector<Float_t> vars(_observables.size());
        std::vector<TLeaf *> leaves;
        TFile *file_flat = new TFile(_anaConf.flat_tree_filename().c_str(), "RECREATE");
        TTree *tree_flat = new TTree("trkana_flat", "flatten trkana tree");

        std::cout << "# observables: " << _observables.size() << std::endl;
        for (unsigned int i_obs = 0; i_obs < _observables.size(); ++i_obs)
        {
            auto& obs_conf = _observables[i_obs].getConf();
            branchleaf.push_back(obs_conf.leaf());
            std::string branch_name, leaf_name;
            // Replace the "." by a "/" in the string to create the leaf later; also extract the name of the branch and the name of the leaf
            for (unsigned int c = 0; c < branchleaf[i_obs].length(); ++c)
            {
                if (branchleaf[i_obs][c] == '.')
                {
                    branchleaf[i_obs][c] = '/';
                    branch_name = branchleaf[i_obs].substr(0, c);
                    leaf_name = branchleaf[i_obs].substr(c+1, branchleaf[i_obs].length());
                }
            }

            std::cout << "Leaf name: " << leaf_name << std::endl;
            tree_flat->Branch(leaf_name.c_str(), &vars[i_obs]);
            leaves.push_back(tree->GetLeaf(branchleaf[i_obs].c_str()));
        }

        // Formula for the cut to apply on the tree
        TTreeFormula *cut_formula = new TTreeFormula("cut", cutcmd(), tree);

        Int_t n_entries = (Int_t) tree->GetEntries();
        for (Int_t i = 0; i < n_entries; i++)
        {
            for (unsigned int i_obs = 0; i_obs < _observables.size(); ++i_obs)
            {
                tree->GetEntry(i);
                if (cut_formula->EvalInstance() == 1)
                {
                    vars[i_obs] = leaves[i_obs]->GetValue();
                    tree_flat->Fill();
                }
            }
        }

        tree_flat->Write();
        file_flat->Write();

        return tree_flat;
    }

    void fillData(TTree* tree) {

    RooArgSet vars;
    RooRealVar* x_var = 0;
    RooRealVar* y_var = 0;
    std::string x_leaf = "";
    std::string y_leaf = "";
    for (const auto& i_obs : _observables) {
        const auto& i_obs_conf = i_obs.getConf();
	    RooRealVar* var = _ws->var(i_obs_conf.name().c_str());
	    vars.add(*var);

	    if (x_leaf.empty()) {
	        x_leaf = i_obs_conf.leaf();
	        x_var = var;
	    }
	    else if (y_leaf.empty()) {
	        y_leaf = i_obs_conf.leaf();
	    y_var = var;
	    }
    }

    if (_anaConf.fit_type() == "binned")
    {
        std::string histname = "h_" + _anaConf.name();
        std::string draw = "";
        if (vars.getSize()==1) {
	        _hist = x_var->createHistogram(histname.c_str());
	        draw = x_leaf;
        }
        else if (vars.getSize()==2) {
	        _hist = x_var->createHistogram(histname.c_str(), RooFit::YVar(*y_var));
	        draw = y_leaf+":"+x_leaf;
        }
        else {
            throw cet::exception("Analysis::fillData()") << "Can't create histogram with more than two axes";
        }
        draw += ">>";
        draw += _hist->GetName();

        tree->Draw(draw.c_str(), cutcmd(), "goff");

        _ws->import(*(new RooDataHist("data", "data", vars, RooFit::Import(*_hist))));
    }
    else if (_anaConf.fit_type() == "unbinned")
    {
        TFile *file_flat = new TFile(_anaConf.flat_tree_filename().c_str());
        TTree *flat_tree = (TTree*) file_flat->Get("trkana_flat");

        RooDataSet data("data", "unbinned dataset", flat_tree, vars);
        _ws->import(data);
    }
    else
    {
        throw cet::exception("Analysis::fillData()") << "incorrect fit_type. Only \"binned\" and \"unbinned\" are currently supported";
    }
}


    void fit() {

      RooAbsData *data;
      data = _ws->data("data");

      RooAbsPdf* model = _ws->pdf(_anaConf.model().name().c_str());
      if (!model) {
	throw cet::exception("Analysis::fit()") << "Can't find model \"" << _anaConf.model().name() << "\" in RooWorkspace";
      }
      _fitResult = model->fitTo(*data, RooFit::Save(), RooFit::Range("fit"), RooFit::Extended(true));
      _fitResult->printValue(std::cout);

      int status = _fitResult->status();
      if (status>0) {
	if (!_anaConf.allow_failure()) {
	  throw cet::exception("Analysis::fitTo()") << "Fit failed! If you want roofitter to continue and not throw this exception then set allow_failure to true in your fcl file";
	}
      }
    }

    void unfold() {
      if (_anaConf.unfold()) {
	// Unfold efficiency
	// should have an efficiency function and yields of each component as function of the observable
	RooAddPdf* full_model = (RooAddPdf*) _ws->pdf(_anaConf.model().name().c_str());
	if (!full_model) {
	  throw cet::exception("Analysis::unfold()") << "Can't find model \"" << _anaConf.model().name() << "\" in RooWorkspace";
	}

	size_t i_element = 0;
	for (const auto& i_comp : _components) {
	  //	if (!i_comp.effPdfName.empty()) {
	  RooRealVar* i_comp_yield = (RooRealVar*) full_model->coefList().at(i_element);
	  double i_comp_yield_val = i_comp_yield->getVal();
	  double i_comp_yield_err = i_comp_yield->getPropagatedError(*_fitResult);
	  
	  double effCorr = i_comp.getEffCorrection(_observables.at(0), _ws); //TODO: handle more than one dimension
	  double i_comp_final_yield_val = i_comp_yield_val * effCorr;
	  double i_comp_final_yield_err = (i_comp_yield_err / i_comp_yield_val) * i_comp_final_yield_val;
	  
	  std::string new_yield_name = i_comp_yield->GetName();
	  new_yield_name += "Eff";
	  RooRealVar* i_comp_final_yield = new RooRealVar(new_yield_name.c_str(), "", i_comp_final_yield_val);
	  i_comp_final_yield->setError(i_comp_final_yield_err);
	  _ws->import(*i_comp_final_yield);

	  // Calculate the fraction of the tru spectrum that has smeared out

	  double frac_smeared_away = i_comp.getFracSmeared(_observables.at(0), _ws); // TODO: handle more than one dimension
	  std::string frac_smeared_name = i_comp.getName() + "FracSmeared";
	  RooRealVar* frac_smeared = new RooRealVar(frac_smeared_name.c_str(), "", frac_smeared_away);
	  //	  unfold_eff_yield->setError(final_yield_err);
	  _ws->import(*frac_smeared);
	  //	}
	  ++i_element;
	}
      }
    }

    void calculate() {
      std::stringstream factory_cmd;
      for (const auto& i_calc : _anaConf.calculations()) {
	factory_cmd.str("");
	factory_cmd << i_calc;
	_ws->factory(factory_cmd.str().c_str());
      }
    }

    void Write() {
      if (_anaConf.fit_type() == "binned")
      {
        _hist->Write();
      }
      
      _fitResult->Write();

      _ws->Print();
      _ws->Write();
    }

    const AnalysisConfig& getConf() const { return _anaConf; }
  };
}

#endif
