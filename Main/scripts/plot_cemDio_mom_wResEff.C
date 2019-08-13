void plot_cemDio_mom_wResEff() {

  std::string filename = "ana-run7.root";
  TFile* file = new TFile(filename.c_str(), "READ");
  
  RooWorkspace* ws = (RooWorkspace*) file->Get("cemDio_mom_wResEff/cemDio_mom_wResEff");

  ws->Print();

  RooRealVar* mom = ws->var("mom");
  RooPlot* plot = mom->frame();
  
  RooAbsData* data = ws->data("data");
  data->plotOn(plot);
  
  RooAbsPdf* pdf = ws->pdf("model");
    pdf->plotOn(plot);
    RooHist* mom_pull = plot->pullHist();
    pdf->plotOn(plot, RooFit::Components("cemLLEffRes"), RooFit::LineColor(kRed), RooFit::LineStyle(kDashed));
    pdf->plotOn(plot, RooFit::Components("dioPol58EffRes"), RooFit::LineColor(kBlue), RooFit::LineStyle(kDashed));
  //  ws->pdf("cemLLresEff")->plotOn(plot);
  //  ws->pdf("cemLLres")->plotOn(plot, RooFit::LineColor(kRed), RooFit::LineStyle(kDashed));
  //  ws->pdf("cemLL")->plotOn(plot, RooFit::LineColor(kBlack), RooFit::LineStyle(kDashed));
  RooRealVar* NCe = ws->var("NCe");
  RooRealVar* NDio = ws->var("NDio");
  RooAbsPdf* Rmue = ws->pdf("Rmue");
  std::cout << "NCe = " << NCe->getValV() << " +/- " << NCe->getError() << std::endl;
  std::cout << "NDio = " << NDio->getValV() << " +/- " << NDio->getError() << std::endl;
  std::cout << "Rmue = " << Rmue->getVal() << std::endl;

  //  RooFitResult* fitResult = (RooFitResult*) file->Get("cemDio_mom_wResEff/fitresult_modelWEff_data");
  //  std::cout << "NCe = " << Rmue->getVal(*NCe) << " +/- " << Rmue->getPropagatedError(*fitResult) << std::endl;


  TCanvas* c = new TCanvas();
    c->Divide(1, 2);
    c->GetPad(1)->SetLogy();
    c->cd(1);
  plot->Draw();  
    c->cd(2);
    RooPlot* mom_pull_frame = mom->frame();
    mom_pull_frame->addPlotable(mom_pull, "P");
    mom_pull_frame->Draw();    
}
