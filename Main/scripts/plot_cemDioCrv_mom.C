void plot_cemDioCrv_mom(std::string filename) {

//  std::string filename = "fcl-test7.root";
  TFile* file = new TFile(filename.c_str(), "READ");
  
  RooWorkspace* ws = (RooWorkspace*) file->Get("cemDioCrv_mom/cemDioCrv_mom");

  ws->Print();

  RooRealVar* mom = ws->var("mom");
  RooPlot* plot = mom->frame(RooFit::Range("fit"));
  
  RooAbsData* data = ws->data("data");
  data->plotOn(plot);
  
  RooAbsPdf* pdf = ws->pdf("model");
  pdf->plotOn(plot);
  RooHist* mom_pull = plot->pullHist();
//  pdf->plotOn(plot, RooFit::Components("cemLLmomEffResp"), RooFit::LineColor(kRed), RooFit::LineStyle(kDashed));
//  pdf->plotOn(plot, RooFit::Components("dioPol58momEffResp"), RooFit::LineColor(kBlue), RooFit::LineStyle(kDashed));
//  pdf->plotOn(plot, RooFit::Components("crvFlatmomEffResp"), RooFit::LineColor(kMagenta), RooFit::LineStyle(kDashed));

  RooRealVar* NCe = ws->var("NCe");
  RooRealVar* NDio = ws->var("NDio");
  RooRealVar* NCrv = ws->var("NCrv");
  //  RooAbsPdf* Rmue = ws->pdf("Rmue");
  std::cout << "NCe = " << NCe->getValV() << " +/- " << NCe->getError() << std::endl;
  std::cout << "NDio = " << NDio->getValV() << " +/- " << NDio->getError() << std::endl;
  std::cout << "NCrv = " << NCrv->getValV() << " +/- " << NCrv->getError() << std::endl;
  //  std::cout << "Rmue = " << Rmue->getVal() << std::endl;

  TCanvas* c = new TCanvas();
    c->Divide(1, 2);
    c->GetPad(1)->SetLogy();
    c->cd(1);
  plot->Draw();  
    c->cd(2);
    RooPlot* mom_pull_frame = mom->frame();
    mom_pull_frame->addPlotable(mom_pull, "P");
    mom_pull_frame->Draw();    
    //    RooPlot* new_plot = mom->frame(RooFit::Range(-5, 5));
    //    res->plotOn(new_plot);
    //    new_plot->Draw();
}
