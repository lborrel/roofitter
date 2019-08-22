#ifndef Analysis_hh_
#define Analysis_hh_

#include "TFile.h"
#include "TH1.h"
#include "TF1.h"

#include "RooWorkspace.h"
#include "RooDataHist.h"
#include "RooPlot.h"
#include "RooAddPdf.h"
#include "RooFFTConvPdf.h"
#include "RooRealVar.h"
#include "RooFitResult.h"
#include "RooHistPdf.h"
#include "RooEffProd.h"

#include "RooNumIntConfig.h"

#include "ConfigTools/inc/SimpleConfig.hh"

#include "Main/inc/Observable.hh"
#include "Main/inc/Component.hh"

namespace trkana {

  typedef std::string PdfName;
  typedef std::string Calculation;

  typedef std::vector<Calculation> Calculations;

  class Analysis {
  public:
    Analysis(std::string name, const mu2e::SimpleConfig& config) : 
      name(name),
      ws(new RooWorkspace(name.c_str(), true))
    {
      std::stringstream factory_cmd;
      std::vector<std::string> all_obs_names;
      config.getVectorString(name+".observables", all_obs_names);
      if (all_obs_names.size()>2) {
	throw cet::exception("trkana::Analysis") << "More than 2 observables is not currently supported";
      }

      // Construct all the observables
      for (const auto& i_obs_name : all_obs_names) {
	std::string eff_name = config.getString(name+"."+i_obs_name+".eff.name", "");
	std::string res_name = config.getString(name+"."+i_obs_name+".res.name", "");
	Observable i_obs(i_obs_name, eff_name, res_name, config, ws);
	observables.push_back(i_obs);
      }

      // Construct the components
      std::vector<std::string> all_comp_names;
      config.getVectorString(name+".components", all_comp_names);
      for (const auto& i_comp_name : all_comp_names) {
	Component i_comp(i_comp_name, config, ws);

	i_comp.constructPdfs(observables, config, ws);

	components.push_back(i_comp);
      }

      
      // Construct all the cuts
      std::vector<std::string> all_cuts;
      config.getVectorString(name+".cuts", all_cuts);
      for (const auto& i_cut : all_cuts) {
	cuts.push_back(TCut(config.getString("cut."+i_cut).c_str()));
      }


      // Construct the modelPdf
      std::string pdf = config.getString(name+".model");
      factory_cmd.str("");
      factory_cmd << pdf;
      ws->factory(factory_cmd.str().c_str());
      modelPdf = "model";

      // Construct all the calculations
      config.getVectorString(name+".calculations", calcs);
    }

    TCut cutcmd() { 
      TCut result;
      for (const auto& i_cut : cuts) {
	result += i_cut;
      }
      return result;
    }

    void fillData(TTree* tree) {

      RooArgSet vars;
      RooRealVar* x_var = 0;
      RooRealVar* y_var = 0;
      std::string x_leaf = "";
      std::string y_leaf = "";
      for (const auto& i_obs : observables) {
	RooRealVar* var = ws->var(i_obs.name.c_str());
	vars.add(*var);

	if (x_leaf.empty()) {
	  x_leaf = i_obs.leaf;
	  x_var = var;
	}
	else if (y_leaf.empty()) {
	  y_leaf = i_obs.leaf;
	  y_var = var;
	}
      }

      std::string histname = "h_" + name;
      std::string draw = "";
      if (vars.getSize()==1) {
	hist = x_var->createHistogram(histname.c_str());
	draw = x_leaf;
      }
      else if (vars.getSize()==2) {
	hist = x_var->createHistogram(histname.c_str(), RooFit::YVar(*y_var));
	draw = y_leaf+":"+x_leaf;
      }
      else {
	throw cet::exception("trkana::Analysis") << "Can't create histogram with more than two axes";
      }
      draw += ">>";
      draw += hist->GetName();

      tree->Draw(draw.c_str(), cutcmd(), "goff");

      ws->import(*(new RooDataHist("data", "data", vars, RooFit::Import(*hist))));
    }


    void fit() {
      RooAbsData* data = ws->data("data");
      fitResult = ws->pdf(modelPdf.c_str())->fitTo(*data, RooFit::Save(), RooFit::Range("fit"));
    }

    void unfold() {
      // Unfold efficiency
      // should have an efficiency function and yields of each component as function of the observable
      RooAddPdf* full_model = (RooAddPdf*) ws->pdf(modelPdf.c_str());
      size_t i_element = 0;
      for (const auto& i_comp : components) {
	//	if (!i_comp.effPdfName.empty()) {
	  RooRealVar* i_comp_yield = (RooRealVar*) full_model->coefList().at(i_element);
	  double i_comp_yield_val = i_comp_yield->getVal();
	  double i_comp_yield_err = i_comp_yield->getPropagatedError(*fitResult);

	  double effCorr = i_comp.getEffCorrection(observables.at(0), ws); //TODO: handle more than one dimension
	  double i_comp_final_yield_val = i_comp_yield_val * effCorr;
	  double i_comp_final_yield_err = (i_comp_yield_err / i_comp_yield_val) * i_comp_final_yield_val;

	  std::string new_yield_name = i_comp_yield->GetName();
	  new_yield_name += "Eff";
	  RooRealVar* i_comp_final_yield = new RooRealVar(new_yield_name.c_str(), "", i_comp_final_yield_val);
	  i_comp_final_yield->setError(i_comp_final_yield_err);
	  ws->import(*i_comp_final_yield);

	  //	  std::cout << "AE: Initial : " << i_comp_yield_val << " +/- " << i_comp_yield_err << std::endl;
	  //	  std::cout << "AE: --> Corrected : " << i_comp_final_yield_val << " +/- " << i_comp_final_yield_err << std::endl;
	  //	}

	// Calculate the fraction of the tru spectrum that has smeared out
	  //	if(!i_comp.resPdfName.empty()) {

	  double frac_smeared_away = i_comp.getFracSmeared(observables.at(0), ws); // TODO: handle more than one dimension
	  std::string frac_smeared_name = i_comp.name + "FracSmeared";
	  RooRealVar* frac_smeared = new RooRealVar(frac_smeared_name.c_str(), "", frac_smeared_away);
	  //	  unfold_eff_yield->setError(final_yield_err);
	  ws->import(*frac_smeared);
	  //	}
	++i_element;
      }
    }

    void calculate() {
      std::stringstream factory_cmd;
      for (const auto& i_calc : calcs) {
	factory_cmd.str("");
	factory_cmd << i_calc;
	ws->factory(factory_cmd.str().c_str());
      }
    }

    void Write() {
      hist->Write();
      
      fitResult->Write();

      ws->Print();
      ws->Write();
    }

    std::string name;
    std::vector<TCut> cuts;
    TH1* hist;
    RooWorkspace* ws;

    PdfName modelPdf;
    Observables observables;
    Components components;
    Calculations calcs;
    RooFitResult* fitResult;
  };
}

#endif
