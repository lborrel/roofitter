void plot_cemDio_mom_true() {

  std::string filename = "ana-run7.root";
  TFile* file = new TFile(filename.c_str(), "READ");
  
  RooWorkspace* ws = (RooWorkspace*) file->Get("cemDio_mom_true/cemDio_mom_true");

  ws->Print();

  RooRealVar* mom = ws->var("mom");
  RooPlot* plot = mom->frame();
  
  RooAbsData* data = ws->data("data");
  data->plotOn(plot);
  
  RooAbsPdf* pdf = ws->pdf("model");
  pdf->plotOn(plot);
  pdf->plotOn(plot, RooFit::Components("cemLL"), RooFit::LineColor(kRed), RooFit::LineStyle(kDashed));
  pdf->plotOn(plot, RooFit::Components("dioPol58"), RooFit::LineColor(kBlue), RooFit::LineStyle(kDashed));

  RooRealVar* NCe = ws->var("NCe");
  RooRealVar* NDio = ws->var("NDio");
  RooAbsPdf* Rmue = ws->pdf("Rmue");
  std::cout << "NCe = " << NCe->getValV() << " +/- " << NCe->getError() << std::endl;
  std::cout << "NDio = " << NDio->getValV() << " +/- " << NDio->getError() << std::endl;
  std::cout << "Rmue = " << Rmue->getVal() << std::endl;

  TCanvas* c = new TCanvas();
  c->SetLogy();
  plot->Draw();
}
