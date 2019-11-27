void plot_cemDio_mom_true() {

  std::string filename = "ana7.root";
  TFile* file = new TFile(filename.c_str(), "READ");
  
  RooWorkspace* ws = (RooWorkspace*) file->Get("cemDio_mom_true/cemDio_mom_true");

  ws->Print();

  RooRealVar* mom = ws->var("mom");
  RooPlot* plot = mom->frame(RooFit::Range("fit"));
  
  RooAbsData* data = ws->data("data");
  data->plotOn(plot);
  
  RooAbsPdf* pdf = ws->pdf("model");
  RooNumIntConfig customConfig(*RooAbsReal::defaultIntegratorConfig());
  customConfig.method1D().setLabel("RooMCIntegrator");
  pdf->setIntegratorConfig(customConfig);
  pdf->plotOn(plot);
  pdf->plotOn(plot, RooFit::Components("cemLLmom"), RooFit::LineColor(kRed), RooFit::LineStyle(kDashed));
  pdf->plotOn(plot, RooFit::Components("dioPol58mom"), RooFit::LineColor(kBlue), RooFit::LineStyle(kDashed));

  RooRealVar* NCe = ws->var("NCe");
  RooRealVar* NDio = ws->var("NDio");
  std::cout << "NCe = " << NCe->getValV() << " +/- " << NCe->getError() << std::endl;
  std::cout << "NDio = " << NDio->getValV() << " +/- " << NDio->getError() << std::endl;

  TCanvas* c = new TCanvas();
  c->SetLogy();
  plot->GetYaxis()->SetRangeUser(1e-2, 1e5);
  plot->Draw();
}
