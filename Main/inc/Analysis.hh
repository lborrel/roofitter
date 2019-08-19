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
	std::string currentPdfSuffix = "";

	// Construct any efficiency pdfs
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

	double min_fit_range = config.getDouble(i_obs+".fit.min");
	double max_fit_range = config.getDouble(i_obs+".fit.max");
	ws->var(i_obs.c_str())->setRange("fit", min_fit_range, max_fit_range);

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
      fitResult = ws->pdf(modelPdf.c_str())->fitTo(*data, RooFit::Save(), RooFit::Range("fit"));
    }

    void unfold() {
      // Unfold efficiency
      // should have an efficiency function, yields of each component as functions of momentum
      RooAddPdf* full_model = (RooAddPdf*) ws->pdf(modelPdf.c_str());
      for (const auto& i_obs : obs) {
	RooRealVar* obs = ws->var(i_obs.c_str());
	double min_obs = obs->getMin();
	double max_obs = obs->getMax();
	double obs_step = (max_obs - min_obs) / obs->getBins();

	RooFormulaVar* effFunc = (RooFormulaVar*) ws->function("effFunc");
	TF1* effFn = effFunc->asTF(*obs, RooArgList(), RooArgList());

	int i_comp = 0;
	for (const auto& i_pdf : pdfs) {
	  std::string pdfname = i_pdf + "EffRes";
	  RooAbsPdf* comp_pdf = ws->pdf(pdfname.c_str());

	  RooRealVar* this_yield = (RooRealVar*) full_model->coefList().at(i_comp);
	  double this_yield_val = this_yield->getVal();
	  double this_yield_error = this_yield->getPropagatedError(*fitResult);

	  double final_yield = 0;
	  for (double i_obs = min_obs; i_obs < max_obs; i_obs += obs_step) {
	    double j_obs = i_obs + obs_step;
	    
	    obs->setRange("range", i_obs, j_obs);
	    
	    double i_eff = effFn->Eval(i_obs);
	    std::cout << "Eff @ " << i_obs << " MeV = " << i_eff << std::endl;
	    
	    RooAbsReal* comp_pdf_integral = comp_pdf->createIntegral(*obs, RooFit::NormSet(*obs), RooFit::Range("range"));
	    double comp_pdf_integral_val = comp_pdf_integral->getVal();
	    std::cout << "Comp_Pdf Integral " << i_obs << "--" << j_obs << " MeV = " << comp_pdf_integral_val;
	    std::cout << "This Yield = " << this_yield_val << " +/- " << this_yield_error << std::endl;
	    double i_yield = this_yield_val * comp_pdf_integral_val;
	    double i_yield_err = (this_yield_error / this_yield_val) * i_yield;
	    double i_total_yield = i_yield / i_eff;
	    double i_total_yield_err = (i_yield_err / i_yield) * i_total_yield;
	    std::cout << "Comp_Pdf Integral " << i_obs << "--" << j_obs << " MeV * Yield = " << i_yield << " +/- " << i_yield_err << std::endl;
	    std::cout << "Accounting for eff = " << i_total_yield << " +/- " << i_total_yield_err << std::endl;
	    final_yield += i_total_yield;
	  }
	  double final_yield_err = (this_yield_error / this_yield_val) * final_yield;
	  std::cout << "Final NYield = " << final_yield << " +/- " << final_yield_err << std::endl;
	  //	  std::cout << "Final Total NYield = " << final_total_yield << std::endl;
	  
	  std::string new_yield_name = this_yield->GetName();
	  new_yield_name += "Eff";
	  RooRealVar* unfold_eff_yield = new RooRealVar(new_yield_name.c_str(), "", final_yield);
	  unfold_eff_yield->setError(final_yield_err);
	  ws->import(*unfold_eff_yield);

	  
	  RooAbsPdf* truePdf = ws->pdf(i_pdf.c_str());
	  RooAbsPdf* resPdf = ws->pdf(resolutionPdf.c_str());
	  double total_fraction_smeared_away = 0;
	  double min_res = -2; double max_res = 5;
	  for (double i_obs = min_obs; i_obs < max_obs; i_obs += obs_step) {
	    double j_obs = i_obs + obs_step;

	    obs->setMax(max_obs);	    
	    obs->setMin(min_obs);
	    obs->setRange("new", i_obs, j_obs);
	    RooAbsReal* truePdf_integral = truePdf->createIntegral(*obs, RooFit::NormSet(*obs), RooFit::Range("new"));
	    double truePdf_integral_val = truePdf_integral->getVal();
	    std::cout << "AE: truePdf_integral_val (" << i_obs << " MeV -- " << j_obs << " MeV) = " << truePdf_integral_val << std::endl;
	    if(truePdf_integral_val<1e-3) {
	      break;
	    }

	    obs->setMin(min_res);
	    obs->setMax(max_res);	    
	    double min_res_smear = min_res;
	    double max_res_smear = min_obs-j_obs;
	    if (max_res_smear < min_res_smear) {
	      break;
	    }
	    obs->setRange("resRange", min_res_smear, max_res_smear);

	    RooAbsReal* resPdf_integral = resPdf->createIntegral(*obs, RooFit::NormSet(*obs), RooFit::Range("resRange"));
	    double resPdf_integral_val = resPdf_integral->getVal();
	    std::cout << "AE: resPdf_integral_val (" << min_res_smear << " MeV -- " << max_res_smear << " MeV) = " << resPdf_integral_val << std::endl;

	    double smeared_away = resPdf_integral_val * truePdf_integral_val;
	    std::cout << "Fraction Smeared Away = " << smeared_away << std::endl;
	    total_fraction_smeared_away += smeared_away;
	  }
	  std::cout << "Total Fraction Smeared Away = " << total_fraction_smeared_away << std::endl;
	  std::string frac_smeared_name = i_pdf + "FracSmeared";
	  RooRealVar* frac_smeared = new RooRealVar(frac_smeared_name.c_str(), "", total_fraction_smeared_away);
	  //	  unfold_eff_yield->setError(final_yield_err);
	  ws->import(*frac_smeared);

	  // Clean up
	  obs->setMax(max_obs);	    
	  obs->setMin(min_obs);
	  
	  ++i_comp;
	}
      }

      // Unfold resolution?
      // just calculate the fraction of the truth that will have smeared outside of the bounds of the histogram
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
