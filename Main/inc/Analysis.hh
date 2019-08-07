#ifndef Analysis_hh_
#define Analysis_hh_

#include "RooWorkspace.h"
#include "RooDataHist.h"
#include "RooPlot.h"
#include "RooAddPdf.h"
#include "RooRealVar.h"

#include "ConfigTools/inc/SimpleConfig.hh"

namespace TrkAnaAnalysis {

  typedef std::string ObsName;
  typedef std::string LeafName;
  typedef std::string PdfName;

  typedef std::map<ObsName, LeafName> LeafNames;
  typedef std::vector<PdfName> PdfNames;

  struct Analysis {
    Analysis(std::string name, const mu2e::SimpleConfig& config) : 
      name(name),
      ws(RooWorkspace(name.c_str(), true))
    {
      std::stringstream factory_cmd;
      std::vector<std::string> all_obs;
      config.getVectorString(name+".observables", all_obs);
      if (all_obs.size()>2) {
	throw cet::exception("TrkAnaAnalysis::Analysis") << "More than 2 observables is not currently supported";
      }

      for (const auto& i_obs : all_obs) {
	factory_cmd.str("");
	double min = config.getDouble(i_obs+".hist.min");
	double max = config.getDouble(i_obs+".hist.max");
	factory_cmd << i_obs << "[" << min << ", " << max << "]";
	ws.factory(factory_cmd.str().c_str());

	double bin_width = config.getDouble(i_obs+".hist.bin_width");
	ws.var(i_obs.c_str())->setBins( (max-min)/bin_width );

	std::string leaf = config.getString(i_obs+".leaf");
	leaves.insert(std::pair<ObsName, LeafName>(i_obs, leaf));
	//	ws.Print();
      }
      
      std::vector<std::string> all_cuts;
      config.getVectorString(name+".cuts", all_cuts);
      for (const auto& i_cut : all_cuts) {
	cuts.push_back(TCut(config.getString("cut."+i_cut).c_str()));
      }

      
      std::vector<std::string> all_comps;
      config.getVectorString(name+".components", all_comps);
      for (const auto& i_comp : all_comps) {
	for (const auto& i_obs : all_obs) {
	std::string pdf = config.getString(i_comp+"."+i_obs+".pdf");
	factory_cmd.str("");
	factory_cmd << pdf;
	ws.factory(factory_cmd.str().c_str());
	pdfs.push_back(i_comp);
	}
      }
    }

    std::string histname() { return "h_" + name; }

    std::string drawcmd(std::string obs_x, std::string obs_y = "") {
      std::string result;
      std::string leaf_x = leaves.at(obs_x);
      std::string leaf_y = "";
      if (!obs_y.empty()) {
	leaf_y = leaves.at(obs_y);
      }
      if (leaf_y.empty()) {
	result = leaf_x;
      }
      else {
	result = leaf_y + ":" + leaf_x;
      }
      return result;
    }

    TCut cutcmd() { 
      TCut result;
      for (const auto& i_cut : cuts) {
	result += i_cut;
      }
      return result;
    }

    void fillData(TTree* tree, std::string obs_x, std::string obs_y = "") {

      RooArgSet vars;
      RooRealVar* x_var = ws.var(obs_x.c_str());
      vars.add(*x_var);

      RooRealVar* y_var = 0;
      if (!obs_y.empty()) {
	y_var = ws.var(obs_y.c_str());
	vars.add(*y_var);
      }	

      if (vars.getSize()==1) {
	hist = x_var->createHistogram(histname().c_str());
      }
      else if (vars.getSize()==2) {
	hist = x_var->createHistogram(histname().c_str(), RooFit::YVar(*y_var));
      }
      else {
	throw cet::exception("TrkAnaAnalysis::Analysis") << "Can't create histogram with more than two axes";
      }

      std::string draw = drawcmd(obs_x, obs_y) + ">>" + hist->GetName();
      tree->Draw(draw.c_str(), cutcmd(), "goff");

      data = new RooDataHist("data", "data", vars, RooFit::Import(*hist));
    }

    void constructModelPdf() {
//      RooArgList pdfs, norms;
//      for (auto& i_comp : comps) {
//	pdfs.add(*i_comp.pdf);
//	norms.add(RooRealVar("N", "N", 0, 200));
//      }
//      modelPdf = new RooAddPdf("model", "model", pdfs, norms);
      modelPdf = ws.pdf(pdfs.begin()->c_str());
    }

    void fit() {
      modelPdf->fitTo(*data);
    }

    RooPlot* plot(std::string obs_x) {

      RooRealVar* var = ws.var(obs_x.c_str());
      RooPlot* result = 0;
      result = var->frame();

      data->plotOn(result);
      modelPdf->plotOn(result);
      
      return result;
    }

    void Write() {
      hist->Write();
      data->Write();
      plot("mom")->Write();

      TCanvas* c = new TCanvas("c", "c");
      plot("mom")->Draw();
      c->Write();

      ws.Print();
    }

    std::string name;

    std::vector<TCut> cuts;

    TH1* hist;
    RooDataHist* data;

    RooAbsPdf* modelPdf;

    RooWorkspace ws;
    LeafNames leaves;
    PdfNames pdfs;
  };
}

#endif
