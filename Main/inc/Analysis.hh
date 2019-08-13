#ifndef Analysis_hh_
#define Analysis_hh_

#include "TFile.h"
#include "TH1.h"

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

namespace trkana {

  typedef std::string ObsName;
  typedef std::string LeafName;
  typedef std::string PdfName;
  typedef std::string Calculation;

  typedef std::vector<ObsName> ObsNames;
  typedef std::map<ObsName, LeafName> LeafNames;
  typedef std::vector<PdfName> PdfNames;
  typedef std::vector<Calculation> Calculations;

  class Analysis {
  public:
    Analysis(std::string name, const mu2e::SimpleConfig& config) : 
      name(name),
      ws(new RooWorkspace(name.c_str(), true))
    {
      std::stringstream factory_cmd;
      std::vector<std::string> all_obs;
      config.getVectorString(name+".observables", all_obs);
      if (all_obs.size()>2) {
	throw cet::exception("trkana::Analysis") << "More than 2 observables is not currently supported";
      }

      // Construct all the observables
      for (const auto& i_obs : all_obs) {
	factory_cmd.str("");
	double min = config.getDouble(i_obs+".hist.min");
	double max = config.getDouble(i_obs+".hist.max");
	factory_cmd << i_obs << "[" << min << ", " << max << "]";
	ws->factory(factory_cmd.str().c_str());

	double bin_width = config.getDouble(i_obs+".hist.bin_width");
	ws->var(i_obs.c_str())->setBins( (max-min)/bin_width );

	std::string leaf = config.getString(i_obs+".leaf");
	
	obs.push_back(i_obs);
	leaves.insert(std::pair<ObsName, LeafName>(i_obs, leaf));
      }
      
      // Construct all the cuts
      std::vector<std::string> all_cuts;
      config.getVectorString(name+".cuts", all_cuts);
      for (const auto& i_cut : all_cuts) {
	cuts.push_back(TCut(config.getString("cut."+i_cut).c_str()));
      }

      // Construct the component PDFs
      std::vector<std::string> all_comps;
      config.getVectorString(name+".components", all_comps);
      for (const auto& i_comp : all_comps) {
	for (const auto& i_obs : all_obs) {
	  std::string pdf = config.getString(i_comp+"."+i_obs+".pdf");
	  factory_cmd.str("");
	  factory_cmd << pdf;
	  ws->factory(factory_cmd.str().c_str());
	  pdfs.push_back(i_comp);

	  std::string new_integrator = config.getString(i_comp+"."+i_obs+".integrator", "");
	  if (!new_integrator.empty()) {
	    RooNumIntConfig customConfig(*RooAbsReal::defaultIntegratorConfig());
	    customConfig.method1D().setLabel(new_integrator.c_str());
	    ws->pdf(i_comp.c_str())->setIntegratorConfig(customConfig);
	  }
	}
      }

      for (const auto& i_obs : all_obs) {
	// Construct any efficiency pdfs
	std::string currentPdfSuffix = "";
	std::string eff_name = config.getString(name+"."+i_obs+".eff.name", "");
	if (!eff_name.empty()) {
	  std::string eff_type = config.getString(i_obs+".eff."+eff_name+".type");
	  
	  if (eff_type == "func") {
	    std::vector<std::string> eff_params;
	    config.getVectorString(i_obs+".eff."+eff_name+".func.params", eff_params);
	    
	    RooArgList list;
	    list.add(*ws->var(i_obs.c_str()));
	    for (const auto i_param : eff_params) {
	      factory_cmd.str("");
	      factory_cmd << i_param << "[" << config.getDouble(i_obs+".eff."+eff_name+".func."+i_param) << "]";
	      ws->factory(factory_cmd.str().c_str());

	      list.add(*ws->var(i_param.c_str()));
	    }
	    std::string eff_func = config.getString(i_obs+".eff."+eff_name+".func");
	    ws->import(*( new RooFormulaVar("effFunc", eff_func.c_str(), list)));

	    std::string this_suffix = "Eff";
	    for (const auto& i_comp : all_comps) {
	      std::string currentPdfName = i_comp + currentPdfSuffix;
	      std::string pdfName = currentPdfName + this_suffix;
	      
	      ws->import(*(new RooEffProd(pdfName.c_str(), "", *ws->pdf(currentPdfName.c_str()), *ws->function("effFunc"))));

	      std::string new_integrator = config.getString(i_comp+"."+i_obs+".integrator", "");
	      if (!new_integrator.empty()) {
		RooNumIntConfig customConfig(*RooAbsReal::defaultIntegratorConfig());
		customConfig.method1D().setLabel(new_integrator.c_str());
		ws->pdf(pdfName.c_str())->setIntegratorConfig(customConfig);
	      }
	    }
	    currentPdfSuffix += this_suffix;
	    
	  }
	  else {
	    throw cet::exception("Analysis") << "Unsupported efficiency type: " << eff_type;
	  }
	}

	// Construct te resolution PDFs and the smeared PDFs
	std::string res_name = config.getString(name+"."+i_obs+".res.name", "");
	if (!res_name.empty()) {
	  std::string res_type = config.getString(i_obs+".res."+res_name+".type");
	  
	  if (res_type == "pdf") {
	    std::string res_pdf = config.getString(i_obs+".res."+res_name+".pdf");
	    factory_cmd.str("");
	    factory_cmd << res_pdf;
	    ws->factory(factory_cmd.str().c_str());
	  }
	  else if (res_type == "hist") {
	    std::string res_filename = config.getString(i_obs+"res."+res_name+".hist.file");
	    std::string res_histname = config.getString(i_obs+"res."+res_name+".hist.name");
	    
	    TFile* res_file = new TFile(res_filename.c_str(), "READ");
	    if (res_file->IsZombie()) {
	      throw cet::exception("Analysis::Analsysis") << "Resolution file " << res_filename << " is a zombie";
	    }
	    TH1* res_hist = (TH1*) res_file->Get(res_histname.c_str());
	    if (!res_hist) {
	      throw cet::exception("Analysis::Analysis") << "Resolution histogram " << res_histname << " is not in file";
	    }

	    ws->import(*(new RooDataHist("res_hist", "res_hist", *ws->var(i_obs.c_str()), RooFit::Import(*res_hist))));
	    ws->import(*(new RooHistPdf("res", "", *ws->var(i_obs.c_str()), *((RooDataHist*)ws->data("res_hist")))));
	  }
	  else {
	    throw cet::exception("Analysis") << "Unsupported resolution type: " << res_type;
	  }

	  resolutionPdf = "res";
	  std::string this_suffix = "Res";
	  for (const auto& i_comp : all_comps) {
	    std::string currentPdfName = i_comp + currentPdfSuffix;
	    std::string pdfName = currentPdfName + this_suffix;
	    factory_cmd.str("");
	    factory_cmd << "FCONV::" << pdfName << "(" << i_obs << ", " << currentPdfName << ", " << resolutionPdf << ")";
	    ws->factory(factory_cmd.str().c_str());
	    
	    ((RooFFTConvPdf*) ws->pdf(pdfName.c_str()))->setBufferFraction(5.0);

	    std::string new_integrator = config.getString(i_comp+"."+i_obs+".integrator", "");
	    if (!new_integrator.empty()) {
	      RooNumIntConfig customConfig(*RooAbsReal::defaultIntegratorConfig());
	      customConfig.method1D().setLabel(new_integrator.c_str());
	      ws->pdf(pdfName.c_str())->setIntegratorConfig(customConfig);
	    }
	  }
	  currentPdfSuffix += this_suffix;
	}

	// Construct the modelPdf
	std::string pdf = config.getString(name+".model");
	factory_cmd.str("");
	factory_cmd << pdf;
	ws->factory(factory_cmd.str().c_str());
	modelPdf = "model";

	//	if (!eff_name.empty()){
	//	  ws->import(*(new RooEffProd("modelWEff", "", *ws->pdf(modelPdf.c_str()), *ws->function("effFunc"))));
	//	  modelPdf = "modelWEff";
	//	}
      }
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
      for (const auto& i_obs : obs) {
	RooRealVar* var = ws->var(i_obs.c_str());
	vars.add(*var);

	if (x_leaf.empty()) {
	  x_leaf = leaves.at(i_obs);
	  x_var = var;
	}
	else if (y_leaf.empty()) {
	  y_leaf = leaves.at(i_obs);
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
      fitResult = ws->pdf(modelPdf.c_str())->fitTo(*data, RooFit::Save());
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
    ObsNames obs;
    LeafNames leaves;
    PdfNames pdfs;
    PdfName resolutionPdf;
    PdfName modelPdf;
    Calculations calcs;
    RooFitResult* fitResult;
  };
}

#endif
