#ifndef Analysis_hh_
#define Analysis_hh_

#include "RooWorkspace.h"
#include "RooDataHist.h"
#include "RooPlot.h"
#include "RooAddPdf.h"
#include "RooFFTConvPdf.h"
#include "RooRealVar.h"

#include "ConfigTools/inc/SimpleConfig.hh"

namespace TrkAnaAnalysis {

  typedef std::string ObsName;
  typedef std::string LeafName;
  typedef std::string PdfName;

  typedef std::vector<ObsName> ObsNames;
  typedef std::map<ObsName, LeafName> LeafNames;
  typedef std::vector<PdfName> PdfNames;

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
	throw cet::exception("TrkAnaAnalysis::Analysis") << "More than 2 observables is not currently supported";
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

      if (config.getBool(name+".fit", false)) {
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
	  }
	}

	// Construct te resolution PDFs and the smeared PDFs
	for (const auto& i_obs : all_obs) {
	  std::string res_pdf = config.getString("res."+i_obs+".pdf");
	  factory_cmd.str("");
	  factory_cmd << res_pdf;
	  ws->factory(factory_cmd.str().c_str());
	  resolutionPdf = "res";

	  for (const auto& i_comp : all_comps) {
	    std::string pdfName = i_comp + "res";
	    factory_cmd.str("");
	    factory_cmd << "FCONV::" << pdfName << "(" << i_obs << ", " << i_comp << ", " << resolutionPdf << ")";
	    ws->factory(factory_cmd.str().c_str());

	    ((RooFFTConvPdf*) ws->pdf(pdfName.c_str()))->setBufferFraction(2.0);
	  }
	}

	// Construct the modelPdf
	std::string pdf = config.getString(name+".model");
	factory_cmd.str("");
	factory_cmd << pdf;
	ws->factory(factory_cmd.str().c_str());
	modelPdf = "model";
      }
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
	throw cet::exception("TrkAnaAnalysis::Analysis") << "Can't create histogram with more than two axes";
      }
      draw += ">>";
      draw += hist->GetName();

      tree->Draw(draw.c_str(), cutcmd(), "goff");

      ws->import(*(new RooDataHist("data", "data", vars, RooFit::Import(*hist))));
    }


    void fit() {
      RooAbsData* data = ws->data("data");
      ws->pdf(modelPdf.c_str())->fitTo(*data);
    }

    RooPlot* plot(std::string obs_x) {

      RooRealVar* var = ws->var(obs_x.c_str());
      RooPlot* result = var->frame();

      ws->data("data")->plotOn(result);

      RooAbsPdf* plotPdf = ws->pdf(modelPdf.c_str());
      if (!plotPdf) {
	throw cet::exception("TrkAnalysis::Analsysi::plot") << "Problem getting pdf " << modelPdf;
      }
      plotPdf->plotOn(result);
      
      return result;
    }

    void Write() {
      hist->Write();

      TCanvas* c = new TCanvas("c", "c");
      plot("mom")->Draw();
      c->Write();

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
  };
}

#endif
