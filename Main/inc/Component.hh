#ifndef Component_hh_
#define Component_hh_

#include "RooWorkspace.h"
#include "ConfigTools/inc/SimpleConfig.hh"

#include "Main/inc/Configs.hh"
#include "Main/inc/Observable.hh"

namespace roofitter {

  typedef std::string ObsName;
  typedef std::string PdfName;

  struct FullPdfConfig {
    fhicl::Atom<std::string> obsName{fhicl::Name("obsName"), fhicl::Comment("Name of the observable that this PDF is for")};
    fhicl::Table<PdfConfig> pdf{fhicl::Name("pdf"), fhicl::Comment("PDF function in RooFit factory format")};

    fhicl::Atom<bool> incEffModel{fhicl::Name("incEffModel"), fhicl::Comment("True/false whether to include the efficiency model"), false};
    fhicl::Atom<std::string> effPdfName{fhicl::Name("effPdfName"), fhicl::Comment("Name to use for the PDF with efficiency model"), ""};

    fhicl::Atom<bool> incRespModel{fhicl::Name("incRespModel"), fhicl::Comment("True/false whether to include the response model"), false};
    fhicl::Atom<std::string> respPdfName{fhicl::Name("respPdfName"), fhicl::Comment("Name to use for the PDF with response model"), ""};

    fhicl::OptionalAtom<std::string> integrator{fhicl::Name("integrator"), fhicl::Comment("Class name for a different integrator to use")};
  };

  struct ComponentConfig {
    fhicl::Atom<std::string> name{fhicl::Name("name"), fhicl::Comment("Component name")};
    fhicl::Sequence< fhicl::Table<FullPdfConfig> > fullPdfs{fhicl::Name("fullPdfs"), fhicl::Comment("List of full PDF configurations for this component")};
  };

  class Component {
  private:
    ComponentConfig _compConf;

  public:
    std::string getName() const { return _compConf.name(); }

    Component (const ComponentConfig& cfg, RooWorkspace* ws, const Observables& observables) : _compConf(cfg) {
      std::stringstream factory_cmd;

      for (const auto& i_pdf_cfg : _compConf.fullPdfs()) {
	std::string i_obs_name = i_pdf_cfg.obsName();

	for (const auto& i_obs : observables) {
	  const auto& i_obs_cfg = i_obs.getConf();
	  
	  // Ignore any PDfs for observables we haven't created
	  if (i_obs_name != i_obs_cfg.name()) {
	    continue;
	  }
	
	  // Construct the true pdf
	  std::string pdf = i_pdf_cfg.pdf().formula();
	  factory_cmd.str("");
	  factory_cmd << pdf;
	  ws->factory(factory_cmd.str().c_str());

	  std::string currentPdfName = i_pdf_cfg.pdf().name();

	  // Create a PDF with the efficiency model, if requested
	  EffModelConfig i_eff_cfg;
	  if (i_pdf_cfg.incEffModel() && i_obs_cfg.efficiencyModel(i_eff_cfg)) {
	    
	    FormulaConfig i_eff_formula_cfg;
	    if (i_eff_cfg.formula(i_eff_formula_cfg)) {
	      ws->import(*(new RooEffProd(i_pdf_cfg.effPdfName().c_str(), "", *ws->pdf(i_pdf_cfg.pdf().name().c_str()), *ws->function(i_eff_cfg.name().c_str()))));
	    }
	    else {
	      throw cet::exception("Component Constructor") << "No function for efficiency model" << std::endl;
	    }
	    currentPdfName = i_pdf_cfg.effPdfName();
	  }

	  // Create a PDF with the response model, if requested
	  RespModelConfig i_resp_cfg;
	  if (i_pdf_cfg.incRespModel() && i_obs_cfg.responseModel(i_resp_cfg)) {
	    factory_cmd.str("");
	    factory_cmd << "FCONV::" << i_pdf_cfg.respPdfName() << "(" << i_obs_name << ", " << currentPdfName << ", " << i_resp_cfg.name() << ")";
	    ws->factory(factory_cmd.str().c_str());
	
	    ((RooFFTConvPdf*) ws->pdf(i_pdf_cfg.respPdfName().c_str()))->setBufferFraction(5.0);
	  }

	  // Set any new integrator for all the Pdfs
	  std::string new_integrator;
	  if (i_pdf_cfg.integrator(new_integrator)) {
	    RooNumIntConfig customConfig(*RooAbsReal::defaultIntegratorConfig());
	    customConfig.method1D().setLabel(new_integrator.c_str());
	    
	    if(!i_pdf_cfg.pdf().name().empty()) {
	      ws->pdf(i_pdf_cfg.pdf().name().c_str())->setIntegratorConfig(customConfig);
	    }
	    if(!i_pdf_cfg.effPdfName().empty()) {
	      ws->pdf(i_pdf_cfg.effPdfName().c_str())->setIntegratorConfig(customConfig);
	    }
	    if(!i_pdf_cfg.respPdfName().empty()) {
	      ws->pdf(i_pdf_cfg.respPdfName().c_str())->setIntegratorConfig(customConfig);
	    }
	  }
	}
      }

      /*
      // Now create 2D models from these
      if (all_obs.size()==2) {

	std::string xObs = all_obs.at(0).getName();
	std::string xPdf = "";
 	if(resPdfNames.find(xObs) != resPdfNames.end()) {
	  xPdf = resPdfNames.at(xObs);
	}
 	else if(effPdfNames.find(xObs) != effPdfNames.end()) {
	  xPdf = effPdfNames.at(xObs);
	}
 	else if(truePdfNames.find(xObs) != truePdfNames.end()) {
	  xPdf = truePdfNames.at(xObs);
	}
	else {
	  throw cet::exception("Component::constructPdfs()") << "No valid Pdf name for 2D pdf";
	}

	std::string yObs = all_obs.at(1).getName();
	std::string yPdf = "";
 	if(resPdfNames.find(yObs) != resPdfNames.end()) {
	  yPdf = resPdfNames.at(yObs);
	}
 	else if(effPdfNames.find(yObs) != effPdfNames.end()) {
	  yPdf = effPdfNames.at(yObs);
	}
 	else if(truePdfNames.find(yObs) != truePdfNames.end()) {
	  yPdf = truePdfNames.at(yObs);
	}
	else {
	  throw cet::exception("Component::constructPdfs()") << "No valid Pdf name for 2D pdf";
	}

	factory_cmd.str("");
	factory_cmd << "PROD::" << name << "2D(" << xPdf << "|" << yObs << "," << yPdf << ")";
	//	factory_cmd << "PROD::" << name << "2D(" << xPdf << "," << yPdf << ")";
	//	std::cout << factory_cmd.str() << std::endl;
	ws->factory(factory_cmd.str().c_str());
      }
      
      if (all_obs.size()>2) {
	throw cet::exception("Component::constructPdfs()") << "Do not currently support more than two observables";
      }
      */
    }

    // Returns the efficiency correction to apply to any yield
    // (i.e. it is the efficiency 
    double getEffCorrection(const Observable& obs, RooWorkspace* ws) const {

      std::string resp_pdf_name = "";
      for (const auto& i_fullPdf : _compConf.fullPdfs()) {
	if (i_fullPdf.obsName() == obs.getName()) {
	  resp_pdf_name = i_fullPdf.respPdfName();
	}
      }
      
      RooAbsPdf* this_pdf = ws->pdf(resp_pdf_name.c_str());
      RooRealVar* this_obs = ws->var(obs.getName().c_str());
      RooFormulaVar* effFunc = (RooFormulaVar*) ws->function(obs.getEffName().c_str());
      TF1* effFn = effFunc->asTF(*this_obs, RooArgList(), RooArgList());

      double result = 0;
      double min_obs = obs.getMin();
      double max_obs = obs.getMax();
      double obs_step = obs.getBinWidth();
      for (double i_obs = min_obs; i_obs < max_obs; i_obs += obs_step) {
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

    // Returns the fraction of the true pdf that has smeared out
    double getFracSmeared(const Observable& obs, RooWorkspace* ws) const {

      std::string true_pdf_name = "";
      for (const auto& i_fullPdf : _compConf.fullPdfs()) {
	if (i_fullPdf.obsName() == obs.getName()) {
	  true_pdf_name = i_fullPdf.pdf().name();
	}
      }

      RooRealVar* this_obs = ws->var(obs.getName().c_str());
      if (!this_obs) {
	throw cet::exception("Component::getFracSmeared") << "Could not find observable \"" << obs.getName() << "\" in RooWorkspace";
      }
      RooAbsPdf* truePdf = ws->pdf(true_pdf_name.c_str());
      if (!truePdf) {
	throw cet::exception("Component::getFracSmeared") << "Could not find truePdf \"" << true_pdf_name << "\" in RooWorkspace";
      }
      RooAbsPdf* respPdf = ws->pdf(obs.getRespName().c_str());
      if (!respPdf) {
	throw cet::exception("Component::getFracSmeared") << "Could not find respPdf \"" << obs.getRespName() << "\" in RooWorkspace";
      }
      double min_res = obs.getRespValidMin(); double max_res = obs.getRespValidMax();

      double result = 0;
      double min_obs = obs.getMin(); double max_obs = obs.getMax();
      double obs_step = obs.getBinWidth();

      std::cout << "getFracSmeared() might take a while..." << std::endl;
      for (double i_obs = min_obs; i_obs < max_obs; i_obs += obs_step) {
	double j_obs = i_obs + obs_step;

	std::cout << i_obs << " MeV -- " << j_obs << " MeV" << std::endl;

	// How much will be smeared out the bottom
	this_obs->setMin(min_res);
	this_obs->setMax(max_res);	    
	double min_res_smear = min_res;
	double max_res_smear = min_obs-j_obs;
	double respPdf_integral_low_val = 0;
	if (max_res_smear > min_res_smear) {
	  this_obs->setRange("resRange", min_res_smear, max_res_smear);
	  RooAbsReal* respPdf_integral_low = respPdf->createIntegral(*this_obs, RooFit::NormSet(*this_obs), RooFit::Range("resRange"));
	  respPdf_integral_low_val = respPdf_integral_low->getVal();
	  std::cout << "Res Smear Integral Low (" << min_res_smear << " MeV -- " << max_res_smear << " MeV) = " << respPdf_integral_low_val << std::endl;
	}

	// How much will be smeared out the top
	min_res_smear = max_obs-j_obs;
	max_res_smear = max_res;
	double respPdf_integral_high_val = 0;
	if (max_res_smear > min_res_smear) {
	  this_obs->setRange("resRange", min_res_smear, max_res_smear);
	  RooAbsReal* respPdf_integral_high = respPdf->createIntegral(*this_obs, RooFit::NormSet(*this_obs), RooFit::Range("resRange"));
	  respPdf_integral_high_val = respPdf_integral_high->getVal();
	  std::cout << "Res Smear Integral High (" << min_res_smear << " MeV -- " << max_res_smear << " MeV) = " << respPdf_integral_high_val << std::endl;
	}

	// Reset the limits here so that they are correct when we exit the for loop
	this_obs->setMax(max_obs);
	this_obs->setMin(min_obs);
	if (respPdf_integral_low_val>1e-4 || respPdf_integral_high_val>1e-4) { // only calculate the truth if we have to
	  // How much of the truth is here?
	  this_obs->setRange("new", i_obs, j_obs);
	  RooAbsReal* truePdf_integral = truePdf->createIntegral(*this_obs, RooFit::NormSet(*this_obs), RooFit::Range("new"));
	  double truePdf_integral_val = truePdf_integral->getVal();
	  std::cout << "True PDF Integral (" << i_obs << " MeV -- " << j_obs << " MeV) = " << truePdf_integral_val << std::endl;
	  //	if(truePdf_integral_val<1e-3) {
	  //	  break;
	  //	}
	  double smeared_away = (respPdf_integral_low_val + respPdf_integral_high_val) * truePdf_integral_val;
	  std::cout << "Fraction Smeared Away = " << smeared_away << std::endl;
	  result += smeared_away;
	}
	else {
	  std::cout << "This region won't smear in either direction" << std::endl;
	}
      }

      return result;
    }
  };
  typedef std::vector<Component> Components;
}

#endif
