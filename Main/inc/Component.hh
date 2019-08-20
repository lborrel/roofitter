#ifndef Component_hh_
#define Component_hh_

#include "RooWorkspace.h"
#include "ConfigTools/inc/SimpleConfig.hh"

#include "Main/inc/Observable.hh"

namespace trkana {

  struct Component {

    Component(const std::string& comp_name, const mu2e::SimpleConfig& config, RooWorkspace* ws) : name(comp_name), effPdfName(""), resPdfName("") {
    }

    void constructPdfs(const Observable& obs, const mu2e::SimpleConfig& config, RooWorkspace* ws) {
      std::stringstream factory_cmd;

      // Construct the true Pdf
      std::string pdf = config.getString(name+"."+obs.name+".pdf");

      factory_cmd.str("");
      factory_cmd << pdf;
      ws->factory(factory_cmd.str().c_str());

      // Construct the Pdf with Eff, if possible
      std::string currentPdfSuffix = "";
      if(!obs.effName.empty()) {
	std::string this_suffix = "Eff";

	std::string currentPdfName = name + currentPdfSuffix;
	effPdfName = currentPdfName + this_suffix;
	    
	// Construct the Pdfs that include efficiency
	ws->import(*(new RooEffProd(effPdfName.c_str(), "", *ws->pdf(currentPdfName.c_str()), *ws->function(obs.effName.c_str()))));
	  
	currentPdfSuffix += this_suffix;
      }

      // Construct the smeared pdf
      if (!obs.resName.empty()) {
	std::string this_suffix = "Res";
	std::string currentPdfName = name + currentPdfSuffix;
	resPdfName = currentPdfName + this_suffix;
	factory_cmd.str("");
	factory_cmd << "FCONV::" << resPdfName << "(" << obs.name << ", " << currentPdfName << ", " << obs.resName << ")";
	ws->factory(factory_cmd.str().c_str());
	
	((RooFFTConvPdf*) ws->pdf(resPdfName.c_str()))->setBufferFraction(5.0);

	currentPdfSuffix += this_suffix;
      }

      // Set any new integrator for all the Pdfs
      std::string new_integrator = config.getString(name+"."+obs.name+".integrator", "");
      if (!new_integrator.empty()) {
	RooNumIntConfig customConfig(*RooAbsReal::defaultIntegratorConfig());
	customConfig.method1D().setLabel(new_integrator.c_str());

	if(!name.empty()) {
	  ws->pdf(name.c_str())->setIntegratorConfig(customConfig);
	}
	if(!effPdfName.empty()) {
	  ws->pdf(effPdfName.c_str())->setIntegratorConfig(customConfig);
	}
	if(!resPdfName.empty()) {
	  ws->pdf(resPdfName.c_str())->setIntegratorConfig(customConfig);
	}
      }
    }

    // Returns the efficiency correction to apply to any yield
    // (i.e. it is the efficiency 
    double getEffCorrection(const Observable& obs, RooWorkspace* ws) const {

      if (effPdfName.empty()) {
	throw cet::exception("Component::getEffCorrection") << "Trying to correct for efficiency when effPdfName is empty";
      }

      RooAbsPdf* this_pdf = ws->pdf(resPdfName.c_str());
      RooRealVar* this_obs = ws->var(obs.name.c_str());
      RooFormulaVar* effFunc = (RooFormulaVar*) ws->function(obs.effName.c_str());
      TF1* effFn = effFunc->asTF(*this_obs, RooArgList(), RooArgList());

      double result = 0;
      double obs_step = obs.hist_bin_width;
      for (double i_obs = obs.hist_min; i_obs < obs.hist_max; i_obs += obs_step) {
	double j_obs = i_obs + obs_step;
	    
	this_obs->setRange("range", i_obs, j_obs);
	    
	double i_eff = effFn->Eval(i_obs);
	//	std::cout << "Eff @ " << i_obs << " MeV = " << i_eff << std::endl;

	RooAbsReal* pdf_integral = this_pdf->createIntegral(*this_obs, RooFit::NormSet(*this_obs), RooFit::Range("range"));
	double i_pdf = pdf_integral->getVal();
	//	std::cout << effPdfName << " @ " << i_obs << " MeV = " << i_pdf << std::endl;
	double i_effCorr = i_pdf / i_eff;
	result += i_effCorr;
      }

      return result;
    }

    std::string name;
    std::string effPdfName;
    std::string resPdfName;
  };

}

#endif
