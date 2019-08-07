#ifndef Analysis_hh_
#define Analysis_hh_

#include "RooDataHist.h"
#include "RooPlot.h"
#include "RooAddPdf.h"

#include "ConfigTools/inc/SimpleConfig.hh"

#include "Main/inc/Observable.hh"
#include "Main/inc/Component.hh"
#include "Main/inc/CeM.hh"

namespace TrkAnaAnalysis {

  struct Analysis {
    Analysis(std::string name, const mu2e::SimpleConfig& config) : 
      name(name)
    {
      std::vector<std::string> all_obs;
      config.getVectorString(name+".observables", all_obs);
      for (const auto& i_obs : all_obs) {
	obs.push_back(Observable(config, i_obs));
      }
      if (obs.size() == 1) {
	hist = new TH1F(histname().c_str(), "", obs.at(0).n_bins(),obs.at(0).min,obs.at(0).max);
      }
      else if (obs.size() == 2) {
	hist = new TH2F(histname().c_str(), "", obs.at(0).n_bins(),obs.at(0).min,obs.at(0).max,obs.at(1).n_bins(),obs.at(1).min,obs.at(1).max);
      }
      else {
	throw cet::exception("TrkAnaAnalysis::Analysis") << "More than 2 observables is not currently supported";
      }
      
      std::vector<std::string> all_cuts;
      config.getVectorString(name+".cuts", all_cuts);
      for (const auto& i_cut : all_cuts) {
	cuts.push_back(TCut(config.getString("cut."+i_cut).c_str()));
      }

      std::vector<std::string> all_comps;
      config.getVectorString(name+".components", all_comps);
      for (const auto& i_comp : all_comps) {
	std::string type = config.getString(i_comp+".type");
	if (type == "cem") {
	  comps.push_back(CeM(config, i_comp, obs));
	}
	else {
	  throw cet::exception("TrkAnalysis::Analysis") << "Component " << i_comp << " is unsupported";
	}
      }
      //      cem = new CeM(config, "cem", obs.at(1));
    }

    std::string histname() { return "h_" + name; }

    std::string drawcmd() {
      std::string result;
      if (obs.size() == 1) {
	result = obs.at(0).leaf + ">>" + histname();
      }
      else if (obs.size() == 2) {
	result = obs.at(1).leaf + ":" + obs.at(0).leaf + ">>" + histname();
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

    void fillData(TTree* tree) {
      tree->Draw(drawcmd().c_str(), cutcmd(), "goff");

      if (obs.size() == 1) {
	data = new RooDataHist("data", "data", RooArgSet(obs.at(0).roo_var), RooFit::Import(*hist));
      }
      else if (obs.size() == 2) {
	data = new RooDataHist("data", "data", RooArgSet(obs.at(0).roo_var, obs.at(1).roo_var), RooFit::Import(*hist));
      }
    }

    void constructModelPdf() {
//      RooArgList pdfs, norms;
//      for (auto& i_comp : comps) {
//	pdfs.add(*i_comp.pdf);
//	norms.add(RooRealVar("N", "N", 0, 200));
//      }
//      modelPdf = new RooAddPdf("model", "model", pdfs, norms);
modelPdf = comps.begin()->pdf;
    }

    void fit() {
std::cout << "AE: modeukPdf = " << modelPdf << std::endl;
std::cout <<" AE: data = " << data << std::endl;
      modelPdf->fitTo(*data);
    }

    RooPlot* plot() {
      RooPlot* result = 0;
      if (obs.size() == 2) {
	result = obs.at(1).frame();
      }
      else if (obs.size() == 1) {
	result = obs.at(0).frame();
      }
      data->plotOn(result);
      //      cem->pdf->plotOn(result);
      
      return result;
    }

    void Write() {
      hist->Write();
      data->Write();
      plot()->Write();

      TCanvas* c = new TCanvas("c", "c");
      plot()->Draw();
      c->Write();
    }

    std::string name;

    std::vector<Observable> obs;
    std::vector<TCut> cuts;

    TH1* hist;
    RooDataHist* data;

    std::vector<Component> comps;
    RooAbsPdf* modelPdf;
  };
}

#endif
